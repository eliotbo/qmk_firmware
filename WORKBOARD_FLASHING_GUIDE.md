# Work Louder Work Board Firmware Flashing Guide

see 3d model for the keys here
https://www.printables.com/model/1066117-choc-louder-keycaps-choc-and-mx-spacing/files

## Quick Start (Environment Already Setup)

Put keyboard in bootloader mode by pressing on the top-left + space keys of the work board while plugging in the cable.

here are the essential commands:

```bash
# 1. Activate virtual environment
source qmk_venv/bin/activate

# 2. Compile firmware (choose one)
qmk compile -kb work_louder/work_board -km eliot_midi  # Your MIDI keymap
qmk compile -kb work_louder/work_board -km via         # VIA-compatible keymap

# 3. Enter bootloader mode
# Press the hardware reset button on the PCB

# 4. Flash the firmware
sudo dfu-programmer atmega32u4 erase
sudo dfu-programmer atmega32u4 flash work_louder_work_board_rev3_via.hex  # or your .hex file
sudo dfu-programmer atmega32u4 reset
```

That's it! Your keyboard should now be running the new firmware.

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

## Compiling Firmware

### Option A: Compile Your Custom Keymap
```bash
qmk compile -kb work_louder/work_board -km eliot_midi
```

### Option B: Compile VIA-Compatible Firmware
```bash
qmk compile -kb work_louder/work_board -km via
```

The compiled `.hex` file will be created in the qmk_firmware root directory.

## Flashing Firmware

### Step 1: Enter Bootloader Mode
Since the keyboard may not have accessible key combos, use the **hardware reset button** on the PCB.

### Step 2: Verify Bootloader Detection
```bash
lsusb | grep -i atmel
# Should show: Bus 001 Device XXX: ID 03eb:2ff4 Atmel Corp. atmega32u4 DFU bootloader
```

### Step 3: Flash the Firmware
Use dfu-programmer directly (requires sudo on Linux):

```bash
sudo dfu-programmer atmega32u4 erase
sudo dfu-programmer atmega32u4 flash work_louder_work_board_rev3_via.hex
sudo dfu-programmer atmega32u4 reset
```

Replace `work_louder_work_board_rev3_via.hex` with your actual firmware filename.

## Keyboard Information

- **MCU**: ATmega32U4
- **Bootloader**: atmel-dfu
- **Board**: Work Louder Work Board Rev3

## Available Keymaps

- `eliot_midi` - Custom MIDI controller with layers for notes, drums, and session control
- `eliot` - Standard keyboard layout
- `via` - VIA-compatible for real-time remapping using [usevia.app](https://usevia.app)

## Troubleshooting

- **"qmk: command not found"**: Make sure your virtual environment is activated: `source qmk_venv/bin/activate`
- **"avr-gcc: not found"**: Install AVR toolchain: `sudo apt install gcc-avr avr-libc`
- **Bootloader not detected**: Press the hardware reset button on the PCB
- **Permission denied**: Use `sudo` with dfu-programmer commands

## Notes

- The bootloader is protected and cannot be overwritten by firmware flashing
- You can always re-enter bootloader mode using the hardware reset button
- Bootloader mode times out after ~8 seconds, so flash quickly after entering it