use hidapi::{HidApi, HidDevice};
use anyhow::Result;
use std::time::Duration;

const VID: u16 = 0x574C;  // Work Louder vendor ID
const PID: u16 = 0xDCD1;  // Work Board product ID

// HID commands
const CMD_FOCUS: u8 = 4;  // Focus on one LED
const CMD_CLEAR: u8 = 3;  // Clear all LEDs

// Keyboard notifications
const NOTIFY_KEYPRESS: u8 = 0xFE;  // Key press notification
const NOTIFY_LAYER: u8 = 0xFD;     // Layer change notification
const NOTIFY_ACK: u8 = 0xFF;       // Command acknowledgment

fn main() -> Result<()> {
    println!("Work Board Interactive Test - Press keys to change focus");
    println!("Looking for Work Board (VID: 0x{:04X}, PID: 0x{:04X})", VID, PID);
    
    let api = HidApi::new()?;
    
    // Find the RAW HID interface
    let device = api.device_list()
        .find(|d| {
            d.vendor_id() == VID && 
            d.product_id() == PID &&
            d.usage_page() == 0xFF60 &&
            d.usage() == 0x61
        })
        .ok_or_else(|| anyhow::anyhow!("Work Board RAW HID interface not found"))?;
    
    let dev = device.open_device(&api)?;
    dev.set_blocking_mode(false)?;  // Non-blocking for reading
    
    println!("Connected to Work Board!");
    println!("Press any key on the keyboard to focus that ticker");
    println!("The corresponding LED will light up white, others will be dim blue\n");
    
    // Define ticker positions (48 keys total)
    let tickers = [
        // Row 1: Numbers (indices 0-11)
        "AAPL", "GOOGL", "MSFT", "AMZN", "TSLA", "META", 
        "NVDA", "AMD", "INTC", "IBM", "ORCL", "NFLX",
        
        // Row 2: QWERTY row (indices 12-23)
        "DIS", "BA", "GE", "F", "GM", "CAT",
        "DE", "MMM", "JNJ", "PFE", "MRK", "CVX",
        
        // Row 3: ASDF row (indices 24-35)
        "XOM", "BP", "SHEL", "COP", "SLB", "HAL",
        "BKR", "OXY", "VLO", "MPC", "PSX", "PBF",
        
        // Row 4: Bottom row (indices 36-46)
        "COST", "WMT", "TGT", "HD", "LOW", "SBUX",
        "MCD", "NKE", "ADBE", "CRM", "PYPL", "SQ"
    ];
    
    let mut current_focus: Option<u8> = None;
    let mut current_layer: u8 = 0;
    
    // Clear any existing LED state
    let mut buf = [0u8; 32];
    buf[0] = CMD_CLEAR;
    dev.write(&buf)?;
    
    println!("Waiting for key presses...\n");
    
    loop {
        // Read HID reports from keyboard
        let mut read_buf = [0u8; 32];
        match dev.read_timeout(&mut read_buf, 10) {
            Ok(_) => {
                match read_buf[0] {
                    NOTIFY_KEYPRESS => {
                        // Key press notification: [0xFE, led_index, layer]
                        let led_index = read_buf[1];
                        let layer = read_buf[2];
                        
                        // Calculate actual ticker index based on layer
                        let ticker_index = if layer == 1 && led_index < 48 {
                            led_index + 48  // Second layer starts at index 48
                        } else {
                            led_index
                        };
                        
                        let ticker = tickers.get(ticker_index as usize)
                            .unwrap_or(&"UNKNOWN");
                        
                        println!("Key pressed: LED {} (Layer {}) -> Ticker: {}", 
                                 led_index, layer, ticker);
                        
                        // Update focus and send LED command
                        if led_index < 48 {
                            current_focus = Some(led_index);
                            
                            // Send focus command to highlight this LED
                            let mut cmd_buf = [0u8; 32];
                            cmd_buf[0] = CMD_FOCUS;
                            cmd_buf[1] = led_index;
                            
                            dev.write(&cmd_buf)?;
                            println!("  -> Focused on LED {} ({})\n", led_index, ticker);
                        }
                    },
                    NOTIFY_LAYER => {
                        // Layer change notification: [0xFD, new_layer]
                        current_layer = read_buf[1];
                        println!("Layer changed to: {}\n", current_layer);
                    },
                    NOTIFY_ACK => {
                        // Command acknowledgment - can ignore
                    },
                    _ => {
                        // Unknown notification
                    }
                }
            },
            Err(_) => {
                // Timeout is normal, just continue
            }
        }
        
        // Small delay to prevent CPU spinning
        std::thread::sleep(Duration::from_millis(1));
    }
}