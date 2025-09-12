# PadLoop2 Trading Application Implementation

## Project Overview
You are working on a Rust application that interfaces with a custom hardware pad (PadLoop2) with 9 buttons and 4 encoders across 3 layers. The hardware is already working and sends HID events. Your task is to create a trading application that binds these hardware inputs to trading actions and displays the state in a GUI.

## Current Project Structure
- The project is located at `/workspace/workspace/qmk_firmware/loop_board_bindings/padloop2/`
- There's already a working HID interface in `src/lib.rs` and `src/event.rs`
- A basic GUI exists in `src/bin/gui.rs` using egui/eframe
- The hardware sends events as `PadEvent::Button { layer, idx, pressed }` and `PadEvent::EncRotate { idx, delta }`

## Your Task
Create a minimal trading application with the following requirements:

### 1. State Management
Create a new file `src/trading_state.rs` with the complete state structure provided below. This represents the entire application state that tracks all button bindings, encoder values, and current modes.

### 2. TOML Configuration
Create a `config/bindings.toml` file that maps hardware buttons to trading actions for each layer (Base, Buy, Sell). The TOML should be simple and human-readable.

### 3. GUI Updates
Modify `src/bin/gui.rs` to:
- Display all state fields in text boxes
- Show the current value with a white border/contour
- Show non-current values without a border
- Update the display when state changes occur
- Keep it SUPER SIMPLE - just text boxes showing values, no complex graphics

### 4. Event Handling
Connect the hardware events to state updates:
- Button presses should trigger the bound actions
- Encoder rotations should update the encoder values
- Encoder presses should cycle through encoder modes

## State Structure to Implement

```rust
use serde::{Deserialize, Serialize};

// Counts for UI layout
pub const KEY_COUNT: usize = 9;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum AccountType { Paper, Live }

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Layer { Base, Buy, Sell }

// Encoder value enums (variant encodes mode + carries value)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Enc1Value { 
    Shares(i32), 
    PercentBuyingPower(i32), 
    RiskDollars(i32) 
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Enc2Value { 
    Ticks(i32), 
    Percent(i32), 
    Atr(i32) 
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Enc3Value { 
    BidOffset(i32), 
    MidOffset(i32), 
    AskOffset(i32) 
}

// Per-layer binding enums
pub type Bound<T> = Option<T>;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Serialize, Deserialize)]
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

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct KeyLabels {
    pub base: [String; KEY_COUNT],
    pub buy: [String; KEY_COUNT],
    pub sell: [String; KEY_COUNT],
}

#[derive(Debug, Clone, Serialize, Deserialize)]
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
    pub last_key_event: Option<(u8, bool)>,

    // Encoder state (variant indicates current mode & holds value)
    pub enc1: Enc1Value,
    pub enc2: Enc2Value,
    pub enc3: Enc3Value,

    pub last_encoder_event: Option<(u8, i16)>,
}
```

## Hardware Event Mapping

### Button Events
- Layer 0 (Base): `PadEvent::Button { layer: 0, idx: 0-8, pressed: true/false }`
- Layer 1 (Buy): `PadEvent::Button { layer: 1, idx: 0-8, pressed: true/false }`
- Layer 2 (Sell): `PadEvent::Button { layer: 2, idx: 0-8, pressed: true/false }`

### Encoder Events
- Rotation: `PadEvent::EncRotate { idx: 0-3, delta: i8 }`
- Press: `PadEvent::EncPress { idx: 0-3, long: bool }`

Encoder 0-2 map to enc1, enc2, enc3 respectively. Encoder presses should cycle through the modes of each encoder.

## Implementation Requirements

1. **Keep it SUPER SIMPLE** - No complex error handling or elaborate UI
2. **Minimal dependencies** - Use what's already in the project
3. **Focus on state display** - Show all state fields as text with white borders for current values
4. **TOML for configuration** - Simple, readable bindings file
5. **No simulation needed** - Just connect real hardware events to state changes

## Files to Create/Modify

1. `src/trading_state.rs` - Complete state structure
2. `config/bindings.toml` - Button to action mappings
3. `src/bin/gui.rs` - Update to display all state fields
4. `src/lib.rs` - Add `pub mod trading_state;`
5. `Cargo.toml` - Ensure `serde` and `toml` dependencies are present

Start by creating the trading_state.rs file, then the TOML configuration, and finally update the GUI to display everything.