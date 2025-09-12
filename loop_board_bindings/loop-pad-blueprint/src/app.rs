use std::sync::Arc;
use std::time::{Duration, Instant};

use crossbeam_channel::{unbounded, Receiver, Sender};
use eframe::egui::{self, Color32, RichText};
use log::{debug, info};

use crate::hid;
use crate::state;
use crate::ui;

use hid::protocol::{PadEvent, Rgb};
use state::app_state::{AppLayer, AppState, Enc1Value, Enc2Value, Enc3Value};

pub struct LoopPadApp {
    state: AppState,
    rx: Option<Receiver<PadEvent>>,
    tx: Sender<PadEvent>,
    #[allow(dead_code)]
    hid: Option<Arc<hid::device::HidPad>>, // available when feature "hid" is enabled
}

impl LoopPadApp {
    pub fn new(_cc: &eframe::CreationContext<'_>) -> Self {
        let (tx, rx) = unbounded();

        // Initial demo labels per layer
        let mut state = AppState::default();
        state.key_labels.base = [
            "Paper/Live".into(),
            "".into(),
            "Account".into(),
            "Refresh".into(),
            "".into(),
            "Sym Lock".into(),
            "".into(),
            "".into(),
            "Snapshot".into(),
        ];
        state.key_labels.buy = [
            "Mkt Buy".into(),
            "Lmt Buy".into(),
            "Bracket".into(),
            "StopEntry".into(),
            "+25%".into(),
            "+50%".into(),
            "Flatten".into(),
            "".into(),
            "Reverse L".into(),
        ];
        state.key_labels.sell = [
            "Mkt Sell".into(),
            "Lmt Sell".into(),
            "Bracket".into(),
            "StopEntry".into(),
            "-25%".into(),
            "-50%".into(),
            "Flatten".into(),
            "".into(),
            "Reverse S".into(),
        ];

        // Load configuration
        let cfg = crate::config::Config::load();

        // Start HID (if enabled) or fall back to mock (if enabled)
        #[allow(unused_mut)]
        let mut hid: Option<Arc<hid::device::HidPad>> = None;

        // Mock takes precedence if explicitly enabled
        let mut started_mock = false;

        #[cfg(feature = "mock")]
        if cfg.mock_enabled {
            info!("Starting mock input (env/CLI)");
            let _jh = hid::mock::start_mock(tx.clone());
            started_mock = true;
        }

        #[cfg(feature = "hid")]
        if !started_mock {
            match hid::device::HidPad::open(cfg.vid, cfg.pid, cfg.usage_page, cfg.usage)
                .map(|p| p.read_loop(tx.clone()))
            {
                Ok(h) => {
                    info!("HID interface started (VID={:04X}, PID={:04X})", cfg.vid, cfg.pid);
                    hid = Some(h);
                }
                Err(e) => {
                    log::warn!("Failed to open HID device: {}", e);
                    #[cfg(feature = "mock")]
                    {
                        info!("Falling back to mock input");
                        let _jh = hid::mock::start_mock(tx.clone());
                        started_mock = true;
                    }
                }
            }
        }

        Self {
            state,
            rx: Some(rx),
            tx,
            hid,
        }
    }

    fn now() -> Instant { Instant::now() }

    fn highlight_active(ts: Option<Instant>, window: Duration) -> bool {
        ts.map(|t| Self::now().saturating_duration_since(t) < window)
            .unwrap_or(false)
    }

    fn desired_leds(&self) -> [Rgb; state::app_state::KEY_COUNT] {
        let mut leds = [Rgb::OFF; state::app_state::KEY_COUNT];
        let is_base = matches!(self.state.layer, AppLayer::Base);
        for i in 0..state::app_state::KEY_COUNT {
            // Precedence: pressed-blue > flashing (pending TBD) > steady (armed) > off
            if self.state.key_pressed[i] {
                leds[i] = Rgb::BLUE;
            } else if self.state.armed && is_base {
                // No pending-flash yet; steady white
                leds[i] = Rgb::WHITE;
            } else {
                leds[i] = Rgb::OFF;
            }
        }
        leds
    }

    fn sync_leds_if_needed(&mut self) {
        // Only write on BASE layer and if HID available
        let Some(hid) = &self.hid else { return };
        if !matches!(self.state.layer, AppLayer::Base) { return; }

        let now = Self::now();
        let throttle = Duration::from_millis(75);
        if let Some(last) = self.state.last_led_update {
            if now.saturating_duration_since(last) < throttle && !self.state.needs_led_sync {
                return;
            }
            if now.saturating_duration_since(last) < throttle && self.state.needs_led_sync {
                // Still throttle
                return;
            }
        }

        let desired = self.desired_leds();
        let mut any = false;
        for (i, (want, had)) in desired.iter().zip(self.state.base_leds.iter()).enumerate() {
            if want != had {
                #[allow(unused_must_use)]
                { let _ = hid.set_led(i as u8, *want); }
                any = true;
            }
        }
        if any {
            self.state.base_leds = desired;
            self.state.last_led_update = Some(now);
            self.state.needs_led_sync = false;
        }
    }

    fn force_full_led_pass(&mut self) {
        let Some(hid) = &self.hid else { return };
        if !matches!(self.state.layer, AppLayer::Base) { return; }
        let desired = self.desired_leds();
        for (i, rgb) in desired.iter().enumerate() {
            #[allow(unused_must_use)]
            { let _ = hid.set_led(i as u8, *rgb); }
        }
        self.state.base_leds = desired;
        self.state.last_led_update = Some(Self::now());
        self.state.needs_led_sync = false;
    }

    fn ui_controls(&mut self, ui: &mut egui::Ui) {
        ui.horizontal(|ui| {
            let is_base = self.state.layer == AppLayer::Base;
            let is_buy = self.state.layer == AppLayer::Buy;
            let is_sell = self.state.layer == AppLayer::Sell;

            if ui
                .selectable_label(is_base, RichText::new("BASE").strong())
                .clicked()
            {
                self.state.layer = AppLayer::Base;
                self.state.needs_led_sync = true;
                #[allow(unused_must_use)]
                {
                    if self.state.connected {
                        if let Some(ref hid) = self.hid {
                            let _ = hid.set_layer(0);
                        }
                    }
                }
            }
            if ui
                .selectable_label(is_buy, RichText::new("BUY").strong())
                .clicked()
            {
                self.state.layer = AppLayer::Buy;
                self.state.needs_led_sync = true;
                #[allow(unused_must_use)]
                {
                    if self.state.connected {
                        if let Some(ref hid) = self.hid {
                            let _ = hid.set_layer(1);
                        }
                    }
                }
            }
            if ui
                .selectable_label(is_sell, RichText::new("SELL").strong())
                .clicked()
            {
                self.state.layer = AppLayer::Sell;
                self.state.needs_led_sync = true;
                #[allow(unused_must_use)]
                {
                    if self.state.connected {
                        if let Some(ref hid) = self.hid {
                            let _ = hid.set_layer(2);
                        }
                    }
                }
            }

            ui.separator();

            if ui
                .button(if self.state.armed { "Disarm" } else { "ARM" })
                .clicked()
            {
                self.state.armed = !self.state.armed;
                self.state.needs_led_sync = true;
            }
        });
    }

    fn ui_encoders(&mut self, ui: &mut egui::Ui) {
        ui.vertical(|ui| {
            ui.heading("Encoders");

            // Enc1
            ui.horizontal(|ui| {
                let blue = Self::highlight_active(self.state.enc_last_ts[0], Duration::from_millis(200));
                let color = if blue { Color32::BLUE } else { Color32::GRAY };
                ui.colored_label(color, "Enc1");
                let mut val = match self.state.enc1 {
                    Enc1Value::Shares(v) => v,
                    Enc1Value::PercentBuyingPower(v) => v,
                    Enc1Value::RiskDollars(v) => v,
                };
                let mut txt = val.to_string();
                if ui.text_edit_singleline(&mut txt).lost_focus() {
                    if let Ok(n) = txt.parse::<i32>() {
                        val = n;
                        match self.state.enc1 {
                            Enc1Value::Shares(_) => self.state.enc1 = Enc1Value::Shares(val),
                            Enc1Value::PercentBuyingPower(_) => {
                                self.state.enc1 = Enc1Value::PercentBuyingPower(val)
                            }
                            Enc1Value::RiskDollars(_) => {
                                self.state.enc1 = Enc1Value::RiskDollars(val)
                            }
                        }
                    }
                }
                if ui.button("Cycle").clicked() {
                    self.state.enc1 = match self.state.enc1 {
                        Enc1Value::Shares(v) => Enc1Value::PercentBuyingPower(v),
                        Enc1Value::PercentBuyingPower(v) => Enc1Value::RiskDollars(v),
                        Enc1Value::RiskDollars(v) => Enc1Value::Shares(v),
                    };
                    self.state.enc_last_ts[0] = Some(Self::now());
                }
            });

            // Enc2
            ui.horizontal(|ui| {
                let blue = Self::highlight_active(self.state.enc_last_ts[1], Duration::from_millis(200));
                let color = if blue { Color32::BLUE } else { Color32::GRAY };
                ui.colored_label(color, "Enc2");
                let mut val = match self.state.enc2 {
                    Enc2Value::Ticks(v) => v,
                    Enc2Value::Percent(v) => v,
                    Enc2Value::Atr(v) => v,
                };
                let mut txt = val.to_string();
                if ui.text_edit_singleline(&mut txt).lost_focus() {
                    if let Ok(n) = txt.parse::<i32>() {
                        val = n;
                        match self.state.enc2 {
                            Enc2Value::Ticks(_) => self.state.enc2 = Enc2Value::Ticks(val),
                            Enc2Value::Percent(_) => self.state.enc2 = Enc2Value::Percent(val),
                            Enc2Value::Atr(_) => self.state.enc2 = Enc2Value::Atr(val),
                        }
                    }
                }
                if ui.button("Cycle").clicked() {
                    self.state.enc2 = match self.state.enc2 {
                        Enc2Value::Ticks(v) => Enc2Value::Percent(v),
                        Enc2Value::Percent(v) => Enc2Value::Atr(v),
                        Enc2Value::Atr(v) => Enc2Value::Ticks(v),
                    };
                    self.state.enc_last_ts[1] = Some(Self::now());
                }
            });

            // Enc3
            ui.horizontal(|ui| {
                let blue = Self::highlight_active(self.state.enc_last_ts[2], Duration::from_millis(200));
                let color = if blue { Color32::BLUE } else { Color32::GRAY };
                ui.colored_label(color, "Enc3");
                let mut val = match self.state.enc3 {
                    Enc3Value::BidOffset(v) => v,
                    Enc3Value::MidOffset(v) => v,
                    Enc3Value::AskOffset(v) => v,
                };
                let mut txt = val.to_string();
                if ui.text_edit_singleline(&mut txt).lost_focus() {
                    if let Ok(n) = txt.parse::<i32>() {
                        val = n;
                        match self.state.enc3 {
                            Enc3Value::BidOffset(_) => self.state.enc3 = Enc3Value::BidOffset(val),
                            Enc3Value::MidOffset(_) => self.state.enc3 = Enc3Value::MidOffset(val),
                            Enc3Value::AskOffset(_) => self.state.enc3 = Enc3Value::AskOffset(val),
                        }
                    }
                }
                if ui.button("Cycle").clicked() {
                    self.state.enc3 = match self.state.enc3 {
                        Enc3Value::BidOffset(v) => Enc3Value::MidOffset(v),
                        Enc3Value::MidOffset(v) => Enc3Value::AskOffset(v),
                        Enc3Value::AskOffset(v) => Enc3Value::BidOffset(v),
                    };
                    self.state.enc_last_ts[2] = Some(Self::now());
                }
            });
        });
    }

    fn ui_keyboard(&mut self, ui: &mut egui::Ui) {
        use egui::Vec2;
        ui.heading("Keyboard");
        let labels = match self.state.layer {
            AppLayer::Base => &self.state.key_labels.base,
            AppLayer::Buy => &self.state.key_labels.buy,
            AppLayer::Sell => &self.state.key_labels.sell,
        };

        let circle_size = 10.0;

        egui::Grid::new("key-grid").num_columns(3).show(ui, |ui| {
            for row in 0..3 {
                for col in 0..3 {
                    let idx = row * 3 + col;
                    ui.vertical(|ui| {
                        ui.label(&labels[idx]);
                        let hi = Self::highlight_active(self.state.key_last_ts[idx], Duration::from_millis(200));
                        let color = if hi {
                            Color32::BLUE
                        } else if self.state.armed {
                            Color32::GREEN
                        } else {
                            Color32::RED
                        };
                        ui.painter().circle_filled(
                            ui.available_rect_before_wrap().center(),
                            circle_size,
                            color,
                        );
                        ui.add_space(20.0);
                    });
                }
                ui.end_row();
            }
        });
    }
}

impl eframe::App for LoopPadApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Ensure periodic repaint for timers/highlights
        ctx.request_repaint_after(Duration::from_millis(16));

        // Update flash phase (@1Hz)
        if Self::now().saturating_duration_since(self.state.last_flash_toggle) >= Duration::from_secs(1)
        {
            self.state.flash_phase = !self.state.flash_phase;
            self.state.last_flash_toggle = Self::now();
            self.state.needs_led_sync = true;
        }

        // Drain events if any (non-blocking), then handle to avoid borrow issues
        let mut events = Vec::new();
        if let Some(rx) = &self.rx {
            while let Ok(evt) = rx.try_recv() { events.push(evt); }
        }
        for evt in events {
            debug!("Event: {:?}", evt);
            match evt {
                PadEvent::Hello { proto, fw_major, fw_minor } => {
                    self.state.connected = true;
                    self.state.fw = Some((proto, fw_major, fw_minor));
                    #[allow(unused_must_use)]
                    {
                        if let Some(ref hid) = self.hid {
                            let _ = hid.send_ready();
                            // Set device layer to current UI selection
                            let _ = match self.state.layer {
                                AppLayer::Base => hid.set_layer(0),
                                AppLayer::Buy => hid.set_layer(1),
                                AppLayer::Sell => hid.set_layer(2),
                            };
                        }
                    }
                    // Full LED pass (BASE only)
                    self.force_full_led_pass();
                }
                PadEvent::Layer { layer } => {
                    self.state.layer = match layer {
                        0 => AppLayer::Base,
                        1 => AppLayer::Buy,
                        2 => AppLayer::Sell,
                        _ => self.state.layer,
                    };
                    self.state.needs_led_sync = true;
                }
                PadEvent::Button { idx, pressed, .. } => {
                    let i = (idx as usize).min(8);
                    self.state.key_pressed[i] = pressed;
                    if pressed {
                        self.state.key_last_ts[i] = Some(Self::now());
                    } else {
                        self.state.key_last_ts[i] = None; // clear highlight on release
                    }
                    self.state.last_key_event = Some((idx, pressed));
                    self.state.needs_led_sync = true;
                }
                PadEvent::EncRotate { idx, delta } => {
                    let delta = delta as i32;
                    match idx {
                        0 => match self.state.enc1 {
                            Enc1Value::Shares(v) => self.state.enc1 = Enc1Value::Shares(v + delta),
                            Enc1Value::PercentBuyingPower(v) => {
                                self.state.enc1 = Enc1Value::PercentBuyingPower(v + delta)
                            }
                            Enc1Value::RiskDollars(v) => {
                                self.state.enc1 = Enc1Value::RiskDollars(v + delta)
                            }
                        },
                        1 => match self.state.enc2 {
                            Enc2Value::Ticks(v) => self.state.enc2 = Enc2Value::Ticks(v + delta),
                            Enc2Value::Percent(v) => self.state.enc2 = Enc2Value::Percent(v + delta),
                            Enc2Value::Atr(v) => self.state.enc2 = Enc2Value::Atr(v + delta),
                        },
                        2 => match self.state.enc3 {
                            Enc3Value::BidOffset(v) => self.state.enc3 = Enc3Value::BidOffset(v + delta),
                            Enc3Value::MidOffset(v) => self.state.enc3 = Enc3Value::MidOffset(v + delta),
                            Enc3Value::AskOffset(v) => self.state.enc3 = Enc3Value::AskOffset(v + delta),
                        },
                        _ => {}
                    }
                    self.state.enc_last_ts[(idx as usize).min(2)] = Some(Self::now());
                    self.state.last_encoder_event = Some((idx, delta as i16));
                }
                PadEvent::EncPress { idx, long: _ } => {
                    match idx {
                        0 => {
                            self.state.enc1 = match self.state.enc1 {
                                Enc1Value::Shares(v) => Enc1Value::PercentBuyingPower(v),
                                Enc1Value::PercentBuyingPower(v) => Enc1Value::RiskDollars(v),
                                Enc1Value::RiskDollars(v) => Enc1Value::Shares(v),
                            }
                        }
                        1 => {
                            self.state.enc2 = match self.state.enc2 {
                                Enc2Value::Ticks(v) => Enc2Value::Percent(v),
                                Enc2Value::Percent(v) => Enc2Value::Atr(v),
                                Enc2Value::Atr(v) => Enc2Value::Ticks(v),
                            }
                        }
                        2 => {
                            self.state.enc3 = match self.state.enc3 {
                                Enc3Value::BidOffset(v) => Enc3Value::MidOffset(v),
                                Enc3Value::MidOffset(v) => Enc3Value::AskOffset(v),
                                Enc3Value::AskOffset(v) => Enc3Value::BidOffset(v),
                            }
                        }
                        _ => {}
                    }
                    self.state.enc_last_ts[(idx as usize).min(2)] = Some(Self::now());
                }
                PadEvent::MidiCc { .. } => {}
            }
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.vertical(|ui| {
                self.ui_controls(ui);
                ui.separator();
                self.ui_encoders(ui);
                ui.separator();
                self.ui_keyboard(ui);
            });
        });

        // LED sync (BASE only)
        self.sync_leds_if_needed();
    }
}
