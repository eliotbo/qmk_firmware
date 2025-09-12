use anyhow::Result;
use std::sync::{Arc, Mutex};
use std::io::{self, Write};

fn main() -> Result<()> {
    println!("=== ENCODER POSITION TRACKER (Simulator) ===");
    println!("Commands:");
    println!("  0+/0-  - Increment/decrement encoder 0");
    println!("  1+/1-  - Increment/decrement encoder 1");  
    println!("  2+/2-  - Increment/decrement encoder 2");
    println!("  r      - Reset all encoders to 0");
    println!("  q      - Quit");
    println!("\nInitial positions: Enc0=0, Enc1=0, Enc2=0");
    println!("Enter commands:");

    // Initialize encoder positions
    let encoder_positions = Arc::new(Mutex::new([0i32; 3]));

    // Clone for the input thread
    let positions = encoder_positions.clone();

    // Input handling in main thread
    let stdin = io::stdin();
    let mut input = String::new();
    
    loop {
        print!("> ");
        io::stdout().flush()?;
        
        input.clear();
        stdin.read_line(&mut input)?;
        let cmd = input.trim();
        
        match cmd {
            "0+" => update_encoder(&positions, 0, 1)?,
            "0-" => update_encoder(&positions, 0, -1)?,
            "1+" => update_encoder(&positions, 1, 1)?,
            "1-" => update_encoder(&positions, 1, -1)?,
            "2+" => update_encoder(&positions, 2, 1)?,
            "2-" => update_encoder(&positions, 2, -1)?,
            "r" => reset_encoders(&positions)?,
            "q" => {
                println!("Exiting...");
                break;
            }
            "" => continue,
            _ => println!("Unknown command: '{}'. Type 'q' to quit.", cmd),
        }
    }
    
    Ok(())
}

fn update_encoder(positions: &Arc<Mutex<[i32; 3]>>, idx: usize, delta: i32) -> Result<()> {
    let mut pos = positions.lock().unwrap();
    let old_pos = pos[idx];
    pos[idx] += delta;
    let new_pos = pos[idx];
    
    println!(
        "Encoder {} changed: {} -> {} (delta: {})",
        idx, old_pos, new_pos, delta
    );
    
    println!(
        "Current positions: Enc0={}, Enc1={}, Enc2={}",
        pos[0], pos[1], pos[2]
    );
    
    Ok(())
}

fn reset_encoders(positions: &Arc<Mutex<[i32; 3]>>) -> Result<()> {
    let mut pos = positions.lock().unwrap();
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 0;
    
    println!("All encoders reset to 0");
    println!(
        "Current positions: Enc0={}, Enc1={}, Enc2={}",
        pos[0], pos[1], pos[2]
    );
    
    Ok(())
}