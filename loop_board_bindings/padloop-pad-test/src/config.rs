use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use std::path::Path;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    #[serde(default)]
    pub hid: HidConfig,
    #[serde(default)]
    pub midi: MidiConfig,
    #[serde(default)]
    pub demo: DemoConfig,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HidConfig {
    pub vid: u16,
    pub pid: u16,
    pub usage_page: Option<u16>,
    pub usage: Option<u16>,
}

impl Default for HidConfig {
    fn default() -> Self {
        Self {
            vid: 0x574C,     // Work Louder
            pid: 0x1DF9,     // Loop
            usage_page: Some(0xFF60),
            usage: Some(0x0061),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MidiConfig {
    pub port_hint: Option<String>,
    #[serde(default)]
    pub enabled: bool,
}

impl Default for MidiConfig {
    fn default() -> Self {
        Self {
            port_hint: Some("Loop".to_string()),
            enabled: false,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DemoConfig {
    pub start_layer: u8,
    pub start_color: RgbConfig,
    #[serde(default = "default_arm_timeout")]
    pub arm_timeout_seconds: u64,
}

fn default_arm_timeout() -> u64 {
    30
}

impl Default for DemoConfig {
    fn default() -> Self {
        Self {
            start_layer: 0,
            start_color: RgbConfig {
                r: 255,
                g: 255,
                b: 255,
            },
            arm_timeout_seconds: 30,
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RgbConfig {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

impl Config {
    /// Create a mock configuration for testing
    pub fn mock() -> Self {
        Self {
            hid: HidConfig {
                vid: 0,  // Setting to 0 forces mock mode
                pid: 0,
                usage_page: None,
                usage: None,
            },
            midi: MidiConfig {
                enabled: false,
                port_hint: None,
            },
            demo: DemoConfig::default(),
        }
    }

    /// Load configuration from file
    pub fn load_from_file<P: AsRef<Path>>(path: P) -> Result<Self> {
        let contents = std::fs::read_to_string(path)
            .context("Failed to read configuration file")?;
        
        toml::from_str(&contents)
            .context("Failed to parse configuration")
    }

    /// Load configuration from default location
    pub fn load() -> Result<Self> {
        // Try multiple locations
        let paths = [
            "padtest.toml",
            "config/padtest.toml",
            "../padtest.toml",
        ];

        for path in &paths {
            if Path::new(path).exists() {
                return Self::load_from_file(path);
            }
        }

        // If no config file found, use defaults
        Ok(Self::default())
    }

    /// Save configuration to file
    pub fn save_to_file<P: AsRef<Path>>(&self, path: P) -> Result<()> {
        let toml = toml::to_string_pretty(self)
            .context("Failed to serialize configuration")?;
        
        std::fs::write(path, toml)
            .context("Failed to write configuration file")?;
        
        Ok(())
    }
}

impl Default for Config {
    fn default() -> Self {
        Self {
            hid: HidConfig::default(),
            midi: MidiConfig::default(),
            demo: DemoConfig::default(),
        }
    }
}

impl From<RgbConfig> for crate::event::Rgb {
    fn from(cfg: RgbConfig) -> Self {
        crate::event::Rgb {
            r: cfg.r,
            g: cfg.g,
            b: cfg.b,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_default_config() {
        let config = Config::default();
        assert_eq!(config.hid.vid, 0x574C);
        assert_eq!(config.hid.pid, 0x1DF9);
        assert_eq!(config.demo.start_layer, 0);
    }

    #[test]
    fn test_config_serialization() {
        let config = Config::default();
        let toml = toml::to_string(&config).unwrap();
        let parsed: Config = toml::from_str(&toml).unwrap();
        assert_eq!(parsed.hid.vid, config.hid.vid);
    }
}