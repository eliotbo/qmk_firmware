# Changelog

All notable changes to the Loop Pad Test crate will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-09-11

### Added

- Initial release of Loop Pad Test crate
- RAW HID Protocol v1 implementation
  - Bidirectional communication with Loop Pad keyboard
  - Button press/release events with layer context
  - Encoder rotation and press events (short/long detection)
  - Layer change notifications
  - Boot hello handshake
  - LED control commands (individual and all LEDs)
  - Layer switching commands
  - Host ready acknowledgment

- MIDI input support (optional feature)
  - CC event capture for encoder compatibility
  - Configurable port selection

- Mock mode for testing without hardware
  - Simulated device events
  - CI/CD friendly testing

- `pad-tester` CLI tool
  - Listen mode for event monitoring
  - REPL mode with interactive commands
  - Demo mode with LED animations
  - Configuration file support

- Cross-platform support
  - Automatic Windows HID report ID handling
  - Linux udev rule documentation
  - macOS compatibility

- Comprehensive documentation
  - README with quick start guide
  - PROTOCOL.md with full specification
  - Inline code documentation

- Test coverage
  - Unit tests for event parsing
  - Mock backend for integration testing
  - Example code in documentation

### Known Issues

- MIDI feature requires manual port configuration
- No automatic reconnection on device disconnect
- LED commands only work on BASE layer (by design)

### Future Improvements

- Protocol version negotiation
- Encoder event coalescing for high-speed rotation
- Heartbeat/keepalive mechanism
- Automatic device reconnection
- GUI test application with egui