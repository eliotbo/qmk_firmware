# Loop Pad HID Integration Guide

This guide explains how to control the Work Louder Loop Pad's RGB LEDs from a Rust application using HID (Human Interface Device) communication.

## Overview

The Loop Pad is a 3x3 mechanical keyboard with RGB LEDs that can be controlled via QMK firmware. This integration allows external applications to:
- Set all LEDs to a specific color
- Control individual LED colors
- Switch between keyboard layers (modes)

## Prerequisites

### System Requirements
- Ubuntu 24.04 or similar Linux distribution
- Rust toolchain (edition 2024)
- USB HID libraries

### Required System Libraries
```bash
sudo apt-get update && sudo apt-get install -y libudev-dev libusb-1.0-0-dev
```

should be already installed both on the host and the container (where most claude code instances are located)

### USB Device Permissions
By default, Linux restricts access to HID devices. You have two options:

#### Option 1: Create a udev rule (Recommended)
```bash
echo 'SUBSYSTEM=="hidraw", ATTRS{idVendor}=="574c", ATTRS{idProduct}=="1df9", MODE="0666"' | sudo tee /etc/udev/rules.d/99-loop-pad.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```
Then unplug and replug your Loop Pad.

#### Option 2: Run with sudo (Quick testing)
```bash
cargo build
sudo ./target/debug/loop_sync_test
```

## QMK Firmware Setup

### 1. Raw HID Handler Implementation

Add this to your QMK keymap.c file:

```c
#include "raw_hid.h"

// Raw HID command definitions
enum hid_commands {
    CMD_SET_ALL  = 1, // [1, r, g, b]
    CMD_SET_ONE  = 2, // [2, led_index, r, g, b]
    CMD_SET_MODE = 3, // [3, mode] (e.g., 0=BASE,1=BUY,2=SELL)
};

// Store individual LED colors when in HID override mode
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

led_color_t hid_all_color = {0, 0, 0};
led_color_t hid_led_colors[9] = {{0}};  // Store colors for 9 LEDs
bool hid_individual_leds = false;  // Track if we're using individual LED control
bool hid_rgb_override = false;     // Track if HID is controlling RGB

// Track current layer
uint8_t current_layer = 0;

void raw_hid_receive(uint8_t *data, uint8_t length) {
    switch (data[0]) {
        case CMD_SET_ALL:
            if (length >= 4 && current_layer == 0) {  // Only work in BASE layer
                hid_rgb_override = true;
                hid_individual_leds = false;
                hid_all_color.r = data[1];
                hid_all_color.g = data[2];
                hid_all_color.b = data[3];
                // Reset individual LED colors when using SET_ALL
                for (uint8_t i = 0; i < 9; i++) {
                    hid_led_colors[i] = hid_all_color;
                }
            }
            break;

        case CMD_SET_ONE:
            if (length >= 5 && current_layer == 0) {
                hid_rgb_override = true;
                hid_individual_leds = true;
                uint8_t led_index = data[1];
                if (led_index < 9) {
                    hid_led_colors[led_index].r = data[2];
                    hid_led_colors[led_index].g = data[3];
                    hid_led_colors[led_index].b = data[4];
                }
            }
            break;

        case CMD_SET_MODE:
            if (length >= 2) {
                uint8_t mode = data[1];
                if (mode <= 2) { 
                    layer_move(mode);
                    // Only clear HID override when leaving BASE layer
                    if (mode != 0) {
                        hid_rgb_override = false;
                    }
                }
            }
            break;
    }
}
```

### 2. RGB Matrix Indicator Function

**CRITICAL**: The RGB matrix system refreshes continuously. You MUST apply colors in `rgb_matrix_indicators_user()` or they will be overwritten immediately:

```c
void rgb_matrix_indicators_user(void) {
    if (hid_rgb_override) {
        if (hid_individual_leds) {
            // Apply stored individual LED colors
            for (uint8_t i = 0; i < 9; i++) {
                rgb_matrix_set_color(i, hid_led_colors[i].r, hid_led_colors[i].g, hid_led_colors[i].b);
            }
        } else {
            // Apply the same color to all LEDs
            rgb_matrix_set_color_all(hid_all_color.r, hid_all_color.g, hid_all_color.b);
        }
        return;
    }
    
    // Only apply layer colors when NOT in HID override mode
    switch (current_layer) {
        case 0:
            rgb_matrix_set_color_all(RGB_WHITE);
            break;
        case 1:
            rgb_matrix_set_color_all(RGB_GREEN);
            break;
        case 2:
            rgb_matrix_set_color_all(RGB_RED);
            break;
    }
}
```

### 3. Layer State Management

```c
layer_state_t layer_state_set_user(layer_state_t state) {
    current_layer = get_highest_layer(state);
    
    // Clear HID override when leaving BASE layer
    if (current_layer != 0) {
        hid_rgb_override = false;
    }
    
    return state;
}
```

## Rust Application Setup

### 1. Cargo.toml Configuration

```toml
[package]
name = "loop_sync_test"
version = "0.1.0"
edition = "2024"

[workspace]

[dependencies]
hidapi = "2.6"
anyhow = "1.0"
```

### 2. Finding the Correct HID Interface

The Loop Pad exposes multiple HID interfaces. You need to find the RAW HID interface:

```rust
use hidapi::HidApi;

const VID: u16 = 0x574C;  // Work Louder vendor ID
const PID: u16 = 0x1DF9;  // Loop Pad product ID

fn find_raw_hid_device() -> Result<()> {
    let api = HidApi::new()?;
    
    for device in api.device_list() {
        if device.vendor_id() == VID && device.product_id() == PID {
            println!("Found interface:");
            println!("  Usage Page: 0x{:04X}", device.usage_page());
            println!("  Usage: 0x{:04X}", device.usage());
            
            // QMK Raw HID typically uses usage_page 0xFF60 and usage 0x61
            if device.usage_page() == 0xFF60 && device.usage() == 0x61 {
                println!("  -> This is the RAW HID interface!");
                let dev = device.open_device(&api)?;
                // Use this device for communication
                return Ok(());
            }
        }
    }
    
    // Fallback to regular open if specific interface not found
    let dev = api.open(VID, PID)?;
    Ok(())
}
```

### 3. Sending HID Commands

```rust
fn send_all(r: u8, g: u8, b: u8) -> Result<()> {
    let api = HidApi::new()?;
    
    // Find and open the RAW HID interface
    for device in api.device_list() {
        if device.vendor_id() == VID && device.product_id() == PID {
            if device.usage_page() == 0xFF60 && device.usage() == 0x61 {
                let dev = device.open_device(&api)?;
                
                let mut buf = [0u8; 32];  // QMK Raw HID uses 32-byte packets
                buf[0] = 1;  // CMD_SET_ALL
                buf[1] = r;
                buf[2] = g;
                buf[3] = b;
                
                dev.write(&buf)?;
                return Ok(());
            }
        }
    }
    
    // Fallback
    let dev = api.open(VID, PID)?;
    let mut buf = [0u8; 32];
    buf[0] = 1;
    buf[1] = r;
    buf[2] = g;
    buf[3] = b;
    dev.write(&buf)?;
    Ok(())
}
```

## Common Pitfalls and Solutions

### 1. Permission Denied Error
**Problem**: `Error: hidapi error: Failed to open a device with path '/dev/hidraw13': Permission denied`
**Solution**: Create udev rule or run with sudo (see USB Device Permissions section)

### 2. Connection Timeout
**Problem**: `Error: hidapi error: Connection timed out`
**Solutions**:
- Make sure you're using 32-byte buffers (not 33)
- Don't include a report ID byte at the beginning
- Verify the device isn't being used by another program

### 3. Individual LEDs Not Working
**Problem**: Individual LED commands seem to have no effect
**Solution**: Colors must be stored and continuously applied in `rgb_matrix_indicators_user()`. The RGB matrix refreshes constantly and will overwrite one-time color sets.

### 4. Rapid Color Changes Not Visible
**Problem**: Fast color cycling doesn't show all colors
**Solutions**:
- Use at least 250-500ms delays between color changes
- The RGB matrix refresh rate limits how fast changes can be displayed
- Ensure you're in BASE mode (layer 0) for HID control to work

### 5. Colors Reset to White/Layer Color
**Problem**: LEDs keep reverting to white or layer colors
**Solution**: Check that `hid_rgb_override` is properly managed:
- Set to `true` when receiving HID commands in BASE layer
- Only clear when switching to non-BASE layers
- Don't clear when switching TO BASE layer

### 6. Protocol Error with Invalid LED Index
**Problem**: `Error: hidapi error: Protocol error`
**Solution**: Loop Pad has exactly 9 LEDs (indices 0-8). Using index 9 or higher will cause errors.

### 7. Build Errors with hidapi
**Problem**: `Unable to find libudev`
**Solution**: Install required system libraries (see Prerequisites)

## Testing Your Integration

Here's a simple test sequence:

```rust
fn test_loop_pad() -> Result<()> {
    // 1. Set mode to BASE for HID control
    set_mode(0)?;
    thread::sleep(Duration::from_millis(500));
    
    // 2. Test setting all LEDs
    println!("Testing all LEDs - RED");
    send_all(255, 0, 0)?;
    thread::sleep(Duration::from_millis(1000));
    
    // 3. Test individual LEDs
    println!("Testing individual LEDs");
    for i in 0..9 {
        send_one(i, 0, 255, 0)?;  // Green
        thread::sleep(Duration::from_millis(500));
    }
    
    // 4. Test mode switching
    println!("Testing BUY mode (should be green)");
    set_mode(1)?;
    thread::sleep(Duration::from_millis(1000));
    
    println!("Testing SELL mode (should be red)");
    set_mode(2)?;
    thread::sleep(Duration::from_millis(1000));
    
    // 5. Return to BASE
    set_mode(0)?;
    
    Ok(())
}
```

## Important Notes

1. **HID control only works in BASE layer (layer 0)** - This is by design to prevent interference during trading operations
2. **Buffer size is exactly 32 bytes** - QMK Raw HID standard
3. **No report ID needed** - Start command at buf[0], not buf[1]
4. **Colors must be continuously applied** - Use `rgb_matrix_indicators_user()` to maintain state
5. **The Loop Pad will show multiple HID interfaces** - Look for usage_page 0xFF60 and usage 0x61

## Debugging Tips

1. Add debug output to identify which HID interface you're using
2. Test with slower delays first (500ms+) to ensure commands are working
3. Verify you're in BASE mode before testing LED control
4. Check that your udev rules are applied (unplug/replug device)
5. Use `cargo run` to see detailed error messages during development