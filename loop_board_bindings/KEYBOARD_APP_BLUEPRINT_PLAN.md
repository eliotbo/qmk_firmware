# Loop Pad Blueprint Crate (egui) — Plan

## Overview
Blueprint for a new Rust crate that provides an egui desktop app to interface with the custom Loop Pad keyboard (9 keys + 3 encoders) using RAW HID Protocol v1. The app mirrors and drives device state: it shows encoder values in text boxes, renders one label per key with a status circle, and keeps the keyboard LEDs in sync with app state where the firmware allows. Design is consistent with KEYBOARD_ENCODER_PLAN.md and aligned with qmk_keymap.c and the loop-pad-test crate.

Key UI requirements
- Three text boxes: one per encoder’s current value/mode.
- Nine key labels: change with active layer (BASE/BUY/SELL).
- Status circle under each label: green if armed, red if disarmed; turns blue while the corresponding key/encoder is pressed.
- ARM button in the app.
- Layer buttons for BASE, BUY, SELL.

LED sync policy
- BASE layer: host controls LEDs via 0x01/0x02. Color is white for bound keys when Armed; OFF when not Armed or when key is unbound. Press highlight temporarily turns that key blue. Pending, unfilled orders flash the key 1s ON / 1s OFF (app-driven).
- BUY layer: firmware forces LEDs to green. Target policy: green for bound keys when Armed; OFF when not Armed or unbound. Due to firmware forcing, host cannot enforce OFF or flash; reflect OFF/flash only in the UI, not hardware.
- SELL layer: firmware forces LEDs to red. Target policy: red for bound keys when Armed; OFF when not Armed or unbound. Due to firmware forcing, host cannot enforce OFF or flash; reflect OFF/flash only in the UI, not hardware.


## Protocol Summary (consistent with KEYBOARD_ENCODER_PLAN.md)

Device → Host
- Button 0x10: `[0x10, layer(0-2), btn_idx(0-8), act(1=press/0=release)]`
- Encoder rotate 0x11: `[0x11, enc_idx(0-2), delta(int8 +1/-1)]`
- Encoder press 0x12: `[0x12, enc_idx(0-2), kind(1=press)]` (if firmware emits `2=long`, the app ignores it, no long-press compatibility)
- Layer change 0x13: `[0x13, layer(0-2)]`
- Boot hello 0x7E: `[0x7E, proto_ver=1, fw_major, fw_minor]`

Host → Device
- Set all LEDs 0x01: `[0x01, r, g, b]` (BASE layer only)
- Set one LED 0x02: `[0x02, led_idx(0-8), r, g, b]` (BASE layer only)
- Set layer 0x03: `[0x03, layer(0-2)]`
- Host ready 0x7D: `[0x7D, proto_ver=1]`

Firmware LED behavior (qmk_keymap.c)
- BASE: host LED control permitted; HID override respected until leaving BASE.
- BUY: forced green; host LED control ignored.
- SELL: forced red; host LED control ignored.


## Key/Layer Labels (from KEYBOARD_ENCODER_PLAN.md)

Physical order: `[Enc0] [Enc1] [Enc2]  [Btn0..Btn8]`

### Neutral/Safety Keys
Button events send: `[0x10, 0, btn_idx, press/release]`
1. **Btn0**: Paper/Live Toggle (confirm modal; LED theme swap)
2. **Btn1**: 
3. **Btn2**: Account Select (press opens selection modal with visual confirmation)
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
7. **Btn6**: Flatten Current Symbol (requires confirmation; cancels child orders immediately first)
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
7. **Btn6**: Flatten Current Symbol (requires confirmation; cancels child orders immediately first)
8. **Btn7**: 
9. **Btn8**: Reverse to Short (calculates delta; ARM required; needs confirmation if hedged) 



## Crate Structure

```
loop-pad-blueprint/
├── Cargo.toml
└── src/
    ├── main.rs              # eframe/egui entrypoint
    ├── app.rs               # AppState, update loop, UI
    ├── ui/
    │   ├── mod.rs
    │   ├── keyboard.rs      # 1x9 keys: label + status circles
    │   ├── encoders.rs      # 3 encoder text boxes + mode labels
    │   └── controls.rs      # ARM button + BASE/BUY/SELL buttons
    ├── hid/
    │   ├── mod.rs
    │   ├── device.rs        # HID open, read thread, write API
    │   ├── protocol.rs      # IDs/structs aligned with loop-pad-test
    │   └── events.rs        # Byte → PadEvent parsing
    └── state/
        ├── mod.rs
        ├── app_state.rs     # Armed/layer/press-highlights/LED model
        ├── encoders.rs      # values + modes + cycling
        └── leds.rs          # LED sync policy (BASE-only writes)
```

Dependencies
- `eframe`, `egui` for UI; `crossbeam-channel` for events.
- `hidapi` (gated by `hid` feature); same write/read quirks as loop-pad-test.
- `log`, `env_logger`. Optional `tokio` if reusing async helpers.
- Option A: depend on local `loop-pad-test` as a library for HID/event types.
- Option B: re-implement minimal protocol/glue (mirroring loop-pad-test) inside this crate.


## State Model

AppState (sketch)
```rust
pub struct AppState {
    // Connection
    connected: bool,
    fw: Option<(u8,u8,u8)>,   // (proto, fw_major, fw_minor)

    // Core UI state
    layer: u8,                // 0=BASE,1=BUY,2=SELL

    // Press highlights
    key_pressed: [bool; 9],
    enc_pressed: [bool; 3],
    last_press_ms: [u128; 12], // fade out blue highlight



    // Labels per layer
    labels: [[&'static str; 9]; 3],

    // LED state cache (BASE only)
    base_leds: [Rgb; 9],
}
```

Encoders
- Enc0: cycle [Shares → %BP → Risk$] on press; rotation adjusts current value.
- Enc1: cycle [Ticks → % → ATR] on press; rotation adjusts current value.
- Enc2: cycle [Bid → Mid → Ask] on press; rotation adjusts current value.


## UI Plan (egui)

Layout
- Top bar: buttons `BASE | BUY | SELL` (active one highlighted). Clicking sends 0x03; `ARM` button toggles armed state.
- Encoder panel: three rows with a label for mode and a text box for the current value/offset. When an encoder is pressed/rotated, show a blue circle next to its label for a short duration (e.g., 150–250ms).
- Keyboard grid: 3×3 labels reflecting the active layer; under each label draw a filled circle:
  - Blue while the key is currently pressed (press event debounced/faded).
  - Otherwise green if `armed == true`, red if `armed == false`.

Event handling
- EV_HELLO: set `connected=true`, record version, send Host Ready (0x7D).
- EV_LAYER: update `layer`; update active button highlight.
- EV_BUTTON: set `key_pressed[idx]` for blue highlight; on release, clear highlight.
- EV_ENC_ROTATE: update the appropriate encoder value; briefly flash encoder’s circle blue.
- EV_ENC_PRESS: cycle encoder mode and flash blue (no long-press handling).


## LED Sync Policy

Goals
- Keep hardware LEDs aligned with app state where firmware permits; keep UI circles aligned with user intent (armed/pressed) regardless of layer.

Rules
- BASE layer (layer==0): App computes per-key RGB and issues 0x02 (Set One) updates:
  - Precedence: Press blue overrides flashing; flashing overrides steady colors.
  - Key pressed: temporarily blue (e.g., RGB(0,0,255)).
  - Pending order AND Armed AND bound: flash 1s ON / 1s OFF between white and OFF.
  - Armed AND bound (no pending): white (RGB(255,255,255)).
  - Not Armed OR unbound: OFF (RGB(0,0,0)).
  - Optionally batch: on state changes, send a full pass for 9 LEDs; throttle to avoid spamming.
- BUY layer (layer==1): Firmware forces green. Target policy is green when Armed+bound; OFF when not Armed or unbound; pending orders should flash in UI only. Because host LED writes are ignored on this layer, reflect OFF/flash only in UI while hardware may still show steady green.
- SELL layer (layer==2): Firmware forces red. Target policy is red when Armed+bound; OFF when not Armed or unbound; pending orders should flash in UI only. Because host LED writes are ignored on this layer, reflect OFF/flash only in UI while hardware may still show steady red.

Press highlight timing
- Maintain a monotonic timestamp per key/encoder; render blue for N ms after press/rotation, then revert to the layer color (white/green/red) if Armed+bound, otherwise OFF. While blue is active, suspend any flashing.

Pending-order flash timing (app-driven)
- Maintain a 1 Hz toggler (e.g., based on `ctx.input(|i| i.time)` or a `std::time::Instant`).
- When `pending && armed && bound` in BASE, alternate the LED between white and OFF each second via 0x02. Send updates only on phase changes to minimize traffic.
- In BUY/SELL layers, reflect the flash visually in the UI status circle; skip HID writes (firmware ignores them).


## TradingState (Minimal, No Business Logic)

Purpose
- Provide a plain data model to verify keyboard keys/encoders and UI wiring.
- Contains no stock trading logic, calculations, order handling, or API calls.

Rust types (no business logic)
```rust
// Counts for UI layout
pub const KEY_COUNT: usize = 9;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AccountType { Paper, Live }

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Layer { Base, Buy, Sell }

// Encoder value enums (variant encodes mode + carries value)
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc1Value { Shares(i32), PercentBuyingPower(i32), RiskDollars(i32) }
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc2Value { Ticks(i32), Percent(i32), Atr(i32) }
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc3Value { BidOffset(i32), MidOffset(i32), AskOffset(i32) }

// Per-layer binding enums
// Use `Bound<T>` for presence/absence of a binding.
pub type Bound<T> = Option<T>;

// Per-layer binding enums (wrapped in Bound<...> to indicate unbound)
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BaseAction {
    PaperLiveToggle,
    Btn1,
    AccountSelect,
    RefreshData,
    Btn4,
    SymbolLockToggle,
    Btn6,
    Btn7,
    SnapshotBackup,
}

#[derive(Debug, Clone)]
pub struct BaseKeysState {
    pub btn0: Bound<BaseAction>,
    pub btn1: Bound<BaseAction>,
    pub btn2: Bound<BaseAction>,
    pub btn3: Bound<BaseAction>,
    pub btn4: Bound<BaseAction>,
    pub btn5: Bound<BaseAction>,
    pub btn6: Bound<BaseAction>,
    pub btn7: Bound<BaseAction>,
    pub btn8: Bound<BaseAction>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum BuyAction {
    MarketBuy,
    LimitBuy,
    BracketBuy,
    StopEntryBuy,
    ScaleIn25,
    ScaleIn50,
    FlattenCurrentSymbol,
    Btn7,
    ReverseToLong,
}

#[derive(Debug, Clone)]
pub struct BuyKeysState {
    pub btn0: Bound<BuyAction>,
    pub btn1: Bound<BuyAction>,
    pub btn2: Bound<BuyAction>,
    pub btn3: Bound<BuyAction>,
    pub btn4: Bound<BuyAction>,
    pub btn5: Bound<BuyAction>,
    pub btn6: Bound<BuyAction>,
    pub btn7: Bound<BuyAction>,
    pub btn8: Bound<BuyAction>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SellAction {
    MarketSell,
    LimitSell,
    BracketSell,
    StopEntrySell,
    ScaleOut25,
    ScaleOut50,
    FlattenCurrentSymbol,
    Btn7,
    ReverseToShort,
}

#[derive(Debug, Clone)]
pub struct SellKeysState {
    pub btn0: Bound<SellAction>,
    pub btn1: Bound<SellAction>,
    pub btn2: Bound<SellAction>,
    pub btn3: Bound<SellAction>,
    pub btn4: Bound<SellAction>,
    pub btn5: Bound<SellAction>,
    pub btn6: Bound<SellAction>,
    pub btn7: Bound<SellAction>,
    pub btn8: Bound<SellAction>,
}

// (old per-mode newtypes removed in favor of the value enums above)

#[derive(Debug, Clone)]
pub struct KeyLabels {
    pub base: [String; KEY_COUNT],
    pub buy: [String; KEY_COUNT],
    pub sell: [String; KEY_COUNT],
}

#[derive(Debug, Clone)]
pub struct TradingState {
    // Core account metadata
    pub account_type: AccountType,
    pub armed: bool,
    pub account: u16,

    // UI layer and labels
    pub layer: Layer,
    pub key_labels: KeyLabels,

    // Key state
    pub base_keys: BaseKeysState,
    pub buy_keys: BuyKeysState,
    pub sell_keys: SellKeysState,
    pub key_pressed: [bool; KEY_COUNT],
    pub last_key_event: Option<(u8 /*0..8*/, bool /*pressed*/ )>,

    // Encoder state (variant indicates current mode & holds value)
    pub enc1: Enc1Value,
    pub enc2: Enc2Value,
    pub enc3: Enc3Value,

    pub last_encoder_event: Option<(u8 /*0..2*/, i16 /*delta*/ )>,
}
```

Notes
- Defaults: `AccountType::Paper`, `Layer::Base`, empty labels, all keys disarmed/unpressed. Encoder defaults: `Enc1Value::Shares(0)`, `Enc2Value::Ticks(0)`, `Enc3Value::MidOffset(0)`.
- Key binding alias: `type Bound<T> = Option<T>`; `None` means unbound; `Some(…Action)` means bound. Use this to drive OFF for unbound keys.
- Encoder mode is the enum variant; cycling changes the variant, rotation updates the inner value.
- Only UI and input handlers update these fields to reflect events; no trading decisions.
- App may mirror this into LED updates on BASE layer (per policy above).


## Message Flow & Threads

- HID thread: owns `HidDevice`; continuously reads into a 32-byte buffer; parses events; sends `PadEvent` over `crossbeam_channel::Sender` to UI thread.
- UI thread (eframe run loop): polls channel non-blocking each frame; updates `AppState`; conditionally pushes LED commands (BASE layer only) via `HidDevice` send queue.
- On startup: open HID, start read thread, send 0x7D Host Ready, set initial layer per UI button selection, then render.


## Implementation Steps

1. Scaffold crate with `eframe` app skeleton and logging.
2. Add protocol constants/types; reuse loop-pad-test `PadEvent` shape.
3. Implement HID open/read thread and write API (0x7D, 0x01/0x02, 0x03).
4. Build `AppState` with encoder modes/values and per-layer labels.
5. Implement egui panels: controls (ARM + layer buttons), encoder text boxes, keyboard grid with status circles.
6. Wire event handling to update state and trigger UI blue highlights.
7. Implement LED sync for BASE layer; throttle/batch writes.
8. Handle reconnection and layer reconciliation (device EV_LAYER vs active button).
9. Add simple config (VID/PID/usage) and mock mode for development.
10. Smoke-test with device using loop-pad-test as reference.


## Testing & Validation

- Unit tests: event parsing; encoder mode cycling; BASE-layer LED color calculation from `(armed, pressed)`.
- Integration: connect real Loop Pad, verify EV_* events, HOST_READY handshake, layer change, and BASE LED updates.
- Manual checks: layer button control, ARM toggle, encoder edits via text boxes, UI blue highlights on press/rotate.


## Build & Run (proposed)

```
cargo run -p loop-pad-blueprint --features hid --release
```

Flags
- `--features hid`: enable real HID device; otherwise run in mock mode emitting synthetic events.


## Notes & Constraints

- Firmware ignores LED override outside BASE; do not attempt 0x01/0x02 on BUY/SELL.
- Windows writes may require a leading 0x00 report ID; mirror loop-pad-test handling.
- Debounce on host side (~10ms) recommended even though device debounces.
- Keep LED updates rate-limited to prevent USB stall (e.g., ≤100 Hz total updates, coalesce changes per frame).
