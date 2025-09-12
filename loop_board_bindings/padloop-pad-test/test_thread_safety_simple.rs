// Simple demonstration of the thread safety solution
use std::sync::Arc;
use std::thread;
use std::sync::mpsc::{channel, Sender};

// Simulating HidDevice that doesn't implement Sync
struct HidDevice {
    data: *const u8, // Raw pointer makes it !Sync
}
unsafe impl Send for HidDevice {}

enum HidCommand {
    Write { id: u8, payload: Vec<u8> },
    Shutdown,
}

// After read_loop is called, this struct only contains the Sender
struct HidPadHandle {
    cmd_tx: Sender<HidCommand>,
}

impl HidPadHandle {
    fn write_packet(&self, id: u8, payload: &[u8]) -> Result<(), String> {
        self.cmd_tx
            .send(HidCommand::Write {
                id,
                payload: payload.to_vec(),
            })
            .map_err(|e| format!("Failed to send: {}", e))
    }

    fn shutdown(&self) -> Result<(), String> {
        self.cmd_tx
            .send(HidCommand::Shutdown)
            .map_err(|e| format!("Failed to send: {}", e))
    }
}

fn start_hid_thread() -> Arc<HidPadHandle> {
    let (cmd_tx, cmd_rx) = channel();
    
    // Create HidDevice in the thread scope
    thread::spawn(move || {
        // HidDevice is created and owned by this thread
        let _device = HidDevice {
            data: std::ptr::null(),
        };
        
        println!("HID thread started");
        
        loop {
            if let Ok(cmd) = cmd_rx.try_recv() {
                match cmd {
                    HidCommand::Write { id, payload } => {
                        println!("Writing to HID: id={}, payload={:?}", id, payload);
                        // Would use _device here
                    }
                    HidCommand::Shutdown => {
                        println!("HID thread shutting down");
                        break;
                    }
                }
            }
            thread::sleep(std::time::Duration::from_millis(10));
        }
    });
    
    // Return handle with only the Sender (which is Send + Sync)
    Arc::new(HidPadHandle { cmd_tx })
}

fn main() {
    println!("Starting HID interface...");
    let pad = start_hid_thread();
    
    // Can safely share across threads now!
    let pad_clone = pad.clone();
    let handle = thread::spawn(move || {
        println!("Secondary thread: Sending write");
        pad_clone.write_packet(0x01, &[1, 2, 3]).unwrap();
    });
    
    thread::sleep(std::time::Duration::from_millis(50));
    pad.write_packet(0x02, &[4, 5, 6]).unwrap();
    
    handle.join().unwrap();
    
    thread::sleep(std::time::Duration::from_millis(50));
    println!("Shutting down...");
    pad.shutdown().unwrap();
    
    thread::sleep(std::time::Duration::from_millis(50));
    println!("Done!");
}