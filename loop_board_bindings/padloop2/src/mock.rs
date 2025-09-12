use anyhow::Result;
use crossbeam_channel::Sender;
use log::info;
use std::sync::Arc;
use std::time::Duration;
use tokio::time::sleep;

use crate::event::{PadEvent, Rgb};

pub struct MockPad {
    _tx: Option<Sender<PadEvent>>,
}

impl MockPad {
    pub fn new() -> Self {
        Self { _tx: None }
    }

    pub fn start_mock_events(self: Arc<Self>, tx: Sender<PadEvent>) {

        tokio::spawn(async move {
            info!("Starting mock event generator");
            
            // Send a hello message
            let _ = tx.send(PadEvent::Hello {
                proto: 1,
                fw_major: 1,
                fw_minor: 0,
            });

            // Initial layer
            sleep(Duration::from_millis(100)).await;
            let _ = tx.send(PadEvent::Layer { layer: 0 });

            // Simulate continuous encoder movements
            sleep(Duration::from_millis(500)).await;
            
            // Encoder 0: Increment by 5
            for _ in 0..5 {
                let _ = tx.send(PadEvent::EncRotate { idx: 0, delta: 1 });
                sleep(Duration::from_millis(100)).await;
            }
            
            sleep(Duration::from_millis(500)).await;
            
            // Encoder 1: Increment by 3, then decrement by 2
            for _ in 0..3 {
                let _ = tx.send(PadEvent::EncRotate { idx: 1, delta: 1 });
                sleep(Duration::from_millis(150)).await;
            }
            for _ in 0..2 {
                let _ = tx.send(PadEvent::EncRotate { idx: 1, delta: -1 });
                sleep(Duration::from_millis(150)).await;
            }
            
            sleep(Duration::from_millis(500)).await;
            
            // Encoder 2: Large movement
            for _ in 0..10 {
                let _ = tx.send(PadEvent::EncRotate { idx: 2, delta: 1 });
                sleep(Duration::from_millis(50)).await;
            }
            
            sleep(Duration::from_millis(500)).await;
            
            // Encoder 0: Decrement back
            for _ in 0..3 {
                let _ = tx.send(PadEvent::EncRotate { idx: 0, delta: -1 });
                sleep(Duration::from_millis(100)).await;
            }
            
            // Test encoder press
            sleep(Duration::from_millis(500)).await;
            let _ = tx.send(PadEvent::EncPress {
                idx: 0,
                long: false,
            });
            
            sleep(Duration::from_millis(500)).await;
            
            // All encoders move together
            for _ in 0..5 {
                let _ = tx.send(PadEvent::EncRotate { idx: 0, delta: 1 });
                let _ = tx.send(PadEvent::EncRotate { idx: 1, delta: -1 });
                let _ = tx.send(PadEvent::EncRotate { idx: 2, delta: 1 });
                sleep(Duration::from_millis(200)).await;
            }

            info!("Mock event generator finished sequence");
            
            // Keep alive with occasional events
            loop {
                sleep(Duration::from_secs(5)).await;
                // Random encoder movement every 5 seconds
                let encoder_idx = (tokio::time::Instant::now().elapsed().as_secs() % 3) as u8;
                let delta = if tokio::time::Instant::now().elapsed().as_secs() % 2 == 0 { 1 } else { -1 };
                let _ = tx.send(PadEvent::EncRotate { idx: encoder_idx, delta });
            }
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