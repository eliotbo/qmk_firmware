use anyhow::Result;
use clap::Parser;
use crossbeam_channel::Receiver;
use env_logger;
use log::info;
use loop_pad_test::{Config, Pad, PadEvent};
use std::sync::Arc;

#[derive(Parser, Debug)]
#[command(author, version, about = "Encoder position tracker for Loop Pad", long_about = None)]
struct Args {
    /// Path to configuration file
    #[arg(short, long, default_value = "padtest.toml")]
    config: String,

    /// Enable mock mode (no real HID device needed)
    #[arg(long)]
    mock: bool,

    /// Enable verbose logging
    #[arg(short, long)]
    verbose: bool,
}

#[tokio::main]
async fn main() -> Result<()> {
    let args = Args::parse();

    // Initialize logging
    let mut builder = env_logger::Builder::from_default_env();
    if args.verbose {
        builder.filter_level(log::LevelFilter::Debug);
    } else {
        builder.filter_level(log::LevelFilter::Info);
    }
    builder.init();

    // Load configuration
    let config = if args.mock {
        Config::mock()
    } else {
        Config::load_from_file(&args.config)?
    };

    // Create pad instance
    let mut pad = Pad::new(&config)?;
    let events = pad.spawn(&config)?;
    let pad = Arc::new(pad);

    // Send ready signal if HID
    #[cfg(feature = "hid")]
    if !args.mock {
        pad.send_ready()?;
    }

    // Track encoder positions
    track_encoders(pad.clone(), events)?;

    Ok(())
}

fn track_encoders(pad: Arc<Pad>, events: Receiver<PadEvent>) -> Result<()> {
    info!("Starting encoder tracker. Rotate encoders to see position changes.");
    println!("=== ENCODER POSITION TRACKER ===");
    println!("Initial positions: Enc0=0, Enc1=0, Enc2=0");
    println!("Rotate encoders to update positions...\n");

    // Initialize encoder positions
    let mut encoder_positions: [i32; 3] = [0, 0, 0];

    loop {
        match events.recv() {
            Ok(event) => {
                match event {
                    PadEvent::EncRotate { idx, delta } => {
                        if idx < 3 {
                            // Update position
                            let old_pos = encoder_positions[idx as usize];
                            encoder_positions[idx as usize] += delta as i32;
                            let new_pos = encoder_positions[idx as usize];
                            
                            // Print change
                            println!(
                                "Encoder {} changed: {} -> {} (delta: {})",
                                idx, old_pos, new_pos, delta
                            );
                            
                            // Print current state of all encoders
                            println!(
                                "Current positions: Enc0={}, Enc1={}, Enc2={}\n",
                                encoder_positions[0],
                                encoder_positions[1],
                                encoder_positions[2]
                            );
                        }
                    }
                    PadEvent::EncPress { idx, long } => {
                        if idx < 3 {
                            let press_type = if long { "LONG" } else { "SHORT" };
                            println!(
                                "Encoder {} {} press detected (current position: {})",
                                idx, press_type, encoder_positions[idx as usize]
                            );
                        }
                    }
                    PadEvent::Button { layer, idx, pressed } => {
                        let action = if pressed { "pressed" } else { "released" };
                        println!("Button {} {} on layer {}", idx, action, layer);
                    }
                    PadEvent::Layer { layer } => {
                        println!("Layer changed to: {}", layer);
                    }
                    PadEvent::Hello { proto, fw_major, fw_minor } => {
                        println!(
                            "Device connected: Protocol v{}, Firmware v{}.{}",
                            proto, fw_major, fw_minor
                        );
                        // Send ready signal after hello
                        if let Err(e) = pad.send_ready() {
                            eprintln!("Failed to send ready signal: {}", e);
                        }
                    }
                    PadEvent::MidiCc { channel, controller, value } => {
                        // Log MIDI CC events but don't track them
                        println!("MIDI CC: channel={}, controller={}, value={}", channel, controller, value);
                    }
                }
            }
            Err(e) => {
                eprintln!("Error receiving event: {}", e);
                break;
            }
        }
    }

    Ok(())
}