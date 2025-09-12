# Custom Trading Keyboard Implementation Plan for Loop Pad from Work Louder

## Overview
Integration of a custom 9-key keyboard with 3 encoders for rapid trading operations in the IBKR Proto application using **RAW HID Protocol v1**. Each command applies to a single armed ticker with tick-aware, risk-aware controls.

Per-key LED: lit only when action is eligible (controlled via HID).

ARMing the keyboard functions is done by the host application.

## Hardware Layout & HID Protocol
- **9 Physical Keys** with 3 layers (27 logical functions):
  - Layer 0 (BASE): Neutral/Safety actions - White LEDs
  - Layer 1 (BUY): Buy operations - Green LEDs
  - Layer 2 (SELL): Sell operations - Red LEDs
  
- **3 Encoders** sending HID events:
  - Encoder 0: Position sizing (multi-mode)
  - Encoder 1: Stop offset from entry  
  - Encoder 2: Limit/entry price anchor

## RAW HID Communication Protocol

### Device → Host Events
- **Button Event (0x10)**: `[0x10, layer, btn_idx(0-8), press/release]`
- **Encoder Rotation (0x11)**: `[0x11, enc_idx(0-2), delta(+1/-1)]`
- **Encoder Press (0x12)**: `[0x12, enc_idx(0-2), type(1=short/2=long)]`
- **Layer Change (0x13)**: `[0x13, layer(0-2)]`
- **Boot Hello (0x7E)**: `[0x7E, proto_ver=1, fw_major, fw_minor]`

### Host → Device Commands
- **Set All LEDs (0x01)**: `[0x01, r, g, b]` (BASE layer only)
- **Set One LED (0x02)**: `[0x02, led_idx(0-8), r, g, b]` (BASE layer only)
- **Set Layer (0x03)**: `[0x03, layer(0-2)]`
- **Host Ready (0x7D)**: `[0x7D, proto_ver=1]`

### Encoder Press Functions
- **Encoder 0**: Short press = cycle [Shares → %BP → Risk$]. Long press = reset to default
- **Encoder 1**: Short press = cycle [Ticks → % → ATR]. Long press = toggle Trailing/Fixed
- **Encoder 2**: Short press = cycle [Bid → Mid → Ask]. Long press = toggle Non-Crossing (post-only)



## Key Assignments (HID Event Based)

### Physical Layout
```
[Enc0]  [Enc1]  [Enc2]  [Btn0]  [Btn1]  [Btn2] [Btn3]  [Btn4]  [Btn5]  [Btn6]  [Btn7]  [Btn8]
```

### Neutral/Safety Keys
Button events send: `[0x10, 0, btn_idx, press/release]`
1. **Btn0**: Paper/Live Toggle (confirm modal; LED theme swap)
2. **Btn1**: 
3. **Btn2**: Account Select (press-and-hold → choose with visual confirmation)
4. **Btn3**: Refresh Data & Status (positions, margin, ticks)
5. **Btn4**: 
6. **Btn5**: Symbol Lock Toggle
7. **Btn6**: 
8. **Btn7**: 
9. **Btn8**: Snapshot Backup (positions + orders + settings, timestamped, append-only)

### Buy Keys
Button events send: `[0x10, 1, btn_idx, press/release]`
1. **Btn0**: Market Buy (ARM required)
2. **Btn1**: Limit Buy (uses Encoder-3 anchor + offset)
3. **Btn2**: Bracket Buy (entry + stop from Enc-2 + optional TP; OCO)
4. **Btn3**: Stop-Entry Buy (momentum entries)
5. **Btn4**: Scale-In +25% toward target size
6. **Btn5**: Scale-In +50% toward target size
7. **Btn6**: Flatten Current Symbol (hold 1.5s or double-click; cancels child orders immidiately first)
8. **Btn7**: 
9. **Btn8**: Reverse to Long (calculates delta; ARM required) 

### Sell Keys
Button events send: `[0x10, 2, btn_idx, press/release]`
1. **Btn0**: Market Sell (ARM required)
2. **Btn1**: Limit Sell (uses Encoder-3 anchor + offset)
3. **Btn2**: Bracket Sell (entry + stop + optional TP)
4. **Btn3**: Stop-Entry Sell (for breakdowns) or Short-at-Market
5. **Btn4**: Scale-Out 25% (current symbol only)
6. **Btn5**: Scale-Out 50% (current symbol only)
7. **Btn6**: Flatten Current Symbol (hold 1.5s or double-click; cancels child orders immidiately first)
8. **Btn7**: 
9. **Btn8**: Reverse to Short (calculates delta; ARM required; needs confirmation if hedged) 

## Tick-Aware Encoder Functionality

### Encoder 1: Position Sizing (Multi-Mode)
**Click to cycle modes**: [Shares] → [% of Buying Power] → [Risk $ / ATR]

**Mode Steps**:
- **Shares**: ±1 / ±10 / ±100 shares
- **Percent**: ±0.1% / ±0.5% / ±1% of buying power
- **Risk**: ±$25 / ±$100 / ±$250 risk per trade

**Display**: Small HUD always shows active sizing mode

### Encoder 2: Stop Offset from Entry
**Click to cycle modes**: [Ticks] → [%] → [ATR multiples]

**Mode Steps**:
- **Ticks**: ±1 / ±5 / ±10 ticks (uses contract minTick)
- **Percent**: ±0.1% / ±0.25% / ±0.5%
- **ATR**: ±0.1× / ±0.25× / ±0.5× ATR

**Click-and-hold**: Toggle between Trailing vs Fixed stop

### Encoder 3: Limit/Entry Price Anchor
**Click to cycle anchor**: [Bid] → [Mid] → [Ask]

**Rotation**: Offsets from anchor by ticks (±1 / ±5 / ±10)

**Optional**: Post-Only toggle for NASDAQ (prevents crossing; beeps if would cross; not post for other venues, show warning if other venue is used with post-only)
