#[cfg(feature = "midi")]
use midir::{MidiInput, MidiInputConnection};
use anyhow::{Context, Result};
use crossbeam_channel::Sender;
use log::{debug, info, warn, error};
use std::sync::Arc;

use crate::event::PadEvent;

#[cfg(feature = "midi")]
pub struct MidiReader {
    _connection: Option<MidiInputConnection<()>>,
}

#[cfg(feature = "midi")]
impl MidiReader {
    /// Spawn a MIDI reader that sends events to the channel
    pub fn spawn_reader(port_name_hint: &str, tx: Sender<PadEvent>) -> Result<Arc<Self>> {
        let midi_in = MidiInput::new("loop-pad-midi")
            .context("Failed to create MIDI input")?;

        // List available ports for debugging
        let ports = midi_in.ports();
        info!("Available MIDI input ports:");
        for (i, port) in ports.iter().enumerate() {
            if let Ok(name) = midi_in.port_name(port) {
                info!("  [{}] {}", i, name);
            }
        }

        // Find matching port
        let in_port = ports
            .into_iter()
            .find(|p| {
                midi_in
                    .port_name(p)
                    .ok()
                    .map_or(false, |n| n.contains(port_name_hint))
            })
            .ok_or_else(|| {
                anyhow::anyhow!(
                    "MIDI input port containing '{}' not found",
                    port_name_hint
                )
            })?;

        let port_name = midi_in.port_name(&in_port)?;
        info!("Connecting to MIDI port: {}", port_name);

        // Connect to the port with a callback
        let connection = midi_in
            .connect(
                &in_port,
                "loop-pad-in",
                move |_timestamp, message, _| {
                    debug!("MIDI message: {:02X?}", message);
                    
                    // Parse MIDI CC messages
                    if message.len() == 3 && (message[0] & 0xF0) == 0xB0 {
                        let channel = message[0] & 0x0F;
                        let controller = message[1];
                        let value = message[2];
                        
                        let event = PadEvent::MidiCc {
                            channel,
                            controller,
                            value,
                        };
                        
                        if let Err(e) = tx.send(event) {
                            error!("Failed to send MIDI event: {}", e);
                        }
                    }
                },
                (),
            )
            .context("Failed to connect to MIDI port")?;

        Ok(Arc::new(Self {
            _connection: Some(connection),
        }))
    }
}

/// Encoder CC mappings from the firmware
pub mod cc {
    pub const SHARES_DOWN: u8 = 20;
    pub const SHARES_UP: u8 = 21;
    pub const STOP_DOWN: u8 = 22;
    pub const STOP_UP: u8 = 23;
    pub const LIMIT_DOWN: u8 = 24;
    pub const LIMIT_UP: u8 = 25;
}

/// Helper to interpret MIDI CC events as encoder actions
pub fn interpret_midi_cc(controller: u8, value: u8) -> Option<String> {
    if value == 0 {
        return None; // Ignore CC off events
    }

    match controller {
        cc::SHARES_UP => Some("Shares encoder up".to_string()),
        cc::SHARES_DOWN => Some("Shares encoder down".to_string()),
        cc::STOP_UP => Some("Stop encoder up".to_string()),
        cc::STOP_DOWN => Some("Stop encoder down".to_string()),
        cc::LIMIT_UP => Some("Limit encoder up".to_string()),
        cc::LIMIT_DOWN => Some("Limit encoder down".to_string()),
        _ => None,
    }
}

#[cfg(feature = "mock")]
pub struct MockMidiReader;

#[cfg(feature = "mock")]
impl MockMidiReader {
    pub fn spawn_reader(_port_name_hint: &str, tx: Sender<PadEvent>) -> Result<()> {
        tokio::spawn(async move {
            info!("[MOCK] Starting mock MIDI reader");
            
            // Simulate some MIDI CC events
            tokio::time::sleep(tokio::time::Duration::from_secs(2)).await;
            
            let _ = tx.send(PadEvent::MidiCc {
                channel: 0,
                controller: cc::SHARES_UP,
                value: 127,
            });
            
            tokio::time::sleep(tokio::time::Duration::from_millis(500)).await;
            
            let _ = tx.send(PadEvent::MidiCc {
                channel: 0,
                controller: cc::SHARES_DOWN,
                value: 127,
            });
            
            info!("[MOCK] Mock MIDI events sent");
        });
        
        Ok(())
    }
}