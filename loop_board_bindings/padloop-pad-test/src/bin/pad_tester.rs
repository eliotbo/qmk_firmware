use anyhow::Result;
use clap::{Parser, Subcommand};
use crossbeam_channel::Receiver;
use log::{error, info};
use loop_pad_test::{Config, Pad, PadEvent, Rgb};
use std::io::{self, Write};
use tokio::time::{sleep, Duration};

#[derive(Parser)]
#[command(name = "pad-tester")]
#[command(about = "Interactive tester for Loop Pad keyboard", long_about = None)]
struct Cli {
    /// Path to configuration file
    #[arg(short, long, default_value = "padtest.toml")]
    config: String,

    /// Enable mock mode (no real HID device needed)
    #[arg(long)]
    mock: bool,

    /// Enable verbose logging
    #[arg(short, long)]
    verbose: bool,

    #[command(subcommand)]
    command: Option<Commands>,
}

#[derive(Subcommand)]
enum Commands {
    /// Run interactive REPL mode
    Repl,
    /// Just listen and print events
    Listen,
    /// Run a demo sequence
    Demo,
}

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();

    // Initialize logger
    if cli.verbose {
        env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("debug"))
            .init();
    } else {
        env_logger::Builder::from_env(env_logger::Env::default().default_filter_or("info"))
            .init();
    }

    // Load configuration
    let config = Config::load_from_file(&cli.config)
        .or_else(|_| {
            info!("Config file not found, using defaults");
            Ok::<Config, anyhow::Error>(Config::default())
        })?;

    // Override with mock if requested
    if cli.mock {
        info!("Mock mode enabled");
    }

    // Create and start the pad interface
    let mut pad = Pad::new(&config)?;
    let events = pad.spawn(&config)?;

    // Send initial setup commands
    sleep(Duration::from_millis(100)).await;
    pad.send_ready()?;
    let pad = std::sync::Arc::new(pad);
    pad.clone().set_layer(config.demo.start_layer)?;
    pad.clone().set_all_leds(config.demo.start_color.into())?;

    info!("Loop Pad interface started");

    match cli.command {
        Some(Commands::Listen) | None => {
            listen_mode(events).await?;
        }
        Some(Commands::Repl) => {
            repl_mode(pad.clone(), events).await?;
        }
        Some(Commands::Demo) => {
            demo_mode(pad.clone(), events).await?;
        }
    }

    Ok(())
}

async fn listen_mode(events: Receiver<PadEvent>) -> Result<()> {
    info!("Listening for events (press Ctrl+C to stop)...");

    tokio::spawn(async move {
        loop {
            match events.recv() {
                Ok(event) => {
                    println!("Event: {:?}", event);

                    // Special formatting for certain events
                    match event {
                        PadEvent::Hello { proto, fw_major, fw_minor } => {
                            println!("  → Device connected: Protocol v{}, Firmware v{}.{}", 
                                proto, fw_major, fw_minor);
                        }
                        PadEvent::Button { layer, idx, pressed } => {
                            let state = if pressed { "PRESSED" } else { "RELEASED" };
                            println!("  → Button {} on layer {} {}", idx, layer, state);
                        }
                        PadEvent::EncRotate { idx, delta } => {
                            let direction = if delta > 0 { "CW" } else { "CCW" };
                            println!("  → Encoder {} rotated {} (delta: {})", idx, direction, delta);
                        }
                        PadEvent::EncPress { idx, long } => {
                            let press_type = if long { "LONG" } else { "SHORT" };
                            println!("  → Encoder {} {} press", idx, press_type);
                        }
                        PadEvent::Layer { layer } => {
                            let layer_name = match layer {
                                0 => "BASE",
                                1 => "BUY",
                                2 => "SELL",
                                _ => "UNKNOWN",
                            };
                            println!("  → Switched to layer {} ({})", layer, layer_name);
                        }
                        #[cfg(feature = "midi")]
                        PadEvent::MidiCc { channel, controller, value } => {
                            if let Some(action) = loop_pad_test::midi::interpret_midi_cc(controller, value) {
                                println!("  → MIDI: {}", action);
                            }
                        }
                        _ => {}
                    }
                }
                Err(e) => {
                    error!("Event receive error: {}", e);
                    break;
                }
            }
        }
    });

    // Keep main thread alive
    tokio::signal::ctrl_c().await?;
    info!("Shutting down...");
    Ok(())
}

async fn repl_mode(pad: std::sync::Arc<Pad>, events: Receiver<PadEvent>) -> Result<()> {
    info!("Starting REPL mode. Type 'help' for commands.");

    // Start event printer in background
    tokio::spawn(async move {
        loop {
            if let Ok(event) = events.recv() {
                println!("[Event] {:?}", event);
            }
        }
    });

    // REPL loop
    loop {
        print!("> ");
        io::stdout().flush()?;

        let mut input = String::new();
        io::stdin().read_line(&mut input)?;
        let input = input.trim();

        let parts: Vec<&str> = input.split_whitespace().collect();
        if parts.is_empty() {
            continue;
        }

        match parts[0] {
            "help" | "h" => {
                print_help();
            }
            "quit" | "q" | "exit" => {
                info!("Exiting...");
                break;
            }
            "all" => {
                if parts.len() == 4 {
                    match (parts[1].parse::<u8>(), parts[2].parse::<u8>(), parts[3].parse::<u8>()) {
                        (Ok(r), Ok(g), Ok(b)) => {
                            pad.clone().set_all_leds(Rgb { r, g, b })?;
                            println!("Set all LEDs to RGB({},{},{})", r, g, b);
                        }
                        _ => println!("Usage: all <r> <g> <b> (values 0-255)"),
                    }
                } else {
                    println!("Usage: all <r> <g> <b>");
                }
            }
            "led" => {
                if parts.len() == 5 {
                    match (
                        parts[1].parse::<u8>(),
                        parts[2].parse::<u8>(),
                        parts[3].parse::<u8>(),
                        parts[4].parse::<u8>(),
                    ) {
                        (Ok(idx), Ok(r), Ok(g), Ok(b)) => {
                            pad.clone().set_led(idx, Rgb { r, g, b })?;
                            println!("Set LED {} to RGB({},{},{})", idx, r, g, b);
                        }
                        _ => println!("Usage: led <idx> <r> <g> <b> (idx: 0-8, values 0-255)"),
                    }
                } else {
                    println!("Usage: led <idx> <r> <g> <b>");
                }
            }
            "layer" => {
                if parts.len() == 2 {
                    match parts[1].parse::<u8>() {
                        Ok(layer) if layer <= 2 => {
                            pad.clone().set_layer(layer)?;
                            let name = match layer {
                                0 => "BASE",
                                1 => "BUY",
                                2 => "SELL",
                                _ => "UNKNOWN",
                            };
                            println!("Set layer to {} ({})", layer, name);
                        }
                        _ => println!("Usage: layer <0-2>"),
                    }
                } else {
                    println!("Usage: layer <0-2>");
                }
            }
            "pulse" => {
                if parts.len() >= 2 {
                    match parts[1].parse::<u8>() {
                        Ok(idx) if idx < 9 => {
                            let color = if parts.len() >= 5 {
                                match (parts[2].parse::<u8>(), parts[3].parse::<u8>(), parts[4].parse::<u8>()) {
                                    (Ok(r), Ok(g), Ok(b)) => Rgb { r, g, b },
                                    _ => Rgb::WHITE,
                                }
                            } else {
                                Rgb::WHITE
                            };
                            println!("Pulsing LED {}...", idx);
                            // Run pulse_led directly without spawning
                            // Note: This will block the command loop during the animation
                            let _ = tokio::runtime::Handle::current().block_on(async {
                                loop_pad_test::pulse_led(&pad, idx, color, 1000).await
                            });
                        }
                        _ => println!("Usage: pulse <idx> [r g b]"),
                    }
                } else {
                    println!("Usage: pulse <idx> [r g b]");
                }
            }
            "rainbow" => {
                println!("Running rainbow animation...");
                // Run rainbow animation directly without spawning
                let _ = tokio::runtime::Handle::current().block_on(async {
                    for _ in 0..3 {
                        for i in 0..9 {
                            let hue = (i as f32 / 9.0) * 360.0;
                            let rgb = hsv_to_rgb(hue, 1.0, 1.0);
                            let _ = pad.clone().set_led(i, rgb);
                            sleep(Duration::from_millis(50)).await;
                        }
                    }
                    pad.clone().set_all_leds(Rgb::WHITE)
                });
            }
            _ => {
                println!("Unknown command: {}. Type 'help' for available commands.", parts[0]);
            }
        }
    }

    Ok(())
}

async fn demo_mode(pad: std::sync::Arc<Pad>, events: Receiver<PadEvent>) -> Result<()> {
    info!("Running demo sequence...");

    // Start event printer in background
    tokio::spawn(async move {
        loop {
            if let Ok(event) = events.recv() {
                println!("[Event] {:?}", event);
            }
        }
    });

    // Demo sequence
    println!("Demo: Setting all LEDs to white");
    pad.clone().set_all_leds(Rgb::WHITE)?;
    sleep(Duration::from_secs(1)).await;

    println!("Demo: Cycling through layers");
    for layer in 0..3 {
        pad.clone().set_layer(layer)?;
        sleep(Duration::from_millis(500)).await;
    }
    pad.clone().set_layer(0)?;

    println!("Demo: LED chase pattern");
    for _ in 0..3 {
        for i in 0..9 {
            pad.clone().set_led(i, Rgb::BLUE)?;
            if i > 0 {
                pad.clone().set_led(i - 1, Rgb::OFF)?;
            }
            sleep(Duration::from_millis(100)).await;
        }
        pad.clone().set_led(8, Rgb::OFF)?;
    }

    println!("Demo: Color fade");
    for i in 0..=10 {
        let factor = i as f32 / 10.0;
        let r = (255.0 * (1.0 - factor)) as u8;
        let g = (255.0 * factor) as u8;
        pad.clone().set_all_leds(Rgb { r, g, b: 0 })?;
        sleep(Duration::from_millis(200)).await;
    }

    println!("Demo: Individual LED colors");
    let colors = [
        Rgb::RED,
        Rgb::GREEN,
        Rgb::BLUE,
        Rgb::WHITE,
        Rgb { r: 255, g: 255, b: 0 }, // Yellow
        Rgb { r: 255, g: 0, b: 255 }, // Magenta
        Rgb { r: 0, g: 255, b: 255 }, // Cyan
        Rgb { r: 128, g: 0, b: 128 }, // Purple
        Rgb { r: 255, g: 128, b: 0 }, // Orange
    ];
    for (i, color) in colors.iter().enumerate() {
        pad.clone().set_led(i as u8, *color)?;
        sleep(Duration::from_millis(200)).await;
    }

    sleep(Duration::from_secs(2)).await;

    println!("Demo: Resetting to white");
    pad.clone().set_all_leds(Rgb::WHITE)?;

    println!("Demo complete!");
    println!("Press Ctrl+C to exit or wait for events...");

    tokio::signal::ctrl_c().await?;
    Ok(())
}

fn print_help() {
    println!("Available commands:");
    println!("  help            - Show this help");
    println!("  quit/exit       - Exit the program");
    println!("  all r g b       - Set all LEDs to RGB color");
    println!("  led idx r g b   - Set LED at index to RGB color");
    println!("  layer n         - Set layer (0=BASE, 1=BUY, 2=SELL)");
    println!("  pulse idx [rgb] - Pulse LED at index");
    println!("  rainbow         - Run rainbow animation");
    println!();
    println!("Examples:");
    println!("  all 255 0 0     - Set all LEDs to red");
    println!("  led 0 0 255 0   - Set first LED to green");
    println!("  layer 1         - Switch to BUY layer");
    println!("  pulse 4         - Pulse LED 4 with white");
}

fn hsv_to_rgb(h: f32, s: f32, v: f32) -> Rgb {
    let c = v * s;
    let x = c * (1.0 - ((h / 60.0) % 2.0 - 1.0).abs());
    let m = v - c;

    let (r, g, b) = if h < 60.0 {
        (c, x, 0.0)
    } else if h < 120.0 {
        (x, c, 0.0)
    } else if h < 180.0 {
        (0.0, c, x)
    } else if h < 240.0 {
        (0.0, x, c)
    } else if h < 300.0 {
        (x, 0.0, c)
    } else {
        (c, 0.0, x)
    };

    Rgb {
        r: ((r + m) * 255.0) as u8,
        g: ((g + m) * 255.0) as u8,
        b: ((b + m) * 255.0) as u8,
    }
}