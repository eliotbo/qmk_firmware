pub mod config;
pub mod event;
pub mod state;

#[cfg(feature = "hid")]
pub mod hid;

#[cfg(feature = "mock")]
pub mod mock;

use anyhow::Result;
use crossbeam_channel::{unbounded, Receiver, Sender};
use log::info;
use std::sync::Arc;

pub use config::Config;
pub use event::{PadEvent, Rgb};

#[cfg(feature = "mock")]
use mock::MockPad;

/// Main interface to the Loop Pad
pub struct Pad {
    #[cfg(feature = "hid")]
    hid: Option<Arc<hid::HidPad>>,
    #[cfg(feature = "mock")]
    mock: Option<Arc<MockPad>>,
    rx: Receiver<PadEvent>,
    tx: Sender<PadEvent>,
}

impl Pad {
    /// Create a new Pad instance with the given configuration
    pub fn new(_config: &Config) -> Result<Self> {
        let (tx, rx) = unbounded();

        Ok(Self {
            #[cfg(feature = "hid")]
            hid: None,
            #[cfg(feature = "mock")]
            mock: None,
            rx,
            tx,
        })
    }

    /// Spawn background I/O tasks and return event receiver
    pub fn spawn(&mut self, _config: &Config) -> Result<Receiver<PadEvent>> {
        info!("Starting Loop Pad interface");

        // Check if we should force mock mode
        let force_mock = _config.hid.vid == 0 && _config.hid.pid == 0;

        // Start HID interface
        #[cfg(feature = "hid")]
        {
            if !force_mock {
                match hid::HidPad::open(
                    _config.hid.vid,
                    _config.hid.pid,
                    _config.hid.usage_page,
                    _config.hid.usage,
                ) {
                    Ok(pad) => {
                        // read_loop now takes ownership and returns an Arc
                        let pad = pad.read_loop(self.tx.clone());
                        self.hid = Some(pad);
                        info!("HID interface started");
                    }
                    Err(e) => {
                        log::warn!("Failed to open HID device: {}", e);
                    }
                }
            }
        }

        // Start mock interface if enabled
        #[cfg(feature = "mock")]
        {
            #[cfg(not(feature = "hid"))]
            {
                let mock = Arc::new(MockPad::new());
                mock.clone().start_mock_events(self.tx.clone());
                self.mock = Some(mock);
                info!("Mock interface started");
            }
            #[cfg(feature = "hid")]
            {
                if self.hid.is_none() {
                    let mock = Arc::new(MockPad::new());
                    mock.clone().start_mock_events(self.tx.clone());
                    self.mock = Some(mock);
                    info!("Mock interface started");
                }
            }
        }



        Ok(self.rx.clone())
    }

    /// Send host ready handshake
    pub fn send_ready(&self) -> Result<()> {
        #[cfg(feature = "hid")]
        if let Some(ref hid) = self.hid {
            return hid.send_ready();
        }

        #[cfg(feature = "mock")]
        if let Some(ref mock) = self.mock {
            return mock.send_ready();
        }

        Err(anyhow::anyhow!("No pad interface available"))
    }

    /// Set all LEDs to the same color
    pub fn set_all_leds(self: Arc<Self>, rgb: Rgb) -> Result<()> {
        #[cfg(feature = "hid")]
        if let Some(ref hid) = self.hid {
            return hid.set_all_leds(rgb);
        }

        #[cfg(feature = "mock")]
        if let Some(ref mock) = self.mock {
            return mock.set_all_leds(rgb);
        }

        Err(anyhow::anyhow!("No pad interface available"))
    }

    /// Set a single LED color
    pub fn set_led(self: Arc<Self>, idx: u8, rgb: Rgb) -> Result<()> {
        #[cfg(feature = "hid")]
        if let Some(ref hid) = self.hid {
            return hid.set_led(idx, rgb);
        }

        #[cfg(feature = "mock")]
        if let Some(ref mock) = self.mock {
            return mock.set_led(idx, rgb);
        }

        Err(anyhow::anyhow!("No pad interface available"))
    }

    /// Set the active layer
    pub fn set_layer(&self, layer: u8) -> Result<()> {
        #[cfg(feature = "hid")]
        if let Some(ref hid) = self.hid {
            return hid.set_layer(layer);
        }

        #[cfg(feature = "mock")]
        if let Some(ref mock) = self.mock {
            return mock.set_layer(layer);
        }

        Err(anyhow::anyhow!("No pad interface available"))
    }

    /// Get event receiver
    pub fn events(&self) -> &Receiver<PadEvent> {
        &self.rx
    }
}

/// Helper function for LED animations
pub async fn pulse_led(pad: &Arc<Pad>, idx: u8, color: Rgb, duration_ms: u64) -> Result<()> {
    use tokio::time::{sleep, Duration};

    let steps = 20;
    let step_duration = Duration::from_millis(duration_ms / (steps * 2));

    // Fade in
    for i in 0..=steps {
        let factor = i as f32 / steps as f32;
        let rgb = Rgb {
            r: (color.r as f32 * factor) as u8,
            g: (color.g as f32 * factor) as u8,
            b: (color.b as f32 * factor) as u8,
        };
        pad.clone().set_led(idx, rgb)?;
        sleep(step_duration).await;
    }

    // Fade out
    for i in (0..=steps).rev() {
        let factor = i as f32 / steps as f32;
        let rgb = Rgb {
            r: (color.r as f32 * factor) as u8,
            g: (color.g as f32 * factor) as u8,
            b: (color.b as f32 * factor) as u8,
        };
        pad.clone().set_led(idx, rgb)?;
        sleep(step_duration).await;
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pad_creation() {
        let config = Config::default();
        let pad = Pad::new(&config);
        assert!(pad.is_ok());
    }
}
