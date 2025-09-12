use log::debug;

#[derive(Copy, Clone, Debug, PartialEq)]
pub struct Rgb {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

impl Rgb {
    pub const WHITE: Rgb = Rgb { r: 255, g: 255, b: 255 };
    pub const RED: Rgb = Rgb { r: 255, g: 0, b: 0 };
    pub const GREEN: Rgb = Rgb { r: 0, g: 255, b: 0 };
    pub const BLUE: Rgb = Rgb { r: 0, g: 0, b: 255 };
    pub const OFF: Rgb = Rgb { r: 0, g: 0, b: 0 };
}

#[derive(Clone, Debug, PartialEq)]
pub enum PadEvent {
    Hello { proto: u8, fw_major: u8, fw_minor: u8 },
    Layer { layer: u8 },
    Button { layer: u8, idx: u8, pressed: bool },
    EncRotate { idx: u8, delta: i8 },
    EncPress { idx: u8, long: bool },
    MidiCc { channel: u8, controller: u8, value: u8 },
}

pub fn parse_hid_event(data: &[u8]) -> Option<PadEvent> {
    if data.is_empty() {
        return None;
    }

    let event = match data[0] {
        0x10 if data.len() >= 4 => Some(PadEvent::Button {
            layer: data[1],
            idx: data[2],
            pressed: data[3] == 1,
        }),
        0x11 if data.len() >= 3 => Some(PadEvent::EncRotate {
            idx: data[1],
            delta: data[2] as i8,
        }),
        0x12 if data.len() >= 3 => Some(PadEvent::EncPress {
            idx: data[1],
            long: data[2] == 2,
        }),
        0x13 if data.len() >= 2 => Some(PadEvent::Layer { layer: data[1] }),
        0x7E if data.len() >= 4 => Some(PadEvent::Hello {
            proto: data[1],
            fw_major: data[2],
            fw_minor: data[3],
        }),
        _ => {
            debug!("Unknown HID event type: 0x{:02X}", data[0]);
            None
        }
    };

    if let Some(ref evt) = event {
        debug!("Parsed HID event: {:?}", evt);
    }

    event
}

