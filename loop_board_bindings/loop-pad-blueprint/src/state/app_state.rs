pub const KEY_COUNT: usize = 9;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AccountType {
    Paper,
    Live,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AppLayer {
    Base,
    Buy,
    Sell,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc1Value {
    Shares(i32),
    PercentBuyingPower(i32),
    RiskDollars(i32),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc2Value {
    Ticks(i32),
    Percent(i32),
    Atr(i32),
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Enc3Value {
    BidOffset(i32),
    MidOffset(i32),
    AskOffset(i32),
}

#[derive(Debug, Clone)]
pub struct KeyLabels {
    pub base: [String; KEY_COUNT],
    pub buy: [String; KEY_COUNT],
    pub sell: [String; KEY_COUNT],
}

#[derive(Debug, Clone)]
pub struct AppState {
    // Connection / firmware
    pub connected: bool,
    pub fw: Option<(u8, u8, u8)>, // (proto, fw_major, fw_minor)

    pub account_type: AccountType,
    pub armed: bool,
    pub account: u16,

    pub layer: AppLayer,
    pub key_labels: KeyLabels,

    pub key_pressed: [bool; KEY_COUNT],
    pub last_key_event: Option<(u8, bool)>,

    // Press/rotate highlights
    pub key_last_ts: [Option<std::time::Instant>; KEY_COUNT],
    pub enc_last_ts: [Option<std::time::Instant>; 3],

    pub enc1: Enc1Value,
    pub enc2: Enc2Value,
    pub enc3: Enc3Value,
    pub last_encoder_event: Option<(u8, i16)>,

    // LED sync (BASE only)
    pub base_leds: [crate::hid::protocol::Rgb; KEY_COUNT],
    pub last_led_update: Option<std::time::Instant>,
    pub needs_led_sync: bool,
    pub flash_phase: bool,
    pub last_flash_toggle: std::time::Instant,
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            connected: false,
            fw: None,
            account_type: AccountType::Paper,
            armed: false,
            account: 0,
            layer: AppLayer::Base,
            key_labels: KeyLabels {
                base: Default::default(),
                buy: Default::default(),
                sell: Default::default(),
            },
            key_pressed: [false; KEY_COUNT],
            last_key_event: None,
            key_last_ts: [None; KEY_COUNT],
            enc_last_ts: [None; 3],
            enc1: Enc1Value::Shares(0),
            enc2: Enc2Value::Ticks(0),
            enc3: Enc3Value::MidOffset(0),
            last_encoder_event: None,
            base_leds: [crate::hid::protocol::Rgb::OFF; KEY_COUNT],
            last_led_update: None,
            needs_led_sync: true,
            flash_phase: false,
            last_flash_toggle: std::time::Instant::now(),
        }
    }
}
