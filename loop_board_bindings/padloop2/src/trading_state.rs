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

impl Default for TradingState {
    fn default() -> Self {
        Self {
            account_type: AccountType::Paper,
            armed: false,
            account: 0,
            layer: Layer::Base,
            key_labels: KeyLabels {
                base: [
                    "Paper/Live".to_string(),
                    "Btn1".to_string(),
                    "Account".to_string(),
                    "Refresh".to_string(),
                    "Btn4".to_string(),
                    "Lock Symbol".to_string(),
                    "Btn6".to_string(),
                    "Btn7".to_string(),
                    "Snapshot".to_string(),
                ],
                buy: [
                    "Market Buy".to_string(),
                    "Limit Buy".to_string(),
                    "Bracket Buy".to_string(),
                    "Stop Entry".to_string(),
                    "Scale In 25%".to_string(),
                    "Scale In 50%".to_string(),
                    "Flatten".to_string(),
                    "Btn7".to_string(),
                    "Reverse Long".to_string(),
                ],
                sell: [
                    "Market Sell".to_string(),
                    "Limit Sell".to_string(),
                    "Bracket Sell".to_string(),
                    "Stop Entry".to_string(),
                    "Scale Out 25%".to_string(),
                    "Scale Out 50%".to_string(),
                    "Flatten".to_string(),
                    "Btn7".to_string(),
                    "Reverse Short".to_string(),
                ],
            },
            base_keys: BaseKeysState {
                btn0: Some(BaseAction::PaperLiveToggle),
                btn1: Some(BaseAction::Btn1),
                btn2: Some(BaseAction::AccountSelect),
                btn3: Some(BaseAction::RefreshData),
                btn4: Some(BaseAction::Btn4),
                btn5: Some(BaseAction::SymbolLockToggle),
                btn6: Some(BaseAction::Btn6),
                btn7: Some(BaseAction::Btn7),
                btn8: Some(BaseAction::SnapshotBackup),
            },
            buy_keys: BuyKeysState {
                btn0: Some(BuyAction::MarketBuy),
                btn1: Some(BuyAction::LimitBuy),
                btn2: Some(BuyAction::BracketBuy),
                btn3: Some(BuyAction::StopEntryBuy),
                btn4: Some(BuyAction::ScaleIn25),
                btn5: Some(BuyAction::ScaleIn50),
                btn6: Some(BuyAction::FlattenCurrentSymbol),
                btn7: Some(BuyAction::Btn7),
                btn8: Some(BuyAction::ReverseToLong),
            },
            sell_keys: SellKeysState {
                btn0: Some(SellAction::MarketSell),
                btn1: Some(SellAction::LimitSell),
                btn2: Some(SellAction::BracketSell),
                btn3: Some(SellAction::StopEntrySell),
                btn4: Some(SellAction::ScaleOut25),
                btn5: Some(SellAction::ScaleOut50),
                btn6: Some(SellAction::FlattenCurrentSymbol),
                btn7: Some(SellAction::Btn7),
                btn8: Some(SellAction::ReverseToShort),
            },
            key_pressed: [false; KEY_COUNT],
            last_key_event: None,
            enc1: Enc1Value::Shares(100),
            enc2: Enc2Value::Ticks(5),
            enc3: Enc3Value::MidOffset(0),
            last_encoder_event: None,
        }
    }
}

impl TradingState {
    pub fn is_action_armed_sensitive(&self, layer: u8, idx: u8) -> bool {
        match layer {
            1 => { // Buy layer
                match idx {
                    0 => matches!(self.buy_keys.btn0, Some(BuyAction::MarketBuy)),
                    8 => matches!(self.buy_keys.btn8, Some(BuyAction::ReverseToLong)),
                    _ => false,
                }
            }
            2 => { // Sell layer
                match idx {
                    0 => matches!(self.sell_keys.btn0, Some(SellAction::MarketSell)),
                    8 => matches!(self.sell_keys.btn8, Some(SellAction::ReverseToShort)),
                    _ => false,
                }
            }
            _ => false,
        }
    }
    
    pub fn is_button_bound(&self, layer: u8, idx: u8) -> bool {
        match layer {
            0 => { // Base layer
                match idx {
                    0 => self.base_keys.btn0.is_some(),
                    1 => self.base_keys.btn1.is_some(),
                    2 => self.base_keys.btn2.is_some(),
                    3 => self.base_keys.btn3.is_some(),
                    4 => self.base_keys.btn4.is_some(),
                    5 => self.base_keys.btn5.is_some(),
                    6 => self.base_keys.btn6.is_some(),
                    7 => self.base_keys.btn7.is_some(),
                    8 => self.base_keys.btn8.is_some(),
                    _ => false,
                }
            }
            1 => { // Buy layer
                match idx {
                    0 => self.buy_keys.btn0.is_some(),
                    1 => self.buy_keys.btn1.is_some(),
                    2 => self.buy_keys.btn2.is_some(),
                    3 => self.buy_keys.btn3.is_some(),
                    4 => self.buy_keys.btn4.is_some(),
                    5 => self.buy_keys.btn5.is_some(),
                    6 => self.buy_keys.btn6.is_some(),
                    7 => self.buy_keys.btn7.is_some(),
                    8 => self.buy_keys.btn8.is_some(),
                    _ => false,
                }
            }
            2 => { // Sell layer
                match idx {
                    0 => self.sell_keys.btn0.is_some(),
                    1 => self.sell_keys.btn1.is_some(),
                    2 => self.sell_keys.btn2.is_some(),
                    3 => self.sell_keys.btn3.is_some(),
                    4 => self.sell_keys.btn4.is_some(),
                    5 => self.sell_keys.btn5.is_some(),
                    6 => self.sell_keys.btn6.is_some(),
                    7 => self.sell_keys.btn7.is_some(),
                    8 => self.sell_keys.btn8.is_some(),
                    _ => false,
                }
            }
            _ => false,
        }
    }
    
    pub fn handle_button(&mut self, layer: u8, idx: u8, pressed: bool) {
        if (idx as usize) < KEY_COUNT {
            self.key_pressed[idx as usize] = pressed;
            self.last_key_event = Some((idx, pressed));
            
            if pressed {
                match layer {
                    0 => self.handle_base_action(idx),
                    1 => self.handle_buy_action(idx),
                    2 => self.handle_sell_action(idx),
                    _ => {}
                }
            }
        }
    }
    
    fn handle_base_action(&mut self, idx: u8) {
        let action = match idx {
            0 => self.base_keys.btn0,
            1 => self.base_keys.btn1,
            2 => self.base_keys.btn2,
            3 => self.base_keys.btn3,
            4 => self.base_keys.btn4,
            5 => self.base_keys.btn5,
            6 => self.base_keys.btn6,
            7 => self.base_keys.btn7,
            8 => self.base_keys.btn8,
            _ => None,
        };
        
        if let Some(action) = action {
            match action {
                BaseAction::PaperLiveToggle => {
                    self.account_type = match self.account_type {
                        AccountType::Paper => AccountType::Live,
                        AccountType::Live => AccountType::Paper,
                    };
                }
                BaseAction::AccountSelect => {
                    self.account = (self.account + 1) % 10;
                }
                _ => {}
            }
        }
    }
    
    fn handle_buy_action(&mut self, idx: u8) {
        let action = match idx {
            0 => self.buy_keys.btn0,
            1 => self.buy_keys.btn1,
            2 => self.buy_keys.btn2,
            3 => self.buy_keys.btn3,
            4 => self.buy_keys.btn4,
            5 => self.buy_keys.btn5,
            6 => self.buy_keys.btn6,
            7 => self.buy_keys.btn7,
            8 => self.buy_keys.btn8,
            _ => None,
        };
        
        if let Some(action) = action {
            // Check if action requires arming
            match action {
                BuyAction::MarketBuy | BuyAction::ReverseToLong => {
                    if !self.armed {
                        println!("Action {:?} blocked - system not armed", action);
                        return;
                    }
                    println!("Executing {:?}", action);
                }
                _ => {
                    println!("Executing {:?}", action);
                }
            }
        }
    }
    
    fn handle_sell_action(&mut self, idx: u8) {
        let action = match idx {
            0 => self.sell_keys.btn0,
            1 => self.sell_keys.btn1,
            2 => self.sell_keys.btn2,
            3 => self.sell_keys.btn3,
            4 => self.sell_keys.btn4,
            5 => self.sell_keys.btn5,
            6 => self.sell_keys.btn6,
            7 => self.sell_keys.btn7,
            8 => self.sell_keys.btn8,
            _ => None,
        };
        
        if let Some(action) = action {
            // Check if action requires arming
            match action {
                SellAction::MarketSell | SellAction::ReverseToShort => {
                    if !self.armed {
                        println!("Action {:?} blocked - system not armed", action);
                        return;
                    }
                    println!("Executing {:?}", action);
                }
                _ => {
                    println!("Executing {:?}", action);
                }
            }
        }
    }
    
    pub fn handle_encoder_rotate(&mut self, idx: u8, delta: i8) {
        self.last_encoder_event = Some((idx, delta as i16));
        
        match idx {
            0 => {
                match &mut self.enc1 {
                    Enc1Value::Shares(v) => *v = (*v + delta as i32).max(0),
                    Enc1Value::PercentBuyingPower(v) => *v = (*v + delta as i32).clamp(0, 100),
                    Enc1Value::RiskDollars(v) => *v = (*v + delta as i32 * 10).max(0),
                }
            }
            1 => {
                match &mut self.enc2 {
                    Enc2Value::Ticks(v) => *v = (*v + delta as i32).max(0),
                    Enc2Value::Percent(v) => *v = (*v + delta as i32).clamp(-100, 100),
                    Enc2Value::Atr(v) => *v = (*v + delta as i32).max(0),
                }
            }
            2 => {
                match &mut self.enc3 {
                    Enc3Value::BidOffset(v) |
                    Enc3Value::MidOffset(v) |
                    Enc3Value::AskOffset(v) => *v += delta as i32,
                }
            }
            _ => {}
        }
    }
    
    pub fn handle_encoder_press(&mut self, idx: u8) {
        match idx {
            0 => {
                self.enc1 = match self.enc1 {
                    Enc1Value::Shares(v) => Enc1Value::PercentBuyingPower(v.min(100)),
                    Enc1Value::PercentBuyingPower(v) => Enc1Value::RiskDollars(v * 10),
                    Enc1Value::RiskDollars(v) => Enc1Value::Shares(v / 10),
                };
            }
            1 => {
                self.enc2 = match self.enc2 {
                    Enc2Value::Ticks(v) => Enc2Value::Percent(v.min(100)),
                    Enc2Value::Percent(v) => Enc2Value::Atr(v.abs()),
                    Enc2Value::Atr(v) => Enc2Value::Ticks(v),
                };
            }
            2 => {
                self.enc3 = match self.enc3 {
                    Enc3Value::BidOffset(v) => Enc3Value::MidOffset(v),
                    Enc3Value::MidOffset(v) => Enc3Value::AskOffset(v),
                    Enc3Value::AskOffset(v) => Enc3Value::BidOffset(v),
                };
            }
            _ => {}
        }
    }
    
    pub fn set_layer(&mut self, layer: Layer) {
        self.layer = layer;
    }
}