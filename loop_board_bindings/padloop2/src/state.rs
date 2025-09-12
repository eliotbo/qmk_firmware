//! Minimal, UI-facing trading state for verifying keyboard/encoder wiring.
//!
//! This module intentionally contains no trading/business logic. It only
//! defines plain data structures that UI or input code can update in response
//! to key/encoder events.

use serde::{Deserialize, Serialize};

/// Number of pad keys and encoders on the device UI targets.
pub const KEY_COUNT: usize = 9;
pub const ENCODER_COUNT: usize = 3;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum AccountType {
    Paper,
    Live,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Layer {
    Base,
    Buy,
    Sell,
}

// Encoder modes per blueprint plan; purely descriptive for the UI.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Encoder1Mode {
    Shares,
    PercentBuyingPower,
    RiskDollars,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Encoder2Mode {
    Ticks,
    Percent,
    Atr,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Encoder3Mode {
    Bid,
    Mid,
    Ask,
}

/// Container for per-layer key labels. Empty strings by default.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct KeyLabels {
    pub base: [String; KEY_COUNT],
    pub buy: [String; KEY_COUNT],
    pub sell: [String; KEY_COUNT],
}

impl Default for KeyLabels {
    fn default() -> Self {
        // Start with empty strings; UI/app can populate later.
        Self {
            base: std::array::from_fn(|_| String::new()),
            buy: std::array::from_fn(|_| String::new()),
            sell: std::array::from_fn(|_| String::new()),
        }
    }
}

/// Minimal app state to reflect inputs and selected modes.
/// No side-effects or computations are performed here.
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
    pub key_armed: [bool; KEY_COUNT],
    pub key_pressed: [bool; KEY_COUNT],
    pub last_key_event: Option<(u8 /*index 0..8*/, bool /*pressed*/ )>,

    // Encoder state
    pub enc1_mode: Encoder1Mode,
    pub enc2_mode: Encoder2Mode,
    pub enc3_mode: Encoder3Mode,
    pub encoder_values: [i32; ENCODER_COUNT],     // simple counters/deltas
    pub last_encoder_event: Option<(u8 /*index 0..2*/, i16 /*delta*/ )>,
}

impl Default for TradingState {
    fn default() -> Self {
        Self {
            account_type: AccountType::Paper,
            armed: false,
            account: 0,
            layer: Layer::Base,
            key_labels: KeyLabels::default(),
            key_armed: [false; KEY_COUNT],
            key_pressed: [false; KEY_COUNT],
            last_key_event: None,
            enc1_mode: Encoder1Mode::Shares,
            enc2_mode: Encoder2Mode::Ticks,
            enc3_mode: Encoder3Mode::Mid,
            encoder_values: [0; ENCODER_COUNT],
            last_encoder_event: None,
        }
    }
}

