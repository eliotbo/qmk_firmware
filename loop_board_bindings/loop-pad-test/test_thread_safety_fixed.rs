// Demonstration of the fixed thread safety approach
use std::sync::Arc;
use std::thread;
use std::sync::mpsc::{channel, Sender, Receiver};

// Simulating the HidDevice structure that doesn't implement Sync
struct HidDevice {
    data: *const u8, // Raw pointer makes it !Sync
}

// SAFETY: We're just simulating - HidDevice is Send but NOT Sync
unsafe impl Send for HidDevice {}

enum HidCommand {
    Write { id: u8, payload: Vec<u8> },
    Shutdown,
}

struct HidPad {
    cmd_tx: Sender<HidCommand>,
    cmd_rx: Option<Receiver<HidCommand>>,
    device: Option<HidDevice>,
}

impl HidPad {
    fn new() -> Self {
        let (cmd_tx, cmd_rx) = channel();
        Self {
            cmd_tx,
            cmd_rx: Some(cmd_rx),
            device: Some(HidDevice {
                data: std::ptr::null(),
            }),
        }
    }

    // Solution: Move ownership of HidDevice into thread, return Arc with only channel
    fn read_loop(mut self) -> Arc<HidPad> {
        let _device = self.device.take().expect("Device already taken");
        let cmd_rx = self.cmd_rx.take().expect("Command receiver already taken");
        let cmd_tx = self.cmd_tx.clone();

        // Spawn thread with OWNED HidDevice (not shared via Arc)
        thread::spawn(move || {
            println!("HID thread started");
            // _device is now owned by this thread
            
            // Simulate reading from device
            loop {
                // Check for commands
                if let Ok(cmd) = cmd_rx.try_recv() {
                    match cmd {
                        HidCommand::Write { id, payload } => {
                            println!("Writing to HID: id={}, payload={:?}", id, payload);
                            // Would write to _device here
                        }
                        HidCommand::Shutdown => {
                            println!("HID thread shutting down");
                            break;
                        }
                    }
                }
                
                // Simulate reading from _device
                thread::sleep(std::time::Duration::from_millis(10));
            }
        });

        // Return Arc with only the command sender (which IS Sync)
        Arc::new(HidPad {
            cmd_tx,
            cmd_rx: None,  // Moved to thread
            device: None,  // Moved to thread
        })
    }

    fn write_packet(&self, id: u8, payload: &[u8]) -> Result<(), String> {
        self.cmd_tx
            .send(HidCommand::Write {
                id,
                payload: payload.to_vec(),
            })
            .map_err(|e| format!("Failed to send command: {}", e))
    }

    fn shutdown(&self) -> Result<(), String> {
        self.cmd_tx
            .send(HidCommand::Shutdown)
            .map_err(|e| format!("Failed to send shutdown: {}", e))
    }
}

fn main() {
    println!("Creating HID pad...");
    let pad = HidPad::new();
    
    println!("Starting read loop...");
    let pad = pad.read_loop();
    
    // Now we can safely share the Arc across threads!
    let pad_clone = pad.clone();
    let handle = thread::spawn(move || {
        println!("Secondary thread: Sending write command");
        pad_clone.write_packet(0x01, &[1, 2, 3]).unwrap();
    });
    
    // Main thread can also use it
    thread::sleep(std::time::Duration::from_millis(100));
    pad.write_packet(0x02, &[4, 5, 6]).unwrap();
    
    handle.join().unwrap();
    
    thread::sleep(std::time::Duration::from_millis(100));
    println!("Shutting down...");
    pad.shutdown().unwrap();
    
    thread::sleep(std::time::Duration::from_millis(100));
    println!("Done!");
}