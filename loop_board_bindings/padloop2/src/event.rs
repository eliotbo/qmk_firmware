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
    Hello {
        proto: u8,
        fw_major: u8,
        fw_minor: u8,
    },
    Layer {
        layer: u8,
    },
    Button {
        layer: u8,
        idx: u8,
        pressed: bool,
    },
    EncRotate {
        idx: u8,
        delta: i8,
    },
    EncPress {
        idx: u8,
        long: bool,
    },
}

/// Parse a raw HID packet into a PadEvent
pub fn parse_hid_event(data: &[u8]) -> Option<PadEvent> {
    if data.is_empty() {
        return None;
    }

    let event = match data[0] {
        0x10 if data.len() >= 4 => {
            // Button event: [0x10, layer, btn_idx, act]
            Some(PadEvent::Button {
                layer: data[1],
                idx: data[2],
                pressed: data[3] == 1,
            })
        }
        0x11 if data.len() >= 3 => {
            // Encoder rotation: [0x11, enc_idx, delta]
            Some(PadEvent::EncRotate {
                idx: data[1],
                delta: data[2] as i8,
            })
        }
        0x12 if data.len() >= 3 => {
            // Encoder press: [0x12, enc_idx, kind]
            Some(PadEvent::EncPress {
                idx: data[1],
                long: data[2] == 2,
            })
        }
        0x13 if data.len() >= 2 => {
            // Layer change: [0x13, layer]
            Some(PadEvent::Layer { layer: data[1] })
        }
        0x7E if data.len() >= 4 => {
            // Boot hello: [0x7E, proto_ver, fw_major, fw_minor]
            Some(PadEvent::Hello {
                proto: data[1],
                fw_major: data[2],
                fw_minor: data[3],
            })
        }
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_button_press() {
        let data = [0x10, 0, 3, 1];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::Button {
                layer: 0,
                idx: 3,
                pressed: true,
            }
        );
    }

    #[test]
    fn test_parse_button_release() {
        let data = [0x10, 1, 5, 0];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::Button {
                layer: 1,
                idx: 5,
                pressed: false,
            }
        );
    }

    #[test]
    fn test_parse_encoder_rotate() {
        let data = [0x11, 2, 0xFF]; // -1 as u8
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::EncRotate {
                idx: 2,
                delta: -1,
            }
        );
    }

    #[test]
    fn test_parse_encoder_press_short() {
        let data = [0x12, 0, 1];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::EncPress {
                idx: 0,
                long: false,
            }
        );
    }

    #[test]
    fn test_parse_encoder_press_long() {
        let data = [0x12, 1, 2];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::EncPress {
                idx: 1,
                long: true,
            }
        );
    }

    #[test]
    fn test_parse_layer_change() {
        let data = [0x13, 2];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(event, PadEvent::Layer { layer: 2 });
    }

    #[test]
    fn test_parse_hello() {
        let data = [0x7E, 1, 2, 3];
        let event = parse_hid_event(&data).unwrap();
        assert_eq!(
            event,
            PadEvent::Hello {
                proto: 1,
                fw_major: 2,
                fw_minor: 3,
            }
        );
    }

    #[test]
    fn test_parse_unknown_event() {
        let data = [0xFF, 1, 2, 3];
        assert_eq!(parse_hid_event(&data), None);
    }

    #[test]
    fn test_parse_empty_data() {
        let data = [];
        assert_eq!(parse_hid_event(&data), None);
    }

    #[test]
    fn test_parse_insufficient_data() {
        let data = [0x10, 0]; // Button event needs 4 bytes
        assert_eq!(parse_hid_event(&data), None);
    }
}