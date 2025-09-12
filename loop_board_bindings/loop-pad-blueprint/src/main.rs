use eframe::egui;

mod app;
mod hid;
mod state;
mod ui;
mod config;

fn main() -> eframe::Result<()> {
    // Init logger once
    env_logger::init();

    let native_options = eframe::NativeOptions::default();
    eframe::run_native(
        "Loop Pad Blueprint",
        native_options,
        Box::new(|cc| Box::new(app::LoopPadApp::new(cc))),
    )
}
