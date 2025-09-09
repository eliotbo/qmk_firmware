# Work Louder Loop IBKR Trading Firmware Flashing Guide

## Quick Start (Environment Already Setup)

**IMPORTANT**: This keymap enables MIDI functionality for trading controls. Your OS must recognize the device as both a keyboard AND a MIDI device.

### Essential Commands:

```bash
# 1. Activate virtual environment
source qmk_venv/bin/activate

# 2. Compile the IBKR trading firmware
qmk compile -kb work_louder/loop -km ibkr

# 3. Enter bootloader mode
# Hold the top-left encoder while plugging in the USB cable
# OR press the reset button on the back of the PCB

# 4. Flash the firmware
sudo dfu-programmer atmega32u4 erase
sudo dfu-programmer atmega32u4 flash work_louder_loop_rev3_ibkr.hex
sudo dfu-programmer atmega32u4 reset
```

Your Loop pad is now ready for trading!

## Prerequisites

### 1. Install Required System Packages
```bash
sudo apt install gcc-avr avr-libc dfu-programmer
```

### 2. Setup Python Environment
```bash
# Create virtual environment
python3 -m venv qmk_venv
source qmk_venv/bin/activate

# Install QMK CLI
pip install qmk
```

### 3. Initialize QMK Repository
```bash
# From the qmk_firmware directory
make git-submodule  # Downloads required libraries
qmk setup          # Choose option 3 to keep existing code
```

## Compiling the IBKR Trading Firmware

```bash
# Navigate to QMK directory
cd qmk_firmware

# Compile the firmware
qmk compile -kb work_louder/loop -km ibkr

# Or use make directly
make work_louder/loop:ibkr
```

The compiled `.hex` file will be created in the qmk_firmware root directory.

## Flashing Firmware

### Step 1: Enter Bootloader Mode

**Method 1: Hardware Reset Button**
- Flip the Loop pad over
- Press the small reset button on the PCB

**Method 2: Bootmagic (Recommended)**
- Unplug the USB cable
- Hold down the **top-left encoder button**
- Plug in the USB cable while holding the button
- Release after 1 second

### Step 2: Verify Bootloader Detection
```bash
lsusb | grep -i atmel
# Should show: Bus XXX Device XXX: ID 03eb:2ff4 Atmel Corp. atmega32u4 DFU bootloader
```

### Step 3: Flash the Firmware
```bash
sudo dfu-programmer atmega32u4 erase
sudo dfu-programmer atmega32u4 flash work_louder_loop_ibkr.hex
sudo dfu-programmer atmega32u4 reset
```

## Post-Flash Setup

### 1. Verify MIDI Device Recognition

**Linux:**
```bash
# Check MIDI devices
aconnect -l
# Should show "Work Louder Loop" as a MIDI device
```

**Windows:**
- Open Device Manager
- Look for "Work Louder Loop" under both:
  - Keyboards
  - Sound, video and game controllers

**macOS:**
- Open Audio MIDI Setup
- Click "Window" → "Show MIDI Studio"
- Look for "Work Louder Loop" device

### 2. Test the Layers

The RGB lighting indicates the current layer:
- **White**: Base layer (general trading)
- **Green**: Buy layer 
- **Red**: Sell layer

Press the bottom-right key (position 8) to cycle through layers.

### 3. Configure Your Trading Software

1. **Set up MIDI mappings** for encoder rotations:
   - CC 20/21: Share quantity
   - CC 22/23: Stop loss
   - CC 24/25: Limit price

2. **Map F-keys** in your trading software:
   - F13-F20: Base layer actions
   - F21-F28: Buy-specific actions
   - F29-F35, Shift+F35: Sell-specific actions
   - Alt+F13-F21: Encoder press actions

## Trading Key Layout Reference

```
Physical Layout:
 [Enc0]    [Enc1]    [Enc2]
   ↓         ↓         ↓
 Shares   StopLoss   Limit
 
 [F13] [F14] [F15]  ← Base Layer
 [F16] [F17] [F18]  ← Base Layer
 [F19] [F20] [Layer Switch]
```

## IBKR Keymap Features

- **3 Trading Layers**: Base (White), Buy (Green), Sell (Red)
- **MIDI Encoders**: Smooth control for quantities and prices
- **Layer-Aware MIDI**: Different channels per layer (1/2/3)
- **33 Unique Actions**: 24 keys + 9 encoder presses
- **Shift Acceleration**: Hold shift while turning encoders for 10x speed

## Troubleshooting

### Common Issues:

**"qmk: command not found"**
```bash
source qmk_venv/bin/activate
```

**"avr-gcc: not found"**
```bash
sudo apt install gcc-avr avr-libc
```

**Bootloader not detected**
- Try the hardware reset button on the PCB
- Bootloader mode times out after ~8 seconds, so flash quickly

**MIDI not working**
- Ensure MIDI_ENABLE = yes in rules.mk
- Check if OS recognizes the device as MIDI
- Try unplugging and replugging the device

**Wrong F-keys being sent**
- The firmware defines F25-F35 which aren't standard
- Your OS/software must support extended F-keys
- Test with a key tester first

## Safety Warning

⚠️ **IMPORTANT FOR TRADING USE**:
1. Test ALL mappings in a demo/paper trading account first
2. Verify each button does what you expect
3. Keep a backup keyboard/mouse ready
4. Never trade with untested hardware configurations
5. Consider adding software-side confirmation dialogs for critical actions

## Notes

- The bootloader cannot be overwritten by firmware flashing
- You can always re-enter bootloader mode using the reset button
- The MIDI implementation uses channels 1-3 for layer distinction
- F25-F35 are custom defined and may not work with all software

## Support

For issues specific to the IBKR keymap, check:
- `/keyboards/work_louder/loop/keymaps/ibkr/readme.md`
- The keymap source at `keymap.c`

For general QMK issues:
- [QMK Documentation](https://docs.qmk.fm)
- [Work Louder Support](https://worklouder.cc/support)