# IBKR Trading Keymap for Work Louder Loop

A specialized keymap designed for live trading with Interactive Brokers or similar trading platforms. Uses MIDI CC messages for encoder control, allowing precise binding in trading software.

## Features

### 3 Layers with MIDI Channels
- **Base Layer** (White LED): Common trading functions - MIDI Channel 1
- **Buy Layer** (Green LED): Buy-specific actions - MIDI Channel 2
- **Sell Layer** (Red LED): Sell-specific actions - MIDI Channel 3

### Layout
```
Physical Layout (3x3 grid + 3 encoders):

 [Enc0]    [Enc1]    [Enc2]
 
 [0] [1] [2]
 [3] [4] [5]  
 [6] [7] [8←Layer Switch]
```

### Encoder Control

Encoder rotation uses MIDI CC for smooth control, while presses send Alt+F-keys:

1. **Encoder 0 - Share Quantity**
   - Rotate CW: MIDI CC 21 (increment shares)
   - Rotate CCW: MIDI CC 20 (decrement shares)
   - Press: Alt+F13 (Base) / Alt+F14 (Buy) / Alt+F15 (Sell) - preset share quantities
   - Hold Shift + Rotate: Sends 10x CC messages for fast adjustment

2. **Encoder 1 - Stop Loss**
   - Rotate CW: MIDI CC 23 (increase stop)
   - Rotate CCW: MIDI CC 22 (decrease stop)
   - Press: Alt+F16 (Base) / Alt+F17 (Buy) / Alt+F18 (Sell) - toggle $/% mode
   - Hold Shift + Rotate: 10x adjustment speed

3. **Encoder 2 - Limit Price**
   - Rotate CW: MIDI CC 25 (increase limit)
   - Rotate CCW: MIDI CC 24 (decrease limit)
   - Press: Alt+F19 (Base) / Alt+F20 (Buy) / Alt+F21 (Sell) - toggle market/limit order
   - Hold Shift + Rotate: 10x adjustment speed

### Key Mappings

Each layer sends unique F-key combinations for the 8 main keys:

- **Base Layer**: F13 through F20 (clean, no modifiers)
- **Buy Layer**: F21 through F28
- **Sell Layer**: F29 through F35, then Shift+F35 for the last key

### MIDI Channel Assignment
The MIDI channel changes automatically with the layer:
- Channel 1: Base layer operations
- Channel 2: Buy-specific operations
- Channel 3: Sell-specific operations

This allows your trading software to distinguish between buy/sell contexts for the same encoder movement.

## Building & Flashing

```bash
# Build firmware
make work_louder/loop:ibkr

# Flash firmware (put keyboard in bootloader mode first)
make work_louder/loop:ibkr:flash
```

## Trading Software Setup

### MIDI Setup
1. Connect the keyboard via USB
2. Your OS should recognize it as a MIDI device
3. In your trading software, set up MIDI learn/mapping:
   - Map CC 20/21 to share quantity decrease/increase
   - Map CC 22/23 to stop loss adjustments
   - Map CC 24/25 to limit price adjustments
4. Map the F-keys for button actions:
   - F13-F20 for Base layer keys
   - F21-F28 for Buy layer keys
   - F29-F35 + Shift+F35 for Sell layer keys
   - Alt+F13-F21 for encoder presses

### Using with Different Trading Platforms

#### AutoHotkey/Keyboard Maestro (Windows/Mac)
- Create MIDI triggers that convert CC messages to keystrokes or mouse actions
- Use different actions based on MIDI channel for context-aware controls

#### Interactive Brokers TWS
- Use third-party MIDI-to-keystroke software
- Map MIDI CCs to TWS hotkey combinations

#### TradingView
- Use browser MIDI API extensions
- Map CCs to TradingView shortcuts

### Key Combination Mapping
1. Configure hotkeys for each function you want
2. Assign the corresponding key combination from this keymap
3. Test each function before live trading

## Safety Notes

⚠️ **Always test thoroughly in a paper trading environment before using in live trading**
⚠️ **Double-check all MIDI mappings to avoid accidental trades**
⚠️ **Consider adding confirmation dialogs for critical actions**
⚠️ **Start with small position sizes when first using the system**
⚠️ **Have a backup method to close positions in case of hardware failure**