use std::fs;
use std::path::Path;

#[derive(Debug, Clone, Copy)]
pub struct Config {
    pub vid: u16,
    pub pid: u16,
    pub usage_page: Option<u16>,
    pub usage: Option<u16>,
    pub mock_enabled: bool,
}

fn parse_u16_auto(s: &str) -> Option<u16> {
    let s = s.trim();
    if let Some(hex) = s.strip_prefix("0x").or_else(|| s.strip_prefix("0X")) {
        u16::from_str_radix(hex, 16).ok()
    } else {
        s.parse::<u16>().ok()
    }
}

fn parse_bool_auto(s: &str) -> Option<bool> {
    match s.trim().to_ascii_lowercase().as_str() {
        "1" | "true" | "yes" | "on" => Some(true),
        "0" | "false" | "no" | "off" => Some(false),
        _ => None,
    }
}

fn load_from_toml_like(path: &Path) -> Option<Config> {
    let Ok(data) = fs::read_to_string(path) else { return None };
    let mut cfg = Config::default();
    for line in data.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') { continue; }
        let mut parts = line.splitn(2, '=');
        let key = parts.next().map(|s| s.trim());
        let val = parts.next().map(|s| s.trim().trim_matches('"'));
        if let (Some(k), Some(v)) = (key, val) {
            match k {
                "vid" => if let Some(n) = parse_u16_auto(v) { cfg.vid = n },
                "pid" => if let Some(n) = parse_u16_auto(v) { cfg.pid = n },
                "usage_page" => cfg.usage_page = parse_u16_auto(v),
                "usage" => cfg.usage = parse_u16_auto(v),
                "mock_enabled" => if let Some(b) = parse_bool_auto(v) { cfg.mock_enabled = b },
                _ => {}
            }
        }
    }
    Some(cfg)
}

impl Default for Config {
    fn default() -> Self {
        Self { 
            vid: 0x574C,  // Work Louder
            pid: 0x1DF9,  // Loop
            usage_page: Some(0xFF60), 
            usage: Some(0x0061), 
            mock_enabled: false 
        }
    }
}

impl Config {
    pub fn load() -> Self {
        let mut cfg = Config::default();

        // File: prefer local padtest.toml if present
        if let Some(file_cfg) = load_from_toml_like(Path::new("padtest.toml")) {
            cfg = file_cfg;
        }

        // Env overrides
        if let Ok(v) = std::env::var("LOOPPAD_VID") { if let Some(n) = parse_u16_auto(&v) { cfg.vid = n } }
        if let Ok(v) = std::env::var("LOOPPAD_PID") { if let Some(n) = parse_u16_auto(&v) { cfg.pid = n } }
        if let Ok(v) = std::env::var("LOOPPAD_USAGE_PAGE") { cfg.usage_page = parse_u16_auto(&v) }
        if let Ok(v) = std::env::var("LOOPPAD_USAGE") { cfg.usage = parse_u16_auto(&v) }
        if let Ok(v) = std::env::var("LOOPPAD_MOCK") { if let Some(b) = parse_bool_auto(&v) { cfg.mock_enabled = b } }

        // CLI arg switch
        if std::env::args().any(|a| a == "--mock" || a == "--no-hid") {
            cfg.mock_enabled = true;
        }

        cfg
    }
}

