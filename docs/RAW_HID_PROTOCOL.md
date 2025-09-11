# RAW HID Protocol Documentation

**Version:** 1.0

## Overview

This document describes the RAW HID protocol used for communication between the Work Louder Loop keyboard (running IBKR trading keymap) and host applications. The protocol replaces traditional F-key emissions with vendor-specific HID messages for cleaner, more reliable control.

## Configuration

- **Vendor ID:** 0xFEED
- **Product ID:** 0x6060
- **Usage Page:** 0xFF60
- **Usage:** 0x61
- **Buffer Size:** 32 bytes

## Message Format

All messages follow a fixed format:
- **Byte 0:** Message type ID
- **Bytes 1-31:** Payload (message-specific)

## Device → Host Events

### 0x10 - Button Event (EV_BTN)
Sent when a key is pressed or released.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Message Type | 0x10 |
| 1 | Layer | 0=BASE, 1=BUY, 2=SELL |
| 2 | Button Index | 0-8 (physical position) |
| 3 | Action | 1=press, 0=release |

### 0x11 - Encoder Rotation (EV_ENC)
Sent when an encoder is rotated.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Message Type | 0x11 |
| 1 | Encoder Index | 0-2 |
| 2 | Delta | int8 (+1 / -1 per detent) |

### 0x12 - Encoder Press (EV_ENC_P)
Sent when an encoder is pressed.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Message Type | 0x12 |
| 1 | Encoder Index | 0-2 |
| 2 | Press Type | 1=short (<500ms), 2=long (≥500ms) |

### 0x13 - Layer Change (EV_LAYER)
Sent when the active layer changes.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Message Type | 0x13 |
| 1 | Layer | 0=BASE, 1=BUY, 2=SELL |

### 0x7E - Hello/Boot (EV_HELLO)
Sent on device boot/initialization.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Message Type | 0x7E |
| 1 | Protocol Version | 1 |
| 2 | Firmware Major | 1 |
| 3 | Firmware Minor | 0 |

## Host → Device Commands

### 0x01 - Set All LEDs (CMD_SET_ALL)
Sets all 9 LEDs to the same color.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Command Type | 0x01 |
| 1 | Red | 0-255 |
| 2 | Green | 0-255 |
| 3 | Blue | 0-255 |

*Note: Only affects BASE layer. Other layers override with their own colors.*

### 0x02 - Set Single LED (CMD_SET_ONE)
Sets a specific LED color.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Command Type | 0x02 |
| 1 | LED Index | 0-8 |
| 2 | Red | 0-255 |
| 3 | Green | 0-255 |
| 4 | Blue | 0-255 |

*Note: Only affects BASE layer. Other layers override with their own colors.*

### 0x03 - Set Mode/Layer (CMD_SET_MODE)
Changes the active layer.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Command Type | 0x03 |
| 1 | Layer | 0=BASE, 1=BUY, 2=SELL |

### 0x7D - Host Ready (CMD_HOST_READY)
Handshake message indicating host is ready to receive events.

| Byte | Description | Values |
|------|-------------|--------|
| 0 | Command Type | 0x7D |
| 1 | Protocol Version | 1 |

## Physical Layout Mapping

The 3×3 grid maps to button indices as follows:

```
[0] [1] [2]
[3] [4] [5]
[6] [7] [8]
```

Encoders are indexed 0-2 from left to right.

## Layer Behavior

- **BASE (0):** White LEDs (unless overridden by host)
- **BUY (1):** Green LEDs (host LED commands ignored)
- **SELL (2):** Red LEDs (host LED commands ignored)

LED commands from the host only affect the BASE layer. When switching to BUY or SELL layers, the layer-specific colors override any host-set colors.

## Timing and Debouncing

- **Device debounce:** 5ms (configured in firmware)
- **Long press threshold:** 500ms
- **Recommended host debounce:** 10ms minimum
- **Host throttling:** Process events at reasonable rates to avoid overwhelming the application

## Legacy Compatibility

The firmware can be compiled with `LEGACY_FKEY_COMPAT = 1` to enable F-key fallback when no host is connected. This is disabled by default as it can cause focus leaks.

## Example Communication Flow

1. Device boots → sends `EV_HELLO` 
2. Host application connects → sends `CMD_HOST_READY`
3. User presses button 4 on BASE layer:
   - Device sends: `[0x10, 0x00, 0x04, 0x01]` (press)
   - Device sends: `[0x10, 0x00, 0x04, 0x00]` (release)
4. User rotates encoder 0 clockwise:
   - Device sends: `[0x11, 0x00, 0x01]`
5. Host sets all LEDs to blue:
   - Host sends: `[0x01, 0x00, 0x00, 0xFF]`

## Notes

- All multi-byte values are little-endian
- Unused bytes in the 32-byte buffer should be zero-filled
- The protocol is designed to be extensible; unknown message types should be ignored