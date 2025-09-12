# Loop Pad Test

A Rust test crate for interfacing with the Work Louder Loop Pad keyboard via RAW HID protocol.

## Features

- **RAW HID Protocol v1** support for bidirectional communication
- **Mock mode** for testing without hardware
- Interactive CLI tool (`pad-tester`) for testing and debugging
- LED control (individual and all LEDs)
- Layer switching
- Event streaming with typed API

## Quick Start

### Prerequisites

- Rust 1.70+ with Cargo
- Linux: udev rules for USB access (see below)
- Windows/macOS: No special setup required

### Installation

```bash
# Clone the repository
cd padloop2

# Build the project
cargo build --release

# Run with mock mode (no hardware needed)
cargo run --bin pad-tester -- --mock

# Run with real hardware
cargo run --bin pad-tester
```

### Linux Setup

Add udev rules for USB access without sudo:

```bash
# Create udev rule
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="574c", ATTRS{idProduct}=="1df9", MODE="0666"' | \
    sudo tee /etc/udev/rules.d/99-loop-pad.rules

# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

## Usage

### Command Line Interface

```bash
# Listen mode - just print events
cargo run --bin pad-tester -- listen

# REPL mode - interactive commands
cargo run --bin pad-tester -- repl

# Demo mode - run LED animations
cargo run --bin pad-tester -- demo

# Mock mode - no hardware required
cargo run --bin pad-tester -- --mock demo

# Verbose logging
cargo run --bin pad-tester -- --verbose
```

### REPL Commands

When in REPL mode, the following commands are available:

- `help` - Show available commands
- `quit` / `exit` - Exit the program
- `all r g b` - Set all LEDs to RGB color (0-255 each)
- `led idx r g b` - Set specific LED (0-8) to RGB color
- `layer n` - Switch layer (0=BASE, 1=BUY, 2=SELL)
- `pulse idx [r g b]` - Pulse animation on specific LED
- `rainbow` - Run rainbow animation across all LEDs

### Library Usage

```rust
use loop_pad_test::{Config, Pad, PadEvent, Rgb};

#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Load configuration
    let config = Config::load()?;
    
    // Create pad interface
    let mut pad = Pad::new(&config)?;
    let events = pad.spawn(&config)?;
    
    // Send commands
    pad.send_ready()?;
    pad.set_all_leds(Rgb::WHITE)?;
    pad.set_layer(0)?;
    
    // Process events
    loop {
        match events.recv() {
            Ok(PadEvent::Button { layer, idx, pressed }) => {
                println!("Button {} on layer {}: {}", idx, layer, 
                    if pressed { "pressed" } else { "released" });
            }
            Ok(event) => println!("Event: {:?}", event),
            Err(_) => break,
        }
    }
    
    Ok(())
}
```

## Configuration

Edit `padtest.toml` to configure the device:

```toml
[hid]
vid = 0x574C        # Work Louder vendor ID
pid = 0x1DF9        # Loop product ID
usage_page = 0xFF60 # Vendor-specific usage page
usage = 0x61        # Vendor-specific usage



[demo]
start_layer = 0     # Initial layer (0-2)
arm_timeout_seconds = 30

[demo.start_color]
r = 255
g = 255
b = 255
```

## Features

### Cargo Features

- `hid` (default): Enable HID support
- `mock`: Enable mock mode for testing

```bash
# Build with only mock support 
cargo build --no-default-features --features mock
```

## Protocol Documentation

See [PROTOCOL.md](PROTOCOL.md) for detailed RAW HID protocol specification.

## Troubleshooting

### Device Not Found

1. Check USB connection
2. Verify VID/PID in `padtest.toml` matches your device
3. On Linux, ensure udev rules are set up
4. Try with `--verbose` flag for debug output

### Permission Denied (Linux)

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in for changes to take effect
```

### No Events Received

1. Verify the firmware is flashed with RAW HID support
2. Check that no other application is using the device
3. Try mock mode to verify the software works: `--mock`

### Windows Issues

If HID writes fail, the crate automatically handles the Windows-specific report ID requirement.

## Development

### Running Tests

```bash
# Run all tests
cargo test

# Run with mock feature
cargo test --features mock

# Run specific test
cargo test test_parse_button_press
```

### Building Documentation

```bash
cargo doc --open
```

## License

MIT OR Apache-2.0