use crossbeam_channel::Sender;
use std::thread;
use std::time::{Duration, Instant};

use super::protocol::PadEvent;

/// Start a simple mock event generator.
/// Emits Hello once, then periodically sends Layer, Button, EncRotate, and EncPress events.
pub fn start_mock(tx: Sender<PadEvent>) -> thread::JoinHandle<()> {
    thread::spawn(move || {
        let _ = tx.send(PadEvent::Hello {
            proto: 1,
            fw_major: 0,
            fw_minor: 1,
        });

        let start = Instant::now();
        let mut i: u64 = 0;
        loop {
            let elapsed = start.elapsed().as_secs_f32();

            // Layer change every ~4 seconds: 0 -> 1 -> 2 -> 0
            if i % 40 == 0 {
                let layer = ((i / 40) % 3) as u8;
                let _ = tx.send(PadEvent::Layer { layer });
            }

            // Button press/release pairs cycling through 0..8 every ~1s
            let btn_idx = ((i / 10) % 9) as u8;
            if i % 10 == 0 {
                let layer = 0; // base layer for simplicity
                let _ = tx.send(PadEvent::Button {
                    layer,
                    idx: btn_idx,
                    pressed: true,
                });
            }
            if i % 10 == 5 {
                let layer = 0;
                let _ = tx.send(PadEvent::Button {
                    layer,
                    idx: btn_idx,
                    pressed: false,
                });
            }

            // Encoder rotate on idx 0..2 every ~0.5s
            if i % 5 == 0 {
                let enc_idx = ((i / 5) % 3) as u8;
                let delta = if (i / 5) % 2 == 0 { 1 } else { -1 };
                let _ = tx.send(PadEvent::EncRotate {
                    idx: enc_idx,
                    delta,
                });
            }

            // Encoder press every ~6s, cycles mode
            if ((elapsed as u64) % 6) == 0 && i % 10 == 0 {
                let enc_idx = ((i / 10) % 3) as u8;
                let _ = tx.send(PadEvent::EncPress {
                    idx: enc_idx,
                    long: false,
                });
            }

            i = i.wrapping_add(1);
            thread::sleep(Duration::from_millis(100));
        }
    })
}

