use eframe::egui;
use padloop2::{Config, Pad, PadEvent};
use std::sync::{Arc, Mutex};
use std::thread;
use crossbeam_channel::{unbounded, Sender};

#[derive(Clone)]
struct AppState {
    armed: bool,
}

impl Default for AppState {
    fn default() -> Self {
        Self {
            armed: false,
        }
    }
}

struct PadGuiApp {
    key_states: Arc<Mutex<[bool; 36]>>,
    encoder_values: Arc<Mutex<[i32; 4]>>,
    layer_tx: Sender<u8>,
    current_layer: Arc<Mutex<u8>>,
    app_state: Arc<Mutex<AppState>>,
}

impl PadGuiApp {
    fn new(layer_tx: Sender<u8>) -> Self {
        Self {
            key_states: Arc::new(Mutex::new([false; 36])),
            encoder_values: Arc::new(Mutex::new([0; 4])),
            layer_tx,
            current_layer: Arc::new(Mutex::new(0)),
            app_state: Arc::new(Mutex::new(AppState::default())),
        }
    }
}

impl eframe::App for PadGuiApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ctx.request_repaint();
        
        egui::CentralPanel::default().show(ctx, |ui| {
            let key_states = self.key_states.lock().unwrap();
            let encoder_values = self.encoder_values.lock().unwrap();
            let current_layer = *self.current_layer.lock().unwrap();
            let mut app_state = self.app_state.lock().unwrap();
            
            ui.vertical(|ui| {
                // Armed status display
                let armed_bg = if app_state.armed {
                    egui::Color32::from_rgb(0, 200, 0)
                } else {
                    egui::Color32::from_rgb(200, 200, 0)
                };
                
                ui.horizontal(|ui| {
                    let rect = ui.available_rect_before_wrap();
                    let rect = egui::Rect::from_min_size(rect.min, egui::Vec2::new(200.0, 60.0));
                    ui.painter().rect_filled(rect, 4.0, armed_bg);
                    ui.allocate_ui_at_rect(rect, |ui| {
                        ui.centered_and_justified(|ui| {
                            ui.label(egui::RichText::new("ARMED").size(30.0).color(egui::Color32::WHITE).strong());
                        });
                    });
                    
                    ui.add_space(10.0);
                    
                    if ui.button(if app_state.armed { "DISARM" } else { "ARM" }).clicked() {
                        app_state.armed = !app_state.armed;
                    }
                });
                
                ui.separator();
                
                ui.horizontal(|ui| {
                    ui.label("Layer Control:");
                    for layer in 0..3 {
                        if ui.button(format!("Layer {}", layer)).clicked() {
                            let _ = self.layer_tx.send(layer);
                        }
                    }
                    ui.label(format!("Current: {}", current_layer));
                });
                
                ui.separator();
                
                for layer in 0..3 {
                    ui.horizontal(|ui| {
                        ui.label(format!("Layer {}:", layer));
                        
                        for i in 0..12 {
                            let idx = layer * 12 + i;
                            let color = if key_states[idx] {
                                egui::Color32::GREEN
                            } else {
                                egui::Color32::RED
                            };
                            
                            let (response, painter) = ui.allocate_painter(
                                egui::Vec2::new(30.0, 30.0),
                                egui::Sense::hover()
                            );
                            
                            painter.circle_filled(
                                response.rect.center(),
                                10.0,
                                color
                            );
                            
                            if i < 4 && layer == 0 {
                                ui.label(format!("{}", encoder_values[i]));
                            }
                        }
                    });
                }
            });
        });
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let (layer_tx, layer_rx) = unbounded();
    let app = PadGuiApp::new(layer_tx);
    let key_states = app.key_states.clone();
    let encoder_values = app.encoder_values.clone();
    let current_layer = app.current_layer.clone();
    let app_state = app.app_state.clone();
    
    thread::spawn(move || {
        let config = Config::default();
        let mut pad = Pad::new(&config).expect("Failed to create pad");
        let rx = pad.spawn(&config).expect("Failed to spawn pad");
        
        loop {
            // Check for layer changes first
            if let Ok(layer) = layer_rx.try_recv() {
                if let Err(e) = pad.set_layer(layer) {
                    eprintln!("Failed to set layer: {}", e);
                }
            }
            
            // Then check for pad events with timeout
            match rx.recv_timeout(std::time::Duration::from_millis(10)) {
                Ok(event) => match event {
                PadEvent::Layer { layer } => {
                    if let Ok(mut current) = current_layer.lock() {
                        *current = layer;
                    }
                }
                PadEvent::Button { layer, idx, pressed } => {
                    if let Ok(mut states) = key_states.lock() {
                        let key_idx = (layer * 12 + idx) as usize;
                        if key_idx < 36 {
                            states[key_idx] = pressed;
                        }
                    }
                }
                PadEvent::EncRotate { idx, delta } => {
                    if let Ok(mut values) = encoder_values.lock() {
                        if idx < 4 {
                            values[idx as usize] += delta as i32;
                        }
                    }
                }
                PadEvent::EncPress { idx, long: _ } => {
                    if let Ok(mut states) = key_states.lock() {
                        if idx < 4 {
                            states[idx as usize] = true;
                        }
                    }
                }
                _ => {}
                },
                Err(crossbeam_channel::RecvTimeoutError::Timeout) => {
                    // No event, continue to check for layer changes
                }
                Err(crossbeam_channel::RecvTimeoutError::Disconnected) => {
                    break;
                }
            }
        }
    });
    
    let native_options = eframe::NativeOptions::default();
    eframe::run_native(
        "PadLoop2 GUI",
        native_options,
        Box::new(|_cc| Ok(Box::new(app)))
    )?;
    
    Ok(())
}