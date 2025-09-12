// Test file to demonstrate the thread safety issue
use std::sync::Arc;
use std::thread;

// Simulating the HidDevice structure that doesn't implement Sync
struct HidDevice {
    data: *const u8, // Raw pointer makes it !Sync
}

// SAFETY: We're just simulating the issue, not actually using the pointer
unsafe impl Send for HidDevice {}
// Note: HidDevice is NOT Sync

struct HidPad {
    device: HidDevice,
}

fn main() {
    let pad = Arc::new(HidPad {
        device: HidDevice {
            data: std::ptr::null(),
        },
    });

    // This will fail to compile because HidDevice is not Sync
    thread::spawn(move || {
        let _pad = pad;
        println!("In thread");
    });
}