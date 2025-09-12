use eframe::egui;
use padloop2::{Config, Pad, PadEvent, Rgb};
use padloop2::trading_state::{TradingState, Layer, AccountType, Enc1Value, Enc2Value, Enc3Value, KEY_COUNT};
use std::sync::{Arc, Mutex};
use std::thread;
use crossbeam_channel::{unbounded, Sender};
use rdev::{listen, Event, EventType, Key};
use std::sync::atomic::{AtomicBool, Ordering};

fn update_all_leds(pad: &Pad, state: &TradingState, layer: u8) {
    println!("=== Starting LED update for layer {} ===", layer);
    println!("Armed state: {}", state.armed);
    
    // Define layer colors
    let layer_color = match layer {
        0 => Rgb { r: 100, g: 100, b: 100 }, // WHITE for BASE
        1 => Rgb { r: 0, g: 100, b: 0 },     // GREEN for BUY
        2 => Rgb { r: 100, g: 0, b: 0 },     // RED for SELL
        _ => Rgb { r: 100, g: 100, b: 100 }, // Default to WHITE
    };
    
    for idx in 0..9u8 {
        // Check if this button is armed-sensitive in the CURRENT layer
        let is_armed_sensitive = state.is_action_armed_sensitive(layer, idx);
        
        // Check if button is bound in the current layer
        let is_bound = state.is_button_bound(layer, idx);
        
        println!("Button {} in layer {}: bound={}, armed_sensitive={}", 
                 idx, layer, is_bound, is_armed_sensitive);
        
        let color = if is_bound {
            if is_armed_sensitive {
                if state.armed {
                    // Armed sensitive button when armed - use layer color
                    println!("  -> Setting LAYER COLOR (armed sensitive + armed)");
                    layer_color
                } else {
                    // Armed sensitive button when disarmed - LED off
                    println!("  -> Setting BLACK (armed sensitive + disarmed)");
                    Rgb { r: 0, g: 0, b: 0 }
                }
            } else {
                // Regular bound button - use layer color
                println!("  -> Setting LAYER COLOR (regular bound button)");
                layer_color
            }
        } else {
            // Unbound button - LED off
            println!("  -> Setting BLACK (unbound button)");
            Rgb { r: 0, g: 0, b: 0 }
        };
        
        println!("  Final: LED {} = RGB({},{},{})", idx, color.r, color.g, color.b);
        if let Err(e) = pad.set_led(idx, color) {
            eprintln!("Failed to set LED {}: {}", idx, e);
        }
    }
    println!("=== LED update complete ===");
}

struct PadGuiApp {
    state: Arc<Mutex<TradingState>>,
    layer_tx: Sender<u8>,
}

impl PadGuiApp {
    fn new(layer_tx: Sender<u8>) -> Self {
        Self {
            state: Arc::new(Mutex::new(TradingState::default())),
            layer_tx,
        }
    }
    
    fn draw_text_box(&self, ui: &mut egui::Ui, label: &str, value: &str, is_current: bool) {
        ui.horizontal(|ui| {
            ui.label(format!("{}: ", label));
            
            let text = egui::RichText::new(value).monospace();
            
            if is_current {
                ui.visuals_mut().widgets.noninteractive.bg_stroke = egui::Stroke::new(2.0, egui::Color32::WHITE);
                ui.add(egui::Label::new(text).sense(egui::Sense::hover()))
                    .on_hover_text(label);
                ui.visuals_mut().widgets.noninteractive.bg_stroke = egui::Stroke::NONE;
            } else {
                ui.label(text);
            }
        });
    }
}

impl eframe::App for PadGuiApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ctx.request_repaint();
        
        egui::CentralPanel::default().show(ctx, |ui| {
            let state = self.state.lock().unwrap();
            
            ui.vertical(|ui| {
                ui.heading("PadLoop2 Trading Application");
                ui.separator();
                
                // Account Section
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Account").strong());
                    ui.horizontal(|ui| {
                        let account_type_str = match state.account_type {
                            AccountType::Paper => "PAPER",
                            AccountType::Live => "LIVE",
                        };
                        self.draw_text_box(ui, "Type", account_type_str, true);
                        self.draw_text_box(ui, "Account #", &state.account.to_string(), true);
                        
                        let armed_color = if state.armed {
                            egui::Color32::GREEN
                        } else {
                            egui::Color32::YELLOW
                        };
                        ui.colored_label(armed_color, if state.armed { "ARMED" } else { "DISARMED" });
                    });
                });
                
                ui.separator();
                
                // Layer Control
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Layer Control").strong());
                    ui.horizontal(|ui| {
                        for layer in [Layer::Base, Layer::Buy, Layer::Sell] {
                            let is_current = state.layer == layer;
                            let layer_name = format!("{:?}", layer);
                            
                            if is_current {
                                ui.add(egui::Button::new(&layer_name).fill(egui::Color32::from_rgb(0, 100, 200)));
                            } else if ui.button(&layer_name).clicked() {
                                let layer_idx = match layer {
                                    Layer::Base => 0,
                                    Layer::Buy => 1,
                                    Layer::Sell => 2,
                                };
                                println!("GUI layer button clicked: {:?} -> {}", layer, layer_idx);
                                let _ = self.layer_tx.send(layer_idx);
                            }
                        }
                    });
                });
                
                ui.separator();
                
                // Encoder Values
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Encoders").strong());
                    
                    ui.horizontal(|ui| {
                        let (enc1_label, enc1_value) = match state.enc1 {
                            Enc1Value::Shares(v) => ("Shares", v.to_string()),
                            Enc1Value::PercentBuyingPower(v) => ("% Buying Power", format!("{}%", v)),
                            Enc1Value::RiskDollars(v) => ("Risk $", format!("${}", v)),
                        };
                        self.draw_text_box(ui, &format!("Enc1 ({})", enc1_label), &enc1_value, true);
                    });
                    
                    ui.horizontal(|ui| {
                        let (enc2_label, enc2_value) = match state.enc2 {
                            Enc2Value::Ticks(v) => ("Ticks", v.to_string()),
                            Enc2Value::Percent(v) => ("Percent", format!("{}%", v)),
                            Enc2Value::Atr(v) => ("ATR", format!("{:.2}", v as f32 / 100.0)),
                        };
                        self.draw_text_box(ui, &format!("Enc2 ({})", enc2_label), &enc2_value, true);
                    });
                    
                    ui.horizontal(|ui| {
                        let (enc3_label, enc3_value) = match state.enc3 {
                            Enc3Value::BidOffset(v) => ("Bid Offset", v.to_string()),
                            Enc3Value::MidOffset(v) => ("Mid Offset", v.to_string()),
                            Enc3Value::AskOffset(v) => ("Ask Offset", v.to_string()),
                        };
                        self.draw_text_box(ui, &format!("Enc3 ({})", enc3_label), &enc3_value, true);
                    });
                });
                
                ui.separator();
                
                // Key States Grid
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Key States").strong());
                    
                    let labels = match state.layer {
                        Layer::Base => &state.key_labels.base,
                        Layer::Buy => &state.key_labels.buy,
                        Layer::Sell => &state.key_labels.sell,
                    };
                    
                    let current_layer = match state.layer {
                        Layer::Base => 0,
                        Layer::Buy => 1,
                        Layer::Sell => 2,
                    };
                    
                    ui.columns(3, |cols| {
                        for row in 0..3 {
                            for col in 0..3 {
                                let idx = row * 3 + col;
                                let column_idx = col;
                                
                                cols[column_idx].horizontal(|ui| {
                                    let pressed = state.key_pressed[idx];
                                    let label = &labels[idx];
                                    
                                    // Check if this button requires arming
                                    let is_armed_sensitive = state.is_action_armed_sensitive(current_layer, idx as u8);
                                    let is_disabled = is_armed_sensitive && !state.armed;
                                    
                                    let color = if is_disabled {
                                        egui::Color32::from_rgb(40, 40, 40) // Dark gray for disabled
                                    } else if pressed {
                                        egui::Color32::GREEN
                                    } else {
                                        egui::Color32::GRAY
                                    };
                                    
                                    let rect = ui.allocate_space(egui::Vec2::new(120.0, 40.0)).1;
                                    
                                    // Only show white border if pressed AND not disabled
                                    if pressed && !is_disabled {
                                        ui.painter().rect_stroke(
                                            rect,
                                            4.0,
                                            egui::Stroke::new(2.0, egui::Color32::WHITE)
                                        );
                                    }
                                    
                                    ui.painter().rect_filled(
                                        rect.shrink(pressed.then_some(2.0).unwrap_or(0.0)),
                                        4.0,
                                        color
                                    );
                                    
                                    let text_color = if is_disabled {
                                        egui::Color32::from_rgb(80, 80, 80) // Dim text for disabled
                                    } else {
                                        egui::Color32::WHITE
                                    };
                                    
                                    ui.painter().text(
                                        rect.center(),
                                        egui::Align2::CENTER_CENTER,
                                        label,
                                        egui::FontId::proportional(12.0),
                                        text_color
                                    );
                                });
                            }
                        }
                    });
                });
                
                ui.separator();
                
                // Last Events
                ui.group(|ui| {
                    ui.label(egui::RichText::new("Last Events").strong());
                    
                    if let Some((key, pressed)) = state.last_key_event {
                        let event_text = format!("Key {} {}", key, if pressed { "pressed" } else { "released" });
                        self.draw_text_box(ui, "Last Key", &event_text, false);
                    }
                    
                    if let Some((enc, delta)) = state.last_encoder_event {
                        let event_text = format!("Encoder {} rotated {}", enc, delta);
                        self.draw_text_box(ui, "Last Encoder", &event_text, false);
                    }
                });
            });
        });
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let (layer_tx, layer_rx) = unbounded();
    let app = PadGuiApp::new(layer_tx);
    let state = app.state.clone();
    let keyboard_state = state.clone();
    
    // Global keyboard listener thread for ARM toggle
    thread::spawn(move || {
        let right_shift_pressed = Arc::new(AtomicBool::new(false));
        let right_shift_clone = right_shift_pressed.clone();
        
        let callback = move |event: Event| {
            match event.event_type {
                EventType::KeyPress(Key::ShiftRight) => {
                    right_shift_clone.store(true, Ordering::Relaxed);
                }
                EventType::KeyRelease(Key::ShiftRight) => {
                    right_shift_clone.store(false, Ordering::Relaxed);
                }
                EventType::KeyPress(Key::F12) => {
                    if right_shift_clone.load(Ordering::Relaxed) {
                        // Toggle ARM state when Right_Shift + F12 is pressed
                        if let Ok(mut s) = keyboard_state.lock() {
                            s.armed = !s.armed;
                            println!("ARM toggled to: {}", s.armed);
                        }
                    }
                }
                _ => {}
            }
        };
        
        // This will block the thread
        if let Err(e) = listen(callback) {
            eprintln!("Error listening to keyboard: {:?}", e);
        }
    });
    
    // Hardware event thread
    thread::spawn(move || {
        let config = Config::default();
        let mut pad = Pad::new(&config).expect("Failed to create pad");
        let rx = pad.spawn(&config).expect("Failed to spawn pad");
        
        // Send host ready signal
        if let Err(e) = pad.send_ready() {
            eprintln!("Failed to send host ready: {}", e);
        }
        
        // Small delay to ensure pad is ready
        thread::sleep(std::time::Duration::from_millis(100));
        
        // Initialize LEDs on startup (BASE layer)
        if let Ok(s) = state.lock() {
            update_all_leds(&pad, &s, 0); // Base layer on startup
        }
        
        // Track armed state for LED updates
        let mut last_armed_state = false;
        let mut last_layer = 0u8;
        
        loop {
            // Check for layer changes from GUI
            if let Ok(layer) = layer_rx.try_recv() {
                // Change layer
                if let Err(e) = pad.set_layer(layer) {
                    eprintln!("Failed to set layer: {}", e);
                }
                
                // Update state
                if let Ok(mut s) = state.lock() {
                    s.set_layer(match layer {
                        0 => Layer::Base,
                        1 => Layer::Buy,
                        2 => Layer::Sell,
                        _ => Layer::Base,
                    });
                    last_layer = layer;
                }
                
                // Update LEDs for the new layer
                if let Ok(s) = state.lock() {
                    update_all_leds(&pad, &s, layer);
                }
            }
            
            // Check if armed state changed and update LEDs
            if let Ok(s) = state.lock() {
                let current_layer = match s.layer {
                    Layer::Base => 0,
                    Layer::Buy => 1, 
                    Layer::Sell => 2,
                };
                
                if s.armed != last_armed_state {
                    last_armed_state = s.armed;
                    println!("Armed state changed to {} - updating LEDs", s.armed);
                    update_all_leds(&pad, &s, current_layer);
                }
            }
            
            // Process pad events
            match rx.recv_timeout(std::time::Duration::from_millis(10)) {
                Ok(event) => {
                    let mut s = state.lock().unwrap();
                    
                    match event {
                        PadEvent::Layer { layer } => {
                            s.set_layer(match layer {
                                0 => Layer::Base,
                                1 => Layer::Buy,
                                2 => Layer::Sell,
                                _ => Layer::Base,
                            });
                            last_layer = layer;
                            // Update LEDs for any layer change
                            update_all_leds(&pad, &s, layer);
                        }
                        PadEvent::Button { layer, idx, pressed } => {
                            s.handle_button(layer, idx, pressed);
                        }
                        PadEvent::EncRotate { idx, delta } => {
                            s.handle_encoder_rotate(idx, delta);
                        }
                        PadEvent::EncPress { idx, long: _ } => {
                            s.handle_encoder_press(idx);
                        }
                        _ => {}
                    }
                }
                Err(crossbeam_channel::RecvTimeoutError::Timeout) => {
                    // No event, continue
                }
                Err(crossbeam_channel::RecvTimeoutError::Disconnected) => {
                    break;
                }
            }
        }
    });
    
    let native_options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([800.0, 700.0]),
        ..Default::default()
    };
    
    eframe::run_native(
        "PadLoop2 Trading",
        native_options,
        Box::new(|_cc| Ok(Box::new(app)))
    )?;
    
    Ok(())
}