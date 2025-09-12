# RAW HID Protocol v1 Specification

This document describes the RAW HID protocol used for communication between the host application and the Loop Pad keyboard.

## Overview

- **Protocol Version**: 1
- **Buffer Size**: 32 bytes (fixed)
- **Communication**: Bidirectional
- **Vendor ID**: 0x574C (Work Louder)
- **Product ID**: 0x1DF9 (Loop)
- **Usage Page**: 0xFF60 (Vendor-specific)
- **Usage**: 0x61

## Message Format

All messages are 32 bytes with zero-padding:
- **Byte 0**: Message type/command ID
- **Bytes 1-31**: Payload (message-specific, zero-padded)

## Device → Host Events

### Button Event (0x10)
Sent when a button is pressed or released.

```
[0x10, layer, btn_idx, action]
```

- `layer`: Current layer (0=BASE, 1=BUY, 2=SELL)
- `btn_idx`: Button index (0-8)
- `action`: 1 = pressed, 0 = released

### Encoder Rotation (0x11)
Sent when an encoder is rotated.

```
[0x11, enc_idx, delta]
```

- `enc_idx`: Encoder index (0-2)
- `delta`: Rotation delta as signed int8 (+1 CW, -1 CCW per detent)

### Encoder Press (0x12)
Sent when an encoder is pressed.

```
[0x12, enc_idx, type]
```

- `enc_idx`: Encoder index (0-2)
- `type`: 1 = short press (<500ms), 2 = long press (≥500ms)

### Layer Change (0x13)
Sent when the active layer changes.

```
[0x13, layer]
```

- `layer`: New active layer (0=BASE, 1=BUY, 2=SELL)

### Boot Hello (0x7E)
Sent on device boot/connection.

```
[0x7E, proto_ver, fw_major, fw_minor]
```

- `proto_ver`: Protocol version (currently 1)
- `fw_major`: Firmware major version
- `fw_minor`: Firmware minor version

## Host → Device Commands

### Set All LEDs (0x01)
Sets all LEDs to the same color (BASE layer only).

```
[0x01, r, g, b]
```

- `r`: Red value (0-255)
- `g`: Green value (0-255)
- `b`: Blue value (0-255)

**Note**: Only works when layer = 0 (BASE). Other layers force their own colors.

### Set One LED (0x02)
Sets a single LED color (BASE layer only).

```
[0x02, led_idx, r, g, b]
```

- `led_idx`: LED index (0-8)
- `r`: Red value (0-255)
- `g`: Green value (0-255)
- `b`: Blue value (0-255)

**Note**: Only works when layer = 0 (BASE).

### Set Layer (0x03)
Changes the active layer.

```
[0x03, layer]
```

- `layer`: Target layer (0=BASE, 1=BUY, 2=SELL)

### Host Ready (0x7D)
Handshake acknowledgment from host.

```
[0x7D, proto_ver]
```

- `proto_ver`: Protocol version supported by host (currently 1)

## LED Behavior by Layer

- **Layer 0 (BASE)**: Host-controllable via 0x01/0x02 commands, default white
- **Layer 1 (BUY)**: Forced green (ignores host LED commands)
- **Layer 2 (SELL)**: Forced red (ignores host LED commands)

## Timing Specifications

- **Short press**: < 500ms
- **Long press**: ≥ 500ms
- **Device debounce**: 5ms
- **Recommended host debounce**: 10ms
- **Auto-disarm timeout**: Configurable (default 30s)

## Encoder Press Semantics

The protocol defines these encoder press behaviors:

- **Encoder 0 (Shares/Position)**:
  - Short: Cycle modes [Shares → %BP → Risk$]
  - Long: Reset to default mode

- **Encoder 1 (Stop Offset)**:
  - Short: Cycle modes [Ticks → % → ATR]
  - Long: Toggle Trailing/Fixed stop

- **Encoder 2 (Limit Price)**:
  - Short: Cycle anchor [Bid → Mid → Ask]
  - Long: Toggle Non-Crossing mode

## Physical Layout

```
[Enc0]  [Enc1]  [Enc2]
[Btn0]  [Btn1]  [Btn2]
[Btn3]  [Btn4]  [Btn5]
[Btn6]  [Btn7]  [Btn8]
```

## Implementation Notes

### Windows Compatibility
On Windows, HID writes may require prepending a report ID byte (0x00):
```
[0x00, actual_32_byte_message]
```
This is handled automatically by the implementation.

### Error Handling
- Unknown message types should be ignored
- Malformed messages should be logged but not fatal
- Device disconnection requires re-handshake on reconnect
- Host should retry handshake if no hello received within 1s

### Protocol Advantages
- **No OS interference**: Vendor-specific HID bypasses keyboard handling
- **Bidirectional**: Full duplex communication
- **Layer context**: Every button event includes layer state
- **Extensible**: Protocol version allows future enhancements
- **Reliable**: No keystroke leaks to other applications

## Example Communication Flow

1. **Device Boot**:
   ```
   Device → Host: [0x7E, 0x01, 0x01, 0x00]  // Hello, proto v1, fw v1.0
   Host → Device: [0x7D, 0x01]               // Host ready, proto v1
   ```

2. **Button Press**:
   ```
   Device → Host: [0x10, 0x00, 0x03, 0x01]   // Layer 0, button 3 pressed
   Device → Host: [0x10, 0x00, 0x03, 0x00]   // Layer 0, button 3 released
   ```

3. **LED Control**:
   ```
   Host → Device: [0x01, 0xFF, 0x00, 0x00]   // All LEDs red
   Host → Device: [0x02, 0x04, 0x00, 0xFF, 0x00] // LED 4 green
   ```

4. **Layer Change**:
   ```
   Host → Device: [0x03, 0x01]               // Switch to layer 1 (BUY)
   Device → Host: [0x13, 0x01]               // Confirm layer changed
   ```

## Testing

Use the `pad-tester` tool to test protocol implementation:

```bash
# Listen for events
cargo run --bin pad-tester -- listen

# Interactive REPL
cargo run --bin pad-tester -- repl

# Mock mode (no hardware)
cargo run --bin pad-tester -- --mock
```