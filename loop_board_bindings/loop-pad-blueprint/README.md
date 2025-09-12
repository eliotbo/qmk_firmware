# Loop Pad Blueprint (egui)

Simple egui desktop app scaffold to interface with the Loop Pad (9 keys + 3 encoders) over RAW HID, aligned with the provided blueprint.

## Run

- UI only (mock input):
  - Enable mock via feature/CLI/env and run: `cargo run --features mock` or `LOOPPAD_MOCK=1 cargo run` or `cargo run -- --mock`
- Plain UI (no input):
  - `cargo run`
- With HID support (requires `hidapi`):
  - Linux: install `libudev-dev` and `libhidapi-dev` (or distro equivalents)
  - macOS: `brew install hidapi`
  - Windows: no extra setup typically required
  - Run: `cargo run --features hid`

## Features implemented (steps 1–5)

- eframe/egui app skeleton with logging
- Protocol constants and `PadEvent` parser
- HID open/read thread and write API (0x7D, 0x01/0x02, 0x03)
- AppState with encoder modes/values and per-layer labels
- UI panels: controls (ARM + layer buttons), encoder text boxes, keyboard 3×3 grid with status circles

Config overrides
- Env vars: `LOOPPAD_VID`, `LOOPPAD_PID`, `LOOPPAD_USAGE_PAGE`, `LOOPPAD_USAGE`, `LOOPPAD_MOCK=1`.
- Optional file `padtest.toml` with keys: `vid`, `pid`, `usage_page`, `usage`, `mock_enabled`.

Features implemented (steps 6–10)
- Event wiring for encoder rotate/press; value adjust + mode cycling
- 200ms blue highlights for keys/encoders
- BASE-only LED sync with throttle/dedupe; pressed→blue, armed→white, else off
- Reconnection: Hello→HostReady+Layer set + full LED pass; device Layer events update UI
- Config + mock generator; `--features mock` or `LOOPPAD_MOCK=1` runs without device
