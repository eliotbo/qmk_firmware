#[cfg(test)]
mod tests {
    use loop_pad_test::{Config, Pad, PadEvent, Rgb};
    use std::time::Duration;

    #[tokio::test]
    #[cfg(feature = "mock")]
    async fn test_mock_pad_events() {
        let config = Config::default();
        let mut pad = Pad::new(&config).expect("Failed to create pad");
        let events = pad.spawn(&config).expect("Failed to spawn pad");

        // Wait for initial events
        tokio::time::sleep(Duration::from_millis(200)).await;

        // Should receive hello event
        let event = events.try_recv();
        assert!(event.is_ok());
        if let Ok(PadEvent::Hello { proto, .. }) = event {
            assert_eq!(proto, 1);
        }

        // Should receive layer event
        tokio::time::sleep(Duration::from_millis(100)).await;
        let event = events.try_recv();
        assert!(event.is_ok());
    }

    #[test]
    fn test_rgb_constants() {
        assert_eq!(Rgb::WHITE, Rgb { r: 255, g: 255, b: 255 });
        assert_eq!(Rgb::RED, Rgb { r: 255, g: 0, b: 0 });
        assert_eq!(Rgb::GREEN, Rgb { r: 0, g: 255, b: 0 });
        assert_eq!(Rgb::BLUE, Rgb { r: 0, g: 0, b: 255 });
        assert_eq!(Rgb::OFF, Rgb { r: 0, g: 0, b: 0 });
    }

    #[test]
    fn test_config_loading() {
        let config = Config::default();
        assert_eq!(config.hid.vid, 0x574C);
        assert_eq!(config.hid.pid, 0x1DF9);
        assert_eq!(config.demo.start_layer, 0);
        assert_eq!(config.demo.arm_timeout_seconds, 30);
    }

    #[test]
    fn test_event_parsing_comprehensive() {
        use loop_pad_test::event::parse_hid_event;

        // Test all event types
        let test_cases = vec![
            (
                vec![0x10, 0, 5, 1],
                Some(PadEvent::Button {
                    layer: 0,
                    idx: 5,
                    pressed: true,
                }),
            ),
            (
                vec![0x11, 1, 0xFF],
                Some(PadEvent::EncRotate {
                    idx: 1,
                    delta: -1,
                }),
            ),
            (
                vec![0x12, 2, 2],
                Some(PadEvent::EncPress {
                    idx: 2,
                    long: true,
                }),
            ),
            (
                vec![0x13, 1],
                Some(PadEvent::Layer { layer: 1 }),
            ),
            (
                vec![0x7E, 1, 2, 3],
                Some(PadEvent::Hello {
                    proto: 1,
                    fw_major: 2,
                    fw_minor: 3,
                }),
            ),
        ];

        for (data, expected) in test_cases {
            let result = parse_hid_event(&data);
            assert_eq!(result, expected, "Failed for data: {:?}", data);
        }
    }

    #[test]
    fn test_boundary_conditions() {
        use loop_pad_test::event::parse_hid_event;

        // Test boundary values
        assert_eq!(
            parse_hid_event(&[0x10, 2, 8, 0]),
            Some(PadEvent::Button {
                layer: 2,
                idx: 8,
                pressed: false,
            })
        );

        assert_eq!(
            parse_hid_event(&[0x11, 2, 127]),
            Some(PadEvent::EncRotate {
                idx: 2,
                delta: 127,
            })
        );

        assert_eq!(
            parse_hid_event(&[0x11, 0, 128]),
            Some(PadEvent::EncRotate {
                idx: 0,
                delta: -128,
            })
        );
    }

    #[cfg(feature = "midi")]
    #[test]
    fn test_midi_cc_interpretation() {
        use loop_pad_test::midi::{cc, interpret_midi_cc};

        assert!(interpret_midi_cc(cc::SHARES_UP, 127).is_some());
        assert!(interpret_midi_cc(cc::SHARES_DOWN, 127).is_some());
        assert!(interpret_midi_cc(cc::STOP_UP, 127).is_some());
        assert!(interpret_midi_cc(cc::STOP_DOWN, 127).is_some());
        assert!(interpret_midi_cc(cc::LIMIT_UP, 127).is_some());
        assert!(interpret_midi_cc(cc::LIMIT_DOWN, 127).is_some());
        
        // Should ignore CC off
        assert!(interpret_midi_cc(cc::SHARES_UP, 0).is_none());
        
        // Unknown CC
        assert!(interpret_midi_cc(99, 127).is_none());
    }
}