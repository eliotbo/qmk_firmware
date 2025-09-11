#[cfg(feature = "hid")]
use hidapi::{HidApi, HidDevice};
use anyhow::{Context, Result};
use crossbeam_channel::{Sender, Receiver, bounded};
use log::{debug, info, error};
use std::sync::Arc;
use std::time::Duration;
use std::thread;
#[cfg(feature = "mock")]
use tokio::time::sleep;

use crate::event::{parse_hid_event, PadEvent, Rgb};

pub const HID_BUFFER_SIZE: usize = 32;
pub const HID_LONGPRESS_MS: u64 = 500;

#[cfg(feature = "hid")]
pub struct HidPad {
    // Use a channel to send commands to the HID thread
    cmd_tx: Sender<HidCommand>,
    cmd_rx: Option<Receiver<HidCommand>>,
    device: Option<HidDevice>,
}

#[cfg(feature = "hid")]
enum HidCommand {
    Write { id: u8, payload: Vec<u8> },
    Shutdown,
}

#[cfg(feature = "hid")]
impl HidPad {
    /// Open HID device by VID/PID and optional usage page/usage
    pub fn open(vid: u16, pid: u16, usage_page: Option<u16>, usage: Option<u16>) -> Result<Self> {
        let api = HidApi::new().context("Failed to create HID API")?;
        
        // Find matching devices
        let devices: Vec<_> = api
            .device_list()
            .filter(|info| {
                info.vendor_id() == vid && info.product_id() == pid
                    && (usage_page.is_none() || info.usage_page() == usage_page.unwrap())
                    && (usage.is_none() || info.usage() == usage.unwrap())
            })
            .collect();

        if devices.is_empty() {
            return Err(anyhow::anyhow!(
                "No HID device found with VID={:04X}, PID={:04X}",
                vid,
                pid
            ));
        }

        // Open the first matching device
        let device = devices[0]
            .open_device(&api)
            .context("Failed to open HID device")?;

        // Set non-blocking mode for reads
        device.set_blocking_mode(false)?;

        info!(
            "Opened HID device: VID={:04X}, PID={:04X}",
            vid, pid
        );

        // Create a channel for sending commands to the HID thread
        let (cmd_tx, cmd_rx) = bounded(100);

        Ok(Self { 
            cmd_tx,
            cmd_rx: Some(cmd_rx),
            device: Some(device),
        })
    }

    /// Start background task to read HID packets
    /// This moves the HidDevice into the thread to avoid Sync requirement
    pub fn read_loop(mut self, tx: Sender<PadEvent>) -> Arc<HidPad> {
        // Take ownership of the device and command receiver
        let device = self.device.take().expect("Device already taken");
        let cmd_rx = self.cmd_rx.take().expect("Command receiver already taken");
        
        // Clear the cmd_tx to prevent Drop from sending shutdown
        // We'll create a new one for the Arc
        let (new_cmd_tx, _) = bounded(100);
        let old_cmd_tx = std::mem::replace(&mut self.cmd_tx, new_cmd_tx);
        
        // Spawn the thread with owned HidDevice
        thread::spawn(move || {
            let mut buf = [0u8; HID_BUFFER_SIZE];
            info!("Starting HID read loop");
            
            loop {
                // Check for commands (non-blocking)
                if let Ok(cmd) = cmd_rx.try_recv() {
                    match cmd {
                        HidCommand::Write { id, payload } => {
                            if let Err(e) = Self::write_packet_internal(&device, id, &payload) {
                                error!("Failed to write HID packet: {}", e);
                            }
                        }
                        HidCommand::Shutdown => {
                            info!("Shutting down HID thread");
                            break;
                        }
                    }
                }
                
                // Non-blocking read with small timeout
                match device.read_timeout(&mut buf, 10) {
                    Ok(n) if n > 0 => {
                        debug!("Read {} bytes from HID: {:02X?}", n, &buf[..n]);
                        if let Some(evt) = parse_hid_event(&buf[..n]) {
                            if let Err(e) = tx.send(evt) {
                                error!("Failed to send event: {}", e);
                                break;
                            }
                        }
                    }
                    Ok(_) => {
                        // No data available, continue
                    }
                    Err(e) => {
                        error!("HID read error: {}", e);
                        // Sleep a bit before retrying
                        thread::sleep(Duration::from_millis(100));
                    }
                }
                
                // Small sleep to prevent CPU spinning
                thread::sleep(Duration::from_millis(1));
            }
            
            info!("HID read loop ended");
        });
        
        // Return Arc to self for sending commands (use old_cmd_tx)
        Arc::new(HidPad {
            cmd_tx: old_cmd_tx,
            cmd_rx: None,
            device: None,
        })
    }

    /// Internal write function used by the HID thread
    fn write_packet_internal(device: &HidDevice, id: u8, payload: &[u8]) -> Result<()> {
        let mut pkt = [0u8; HID_BUFFER_SIZE];
        pkt[0] = id;
        let n = payload.len().min(31);
        pkt[1..1 + n].copy_from_slice(&payload[..n]);

        #[cfg(target_os = "windows")]
        {
            // Windows requires prepending report ID 0x00 for raw HID
            let mut wpkt = [0u8; 33];
            wpkt[0] = 0x00;
            wpkt[1..33].copy_from_slice(&pkt);
            device.write(&wpkt)?;
            debug!("Sent HID packet (Windows): {:02X?}", &wpkt[..5]);
        }

        #[cfg(not(target_os = "windows"))]
        {
            device.write(&pkt)?;
            debug!("Sent HID packet: {:02X?}", &pkt[..5]);
        }

        Ok(())
    }
    
    /// Write a packet to the HID device (sends command to HID thread)
    fn write_packet(&self, id: u8, payload: &[u8]) -> Result<()> {
        self.cmd_tx
            .send(HidCommand::Write {
                id,
                payload: payload.to_vec(),
            })
            .map_err(|e| anyhow::anyhow!("Failed to send command to HID thread: {}", e))
    }

    /// Send host ready handshake (0x7D)
    pub fn send_ready(&self) -> Result<()> {
        info!("Sending host ready handshake");
        self.write_packet(0x7D, &[1])
    }

    /// Set all LEDs to the same color (0x01) - BASE layer only
    pub fn set_all_leds(&self, rgb: Rgb) -> Result<()> {
        debug!("Setting all LEDs to RGB({},{},{})", rgb.r, rgb.g, rgb.b);
        self.write_packet(0x01, &[rgb.r, rgb.g, rgb.b])
    }

    /// Set a single LED color (0x02) - BASE layer only
    pub fn set_led(&self, idx: u8, rgb: Rgb) -> Result<()> {
        if idx >= 9 {
            return Err(anyhow::anyhow!("LED index must be 0-8, got {}", idx));
        }
        debug!("Setting LED {} to RGB({},{},{})", idx, rgb.r, rgb.g, rgb.b);
        self.write_packet(0x02, &[idx, rgb.r, rgb.g, rgb.b])
    }

    /// Set the active layer (0x03)
    pub fn set_layer(&self, layer: u8) -> Result<()> {
        if layer > 2 {
            return Err(anyhow::anyhow!("Layer must be 0-2, got {}", layer));
        }
        info!("Setting layer to {}", layer);
        self.write_packet(0x03, &[layer])
    }
    
    /// Shutdown the HID thread gracefully
    pub fn shutdown(&self) -> Result<()> {
        self.cmd_tx
            .send(HidCommand::Shutdown)
            .map_err(|e| anyhow::anyhow!("Failed to send shutdown command: {}", e))
    }
}

#[cfg(feature = "hid")]
impl Drop for HidPad {
    fn drop(&mut self) {
        // Try to shutdown the HID thread gracefully
        let _ = self.cmd_tx.send(HidCommand::Shutdown);
    }
}

// Mock implementation for testing
#[cfg(feature = "mock")]
pub struct MockPad {
    tx: Option<Sender<PadEvent>>,
}

#[cfg(feature = "mock")]
impl MockPad {
    pub fn new() -> Self {
        Self { tx: None }
    }

    pub fn start_mock_events(self: Arc<Self>, tx: Sender<PadEvent>) {
        // Just use the passed tx directly

        tokio::spawn(async move {
            info!("Starting mock event generator");
            
            // Send a hello message
            let _ = tx.send(PadEvent::Hello {
                proto: 1,
                fw_major: 1,
                fw_minor: 0,
            });

            // Simulate some events
            sleep(Duration::from_millis(100)).await;
            let _ = tx.send(PadEvent::Layer { layer: 0 });

            sleep(Duration::from_millis(500)).await;
            let _ = tx.send(PadEvent::Button {
                layer: 0,
                idx: 0,
                pressed: true,
            });

            sleep(Duration::from_millis(100)).await;
            let _ = tx.send(PadEvent::Button {
                layer: 0,
                idx: 0,
                pressed: false,
            });

            sleep(Duration::from_millis(200)).await;
            let _ = tx.send(PadEvent::EncRotate { idx: 0, delta: 1 });

            sleep(Duration::from_millis(100)).await;
            let _ = tx.send(PadEvent::EncRotate { idx: 0, delta: -1 });

            sleep(Duration::from_millis(300)).await;
            let _ = tx.send(PadEvent::EncPress {
                idx: 1,
                long: false,
            });

            info!("Mock event generator finished initial sequence");
        });
    }

    pub fn send_ready(&self) -> Result<()> {
        info!("[MOCK] Sending host ready");
        Ok(())
    }

    pub fn set_all_leds(&self, rgb: Rgb) -> Result<()> {
        info!("[MOCK] Setting all LEDs to RGB({},{},{})", rgb.r, rgb.g, rgb.b);
        Ok(())
    }

    pub fn set_led(&self, idx: u8, rgb: Rgb) -> Result<()> {
        info!("[MOCK] Setting LED {} to RGB({},{},{})", idx, rgb.r, rgb.g, rgb.b);
        Ok(())
    }

    pub fn set_layer(&self, layer: u8) -> Result<()> {
        info!("[MOCK] Setting layer to {}", layer);
        Ok(())
    }
}