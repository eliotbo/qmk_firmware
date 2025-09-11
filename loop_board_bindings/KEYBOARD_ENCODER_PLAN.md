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
[Enc0]  [Enc1]  [Enc2]
[Btn0]  [Btn1]  [Btn2]
[Btn3]  [Btn4]  [Btn5]
[Btn6]  [Btn7]  [Btn8]
```

### Layer 0 (BASE) - Neutral/Safety Actions
Button events send: `[0x10, 0, btn_idx, press/release]`
1. **Btn0**: Paper/Live Toggle (double-confirm; LED theme swap)
2. **Btn1**: ARM Trading State (hold 3s)
3. **Btn2**: Account Select (press-and-hold → choose with visual confirmation)
4. **Btn3**: Refresh Data & Status (positions, margin, ticks)
5. **Btn4**: Kill Switch (progressive: 1.5s cancel, 3s full shutdown)
6. **Btn5**: Symbol Lock Toggle
7. **Btn6**: Template System Access
8. **Btn7**: Global Flatten (hold 3s with countdown)
9. **Btn8**: Snapshot Backup (positions + orders + settings, timestamped, append-only)

### Layer 1 (BUY) - Buy Operations
Button events send: `[0x10, 1, btn_idx, press/release]`
1. **Btn0**: Market Buy (ARM required)
2. **Btn1**: Limit Buy (uses Encoder-2 anchor + offset)
3. **Btn2**: Bracket Buy (entry + stop from Enc-1 + optional TP; OCO)
4. **Btn3**: Stop-Entry Buy (momentum entries)
5. **Btn4**: Scale-In +25% toward target size
6. **Btn5**: Scale-In +50% toward target size
7. **Btn6**: Template A (e.g., micro-scalp: Lmt @ mid-1t, SL 4t, TP 6t)
8. **Btn7**: Cancel all buy orders for ticker (Double-tap required)
9. **Btn8**: Reverse to Long (calculates delta; ARM required)

### Layer 2 (SELL) - Sell Operations  
Button events send: `[0x10, 2, btn_idx, press/release]`
1. **Btn0**: Market Sell (ARM required)
2. **Btn1**: Limit Sell (uses Encoder-2 anchor + offset)
3. **Btn2**: Bracket Sell (entry + stop + optional TP)
4. **Btn3**: Stop-Entry Sell (for breakdowns) or Short-at-Market
5. **Btn4**: Scale-Out 25% (current symbol only)
6. **Btn5**: Scale-Out 50% (current symbol only)
7. **Btn6**: Template B (e.g., swing: Lmt @ ask+2t, SL 1.5ATR, TP 3ATR)
8. **Btn7**: Flatten Current Symbol (hold 1.5s; cancels child orders first)
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

## Additional Context Toggles

**TIF (Time in Force)**: DAY ↔ IOC ↔ GTC (small key or long-press)

**Route**: SMART ↔ Direct (e.g., NASDAQ) when needed

**Display Mode**: Last ↔ Mid ↔ Bid/Ask in the UI

## Pre-Trade Validation Checks

Before sending any order, validate:
- Market open/closed status
- Halts/LULD state
- Shortability and SSR status
- Tick alignment with contract specs
- Buying power/margin requirements

If any check fails: Don't send, beep, and show reason in UI

## Implementation Architecture

### 1. Core Safety Systems

#### Armed Trading State
```rust
pub struct ArmingState {
    armed: bool,
    armed_at: Option<Instant>,
    arm_timeout: Duration, // Auto-disarm after inactivity
    symbol_lock: Option<String>,
    account_lock: Option<String>,
}
```

#### Tick-Aware Contract Cache
```rust
pub struct ContractDetails {
    symbol: String,
    min_tick: f64,
    price_scale: i32,
    atr: Option<f64>,
    bid: f64,
    ask: f64,
    last: f64,
    shortable: bool,
    ssr_triggered: bool,
}
```

### 2. HID Input Module (`src/input/mod.rs`)
```rust
pub struct InputManager {
    hid_handler: HidHandler,
    arming_state: ArmingState,
    contract_cache: HashMap<String, ContractDetails>,
    app_handle: Arc<Mutex<IbkrApp>>,
    encoder_states: EncoderStates,
}
```

### 3. HID Handler (`src/input/hid.rs`)
```rust
pub struct HidHandler {
    device: HidDevice, // hidapi device handle
    buffer: [u8; 32],  // RAW HID buffer
    current_layer: u8,
    led_states: [LedColor; 9],
}

impl HidHandler {
    // Process incoming HID events
    pub fn process_event(&mut self, data: &[u8]) {
        match data[0] {
            0x10 => self.handle_button(data[1], data[2], data[3] == 1),
            0x11 => self.handle_encoder_rotate(data[1], data[2] as i8),
            0x12 => self.handle_encoder_press(data[1], data[2]),
            0x13 => self.handle_layer_change(data[1]),
            0x7E => self.handle_hello(data),
            _ => {}
        }
    }
    
    // Send commands to device
    pub fn set_led(&mut self, idx: u8, color: RGB) { /* send 0x02 */ }
    pub fn set_all_leds(&mut self, color: RGB) { /* send 0x01 */ }
    pub fn set_layer(&mut self, layer: u8) { /* send 0x03 */ }
    pub fn send_ready(&mut self) { /* send 0x7D */ }
}
```

### 4. Encoder State Management
- Multi-mode state tracking (sizing/stop/price)
- Tick-aware value stepping based on mode
- Mode cycling on short press
- Reset on long press
- No OS keyboard events

### 5. Trading Actions (`src/trading/actions.rs`)
- Pre-trade validation checks
- Tick-aligned price calculations
- Risk-based position sizing
- Idempotent order submission

### 6. UI Integration
- ARM state indicator (large, obvious)
- Symbol lock display with LED simulation
- Live/Paper mode banner
- Encoder mode HUD
- Pre-trade check failure notifications

## Critical Safety Features

1. **ARM System**:
   - All trades require ARM state (F13 hold 3s)
   - Auto-disarm after 30s inactivity
   - Visual ARM indicator (large, red when armed)
   - Cannot arm in live mode without confirmation

2. **Hold-to-Execute**:
   - Flatten Symbol: 1.5s hold
   - Global Flatten: 3s hold with countdown
   - Kill-Switch: Progressive (1.5s cancel, 3s full shutdown)
   - Account/Mode switches: Hold + visual confirm

3. **Pre-Trade Validation**:
   - Market hours check
   - Halt/LULD status
   - SSR and shortability
   - Tick alignment
   - Buying power verification

4. **Visual/Audio Feedback**:
   - Beep on validation failures
   - LED simulation for armed/locked states
   - Paper mode: Blue theme / Live mode: Red theme
   - Mode displays for all encoders

5. **Data Integrity**:
   - Append-only backup snapshots
   - Unique order IDs per keypress
   - Full event logging for replay/audit
   - Crash recovery without duplicates

## Configuration

### User Settings (`config.toml`)
```toml
[hid]
vendor_id = 0x574C  # Work Louder
product_id = 0x1DF9  # Loop
usage_page = 0xFF60
usage = 0x61
arm_timeout_seconds = 30
hold_times_ms = { flatten_symbol = 1500, global_flatten = 3000, kill_switch = 3000 }

[encoders]
default_sizing_mode = "shares"  # shares, percent, risk
default_stop_mode = "ticks"     # ticks, percent, atr
default_price_anchor = "mid"    # bid, mid, ask

[safety]
require_arm_for_trades = true
paper_mode_default = true
max_risk_per_trade = 500.00
min_tick_cache_ttl_seconds = 60

[templates]
scalp = { stop_ticks = 4, target_ticks = 6, size_mode = "risk" }
swing = { stop_atr = 1.5, target_atr = 3.0, size_mode = "percent" }
```

## Development Phases

### Phase 1: HID Integration
- [ ] HID device connection using hidapi
- [ ] RAW HID protocol v1 implementation
- [ ] Event processing for buttons/encoders
- [ ] LED control commands
- [ ] Host ready handshake

### Phase 2: Safety Infrastructure
- [ ] ARM state management system
- [ ] Hold-time tracking from HID events
- [ ] Contract details cache with minTick
- [ ] Event logging system

### Phase 3: Multi-Mode Encoders (via HID)
- [ ] HID encoder event processing
- [ ] Position sizing modes (shares/percent/risk)
- [ ] Stop offset modes (ticks/percent/ATR)
- [ ] Price anchor modes (bid/mid/ask)
- [ ] Mode cycling on short press
- [ ] Reset on long press

### Phase 4: UI Integration
- [ ] ARM state indicator (large, central)
- [ ] Symbol/Account lock displays
- [ ] Paper/Live mode theming
- [ ] Encoder mode HUDs
- [ ] Validation failure notifications

## Implementation Gotchas

1. **HID Safety**: No OS keyboard events - pure vendor HID protocol prevents focus leaks
2. **Idempotency**: Each HID button event = unique client order ID; handle crashes/restarts gracefully
3. **MinTick Caching**: Cache contract specs per symbol; refresh on symbol change
4. **Logging**: Persist every: HID event → order spec → IB status for debugging/compliance
5. **Backups**: Btn8 (BASE layer) writes timestamped snapshots; never delete, only archive
6. **Buffer Size**: RAW HID uses 32-byte buffers; pack messages efficiently

## Testing Strategy

1. **Safety Tests**:
   - ARM timeout behavior
   - Hold-time requirements
   - Pre-trade validation failures

2. **Tick Tests**:
   - Price alignment with minTick
   - Rounding in percent mode
   - Cross prevention for post-only

3. **Recovery Tests**:
   - Crash during order submission
   - Network disconnection handling

## Performance Targets

- Key press → Order submission: <20ms (direct device access)
- Encoder → UI update: <10ms
- MinTick cache lookup: <1ms
- Pre-trade validation: <5ms

## Critical Success Factors

1. **No Single-Tap Disasters**: All destructive actions require holds or ARM
2. **Tick-Perfect Pricing**: All prices aligned to contract minTick
3. **Risk-Aware Sizing**: Position sizing based on stop distance and risk tolerance
4. **Clear State Display**: Always know if armed, which symbol, which account, paper/live
5. **Audit Trail**: Complete event log for every action taken

## Dependencies

- `egui`: UI framework (existing)
- `hidapi`: RAW HID communication with keyboard
- `tokio`: Async runtime (existing)
- `serde`: Configuration (existing)
- `chrono`: Timestamping for backups

## Notes

- Start in paper mode by default with clear visual distinction
- ARM state should be impossible to miss (large red indicator)
- Consider audio feedback for ARM state changes
- Template system allows custom order configurations
- All hold times should be configurable but with safe defaults
- Debounced: ignore repeated HID events within 8ms
- Log every HID event
- Auto disarm after 10 minutes of inactivity

## RAW HID Protocol v1 Reference

### Connection Details
- **Vendor ID**: 0x574C (Work Louder)
- **Product ID**: 0x1DF9 (Loop)
- **Usage Page**: 0xFF60
- **Usage**: 0x61
- **Buffer Size**: 32 bytes (fixed)

### Message Format
All messages are 32 bytes with zero-padding:
- Byte 0: Message type/command ID
- Bytes 1-31: Payload (message-specific)

### Complete Protocol Specification

#### Device → Host Events

| Event | ID | Format | Description |
|-------|-----|--------|-------------|
| Button | 0x10 | `[0x10, layer, idx, act]` | act: 1=press, 0=release |
| Encoder Rotate | 0x11 | `[0x11, enc_idx, delta]` | delta: int8 (+1/-1 per detent) |
| Encoder Press | 0x12 | `[0x12, enc_idx, type]` | type: 1=short, 2=long (≥500ms) |
| Layer Change | 0x13 | `[0x13, layer]` | layer: 0=BASE, 1=BUY, 2=SELL |
| Boot Hello | 0x7E | `[0x7E, proto, major, minor]` | Firmware version announcement |

#### Host → Device Commands

| Command | ID | Format | Description |
|---------|-----|--------|-------------|
| Set All LEDs | 0x01 | `[0x01, r, g, b]` | BASE layer only |
| Set One LED | 0x02 | `[0x02, idx, r, g, b]` | idx: 0-8, BASE layer only |
| Set Layer | 0x03 | `[0x03, layer]` | Switch active layer |
| Host Ready | 0x7D | `[0x7D, proto_ver]` | Handshake acknowledgment |

### Implementation Example (Rust)
```rust
// Reading HID events
let mut buf = [0u8; 32];
match device.read(&mut buf) {
    Ok(_) => {
        match buf[0] {
            0x10 => { // Button event
                let layer = buf[1];
                let button_idx = buf[2];
                let pressed = buf[3] == 1;
                handle_button(layer, button_idx, pressed);
            }
            0x11 => { // Encoder rotation
                let encoder_idx = buf[1];
                let delta = buf[2] as i8;
                handle_encoder(encoder_idx, delta);
            }
            // ... handle other events
        }
    }
    Err(e) => eprintln!("HID read error: {}", e),
}

// Sending LED command
let mut cmd = [0u8; 32];
cmd[0] = 0x01; // Set all LEDs
cmd[1] = 255;  // Red
cmd[2] = 0;    // Green  
cmd[3] = 0;    // Blue
device.write(&cmd)?;
```

### LED Behavior by Layer
- **BASE (0)**: Host-controllable via HID commands
- **BUY (1)**: Force green (ignores host commands)
- **SELL (2)**: Force red (ignores host commands)

### Timing Specifications
- **Short press**: < 500ms
- **Long press**: ≥ 500ms
- **Device debounce**: 5ms
- **Recommended host debounce**: 10ms

### Error Handling
- Unknown message types should be ignored
- Malformed messages should be logged but not fatal
- Device disconnection requires re-handshake on reconnect
- Host should retry handshake if no hello received

### Protocol Advantages
- **No OS interference**: Vendor-specific HID bypasses keyboard handling
- **Bidirectional**: Full duplex communication
- **Layer context**: Every button event includes layer state
- **Extensible**: Protocol version allows future enhancements
- **Reliable**: No keystroke leaks to other applications