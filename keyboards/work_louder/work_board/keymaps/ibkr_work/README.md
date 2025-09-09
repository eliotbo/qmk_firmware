# IBKR Work Board Trading Keymap

This keymap is designed for trading applications with ticker tracking and LED feedback integration.

## Features

### Two Layers of Tickers
- **Layer 0**: First set of ticker bindings (blue LED indication)
- **Layer 1**: Second set of ticker bindings (green LED indication)
- Switch between layers using the encoder press (TO(1) and TO(0))

### Key Bindings
All keys send `Alt+Character` combinations that can be bound to tickers in your Rust trading application:
- Number row: Alt+1 through Alt+=
- QWERTY rows: Alt+Q through Alt+/
- Each key can represent a different ticker symbol

### HID LED Control Integration
The keyboard supports bidirectional communication with your trading app via Raw HID:

#### Commands from App to Keyboard:
1. **CMD_SET_ALL (0x01)**: Set all LEDs to a specific color
   - Format: `[0x01, R, G, B]`
   
2. **CMD_SET_ONE (0x02)**: Set a specific LED color
   - Format: `[0x02, led_index, R, G, B]`
   - LED indices 0-47 correspond to physical keys
   
3. **CMD_CLEAR (0x03)**: Clear HID override, return to layer colors
   - Format: `[0x03]`
   
4. **CMD_FOCUS (0x04)**: Focus on one ticker (bright white) and dim others
   - Format: `[0x04, led_index]`
   - Focused LED: bright white (255, 255, 255)
   - Other LEDs: dim blue (0, 0, 30)

#### Notifications from Keyboard to App:
1. **Key Press (0xFE)**: Sent when a key is pressed
   - Format: `[0xFE, led_index, current_layer]`
   
2. **Layer Change (0xFD)**: Sent when layer changes
   - Format: `[0xFD, new_layer]`
   
3. **Command ACK (0xFF)**: Acknowledges received commands
   - Format: `[0xFF, received_command]`

## LED Mapping

The Work Board has 48 keys arranged in a 4x13 grid (with some positions unused):

```
Row 0 (indices 0-11):  [1] [2] [3] [4] [5] [6] [7] [8] [9] [0] [-] [=]
Row 1 (indices 12-23): [Q] [W] [E] [R] [T] [Y] [U] [I] [O] [P] [[] []]
Row 2 (indices 24-34): [A] [S] [D] [F] [G] [H] [J] [K] [L] [;] [']
Row 3 (indices 35-46): [Z] [X] [C] [V] [B] [N] [M] [,] [.] [/]
```

## Usage Example

### Rust App Integration
```rust
// Set focus on ticker at position 5 (key '6')
send_hid_command(&[0x04, 5])?;

// When user presses a key, keyboard sends:
// [0xFE, led_index, layer] - App can then focus that ticker

// Clear all LED overrides
send_hid_command(&[0x03])?;
```

### Workflow
1. User presses a key (e.g., Alt+5)
2. Keyboard sends HID notification with LED index
3. App focuses on corresponding ticker
4. App sends CMD_FOCUS to highlight that ticker's LED
5. Keyboard lights up focused ticker bright white, dims others

## Building and Flashing

### Prerequisites
- QMK environment set up
- Work Board connected in bootloader mode

### Build Command
```bash
# From QMK root directory
make work_louder/work_board:ibkr_work

# Or with Docker
./util/docker_build.sh work_louder/work_board:ibkr_work
```

### Flash Command
```bash
make work_louder/work_board:ibkr_work:flash
```

## Configuration

### config.h
- RGB Matrix configured for solid colors
- Raw HID enabled with standard usage page (0xFF60)
- Encoder resolution set to 4

### rules.mk
- RAW_ENABLE = yes (for HID communication)
- RGB_MATRIX_ENABLE = yes (for per-key LED control)
- ENCODER_ENABLE = yes (for layer switching)
- LTO_ENABLE = yes (for size optimization)

## Notes

- The keyboard maintains LED state persistently via the `hid_led_colors` array
- LED updates are applied in `rgb_matrix_indicators_user()` for continuous refresh
- Layer colors (blue/green) are shown when not in HID override mode
- All Alt+Character combinations are non-interfering with normal system shortcuts