**Goal:** Create a **Rust test crate** that lets us quickly iterate on the Loop Pad keyboard interface:

* **Read**: both **RAW HID v1** events (from our custom firmware) **and** **MIDI** (for legacy/alternate encoders or other boards).
* **Send**: **LED commands** and **Layer-change** commands to the Loop Pad via RAW HID.
* Provide a clean, typed event API (`PadEvent`) the rest of our app can consume.
* Include a small CLI demo (`pad-tester`) to visualize events and issue LED/layer commands from the terminal.

> **Do not delete anything without a backup.** If you must change/remove files, first duplicate them to `*.bak`.

---

## Raw Requirements

### Hardware & Protocol Context

* Board: **Loop Pad from Work Louder**, 9 keys + 3 encoders, 3 layers.
* **Encoders and keys report via RAW HID**. We may still **listen to MIDI** for other devices or transitional paths.
* **ARMing** is managed by the **host application (this crate won’t arm the device)**.

### RAW HID Protocol v1 (Host ↔ Device)

**Buffer:** 32 bytes fixed. Byte 0 = message type. Bytes 1..31 = payload, zero-padded.

**Device → Host Events**

* **Button (0x10)**: `[0x10, layer(0-2), btn_idx(0-8), act(1=press/0=release)]`
* **Encoder Rotation (0x11)**: `[0x11, enc_idx(0-2), delta(int8 +1/-1)]`
* **Encoder Press (0x12)**: `[0x12, enc_idx(0-2), kind(1=short/2=long)]`
* **Layer Change (0x13)**: `[0x13, layer(0-2)]`
* **Boot Hello (0x7E)**: `[0x7E, proto_ver=1, fw_major, fw_minor]`

**Host → Device Commands**

* **Set All LEDs (0x01)**: `[0x01, r, g, b]` *(BASE layer only)*
* **Set One LED (0x02)**: `[0x02, led_idx(0-8), r, g, b]` *(BASE layer only)*
* **Set Layer (0x03)**: `[0x03, layer(0-2)]`
* **Host Ready (0x7D)**: `[0x7D, proto_ver=1]` *(handshake)*

**Per-Layer LED Themes** (device behavior)

* Layer 0 (BASE): host-controllable via 0x01/0x02.
* Layer 1 (BUY): forced green.
* Layer 2 (SELL): forced red.

**Timing**

* Short press < 500ms; Long press ≥ 500ms
* Device debounce \~5ms, recommend host debounce 10ms

**Encoder Press Semantics**

* Enc0: short = cycle \[Shares → %BP → Risk\$], long = reset default
* Enc1: short = cycle \[Ticks → % → ATR], long = toggle Trailing/Fixed
* Enc2: short = cycle \[Bid → Mid → Ask], long = toggle Non-Crossing

---

## Deliverables

1. **Crate:** `loop-pad-test`
2. **Binary:** `pad-tester` — terminal interactive tool to:

   * print HID and MIDI events live
   * send example LED commands (set-all, set-one)
   * send layer-change command
   * perform handshake
   * optional: simple LED pulses/animations to verify timing
3. **Library API:** A `Pad` struct with:

   * `spawn()` to start background IO tasks
   * a `Receiver<PadEvent>` stream for incoming events
   * methods: `set_all_leds(RGB)`, `set_led(idx, RGB)`, `set_layer(u8)`, `send_ready()`
4. **Config:** `padtest.toml` for VID/PID/usage and MIDI port names + feature flags.
5. **Feature flags:** `hid`, `midi`, `mock` (default: `hid` on, `midi` off). `mock` supplies a simulated pad for CI.
6. **Docs:** README with run instructions + troubleshooting; PROTOCOL.md (host side); CHANGELOG.md.

---

##

---

## Cargo.toml (requirements)

```toml
[package]
name = "loop-pad-test"
version = "0.1.0"
edition = "2021"

[features]
hid = []
midi = []
mock = []

env-logger = []

[dependencies]
anyhow = "1"
thiserror = "1"
log = "0.4"
envy = "0.4"
serde = { version = "1", features = ["derive"] }
serde_json = "1"
toml = "0.8"
crossbeam-channel = "0.5"

tokio = { version = "1.41", features = ["rt-multi-thread", "time", "macros"] }

# HID and MIDI (gated)
#[cfg(feature = "hid")]
hidapi = { version = "2", default-features = false }
#[cfg(feature = "midi")]
midir = "0.9"

# (Optional) colored CLI
clap = { version = "4", features = ["derive"] }
```

> If the package manager complains about cfg flags, you can move `hidapi` and `midir` under normal deps and gate usage in code with `#[cfg(feature = "...")]`.

---

## Types & Events

```rust
#[derive(Copy, Clone, Debug)]
pub struct Rgb { pub r: u8, pub g: u8, pub b: u8 }

#[derive(Clone, Debug)]
pub enum PadEvent {
    Hello { proto: u8, fw_major: u8, fw_minor: u8 },
    Layer { layer: u8 },
    Button { layer: u8, idx: u8, pressed: bool },
    EncRotate { idx: u8, delta: i8 },
    EncPress { idx: u8, long: bool },
    // For MIDI input (optional)
    MidiCc { channel: u8, controller: u8, value: u8 },
}
```

**Parsing:** write a small `event.rs` that maps the RAW HID bytes to `PadEvent` based on IDs.

---

## HID I/O (hid.rs)

Responsibilities:

* Open the device by **VID/PID** and optional usage page; non-blocking reads.
* **Windows quirk:** writing may require a leading `0x00` report ID — handle automatically.
* Background tasks: one task reads HID packets → parses → sends to `crossbeam_channel::Sender<PadEvent>`.
* Public methods for commands: `send_ready()`, `set_all_leds(rgb)`, `set_led(idx, rgb)`, `set_layer(layer)`.

Sketch:

```rust
pub struct HidPad {
    dev: hidapi::HidDevice,
}

impl HidPad {
    pub fn open(vid: u16, pid: u16) -> anyhow::Result<Self> { /* ... */ }

    pub fn read_loop(self: std::sync::Arc<Self>, tx: crossbeam_channel::Sender<PadEvent>) {
        tokio::spawn(async move {
            let mut buf = [0u8; 32];
            loop {
                match self.dev.read_timeout(&mut buf, 10) {
                    Ok(n) if n > 0 => {
                        if let Some(evt) = crate::event::parse_hid_event(&buf[..n]) {
                            let _ = tx.send(evt);
                        }
                    }
                    _ => {}
                }
            }
        });
    }

    fn write_packet(&self, id: u8, payload: &[u8]) -> anyhow::Result<()> {
        let mut pkt = [0u8; 32];
        pkt[0] = id;
        let n = payload.len().min(31);
        pkt[1..1+n].copy_from_slice(&payload[..n]);
        #[cfg(target_os = "windows")]
        {
            // Prepend report ID 0x00 for HIDAPI on Windows
            let mut wpkt = [0u8; 33];
            wpkt[0] = 0x00;
            wpkt[1..33].copy_from_slice(&pkt);
            self.dev.write(&wpkt)?;
            return Ok(());
        }
        #[cfg(not(target_os = "windows"))]
        {
            self.dev.write(&pkt)?;
            return Ok(());
        }
    }

    pub fn send_ready(&self) -> anyhow::Result<()> { self.write_packet(0x7D, &[1]) }
    pub fn set_all_leds(&self, rgb: Rgb) -> anyhow::Result<()> { self.write_packet(0x01, &[rgb.r, rgb.g, rgb.b]) }
    pub fn set_led(&self, idx: u8, rgb: Rgb) -> anyhow::Result<()> { self.write_packet(0x02, &[idx, rgb.r, rgb.g, rgb.b]) }
    pub fn set_layer(&self, layer: u8) -> anyhow::Result<()> { self.write_packet(0x03, &[layer]) }
}
```

---

## MIDI I/O (midi.rs) — optional

Responsibilities:

* Open an input port by name (from `padtest.toml`).
* Map CC events to `PadEvent::MidiCc { channel, controller, value }`.
* Run in its own background task, push into the same channel as HID.

Sketch:

```rust
pub fn spawn_midi_reader(port_name_substr: &str, tx: Sender<PadEvent>) -> anyhow::Result<()> {
    let midi_in = midir::MidiInput::new("pad-midi")?;
    let in_port = midi_in.ports().into_iter()
        .find(|p| midi_in.port_name(p).ok().map_or(false, |n| n.contains(port_name_substr)))
        .ok_or_else(|| anyhow::anyhow!("MIDI port not found"))?;
    midi_in.connect(in_port, "pad-in", move |_, msg, tx| {
        if msg.len() == 3 && (msg[0] & 0xF0) == 0xB0 { // CC
            let ch = msg[0] & 0x0F;
            let cc = msg[1];
            let val = msg[2];
            let _ = tx.send(PadEvent::MidiCc { channel: ch, controller: cc, value: val });
        }
    }, tx)?;
    Ok(())
}
```

---

## Config (config.rs + padtest.toml)

`padtest.toml` example:

```toml
[hid]
vid = 0x574C   # Work Louder (example)
pid = 0x1DF9   # Loop (example)
usage_page = 0xFF60
usage = 0x61

[midi]
port_hint = "Loop"   # substring to match input port name

[demo]
start_layer = 0
start_color = { r = 255, g = 255, b = 255 }
```

---

## CLI Demo (pad-tester.rs)

Capabilities:

* On start: open HID, send `Host Ready (0x7D)`, set initial layer/color from config.
* Print any incoming HID or MIDI events.
* Simple REPL: commands like `all r g b`, `led idx r g b`, `layer n`, `pulse`, `help`, `quit`.

Sketch:

```rust
#[tokio::main]
async fn main() -> anyhow::Result<()> {
    env_logger::init();
    let cfg = Config::load()?;
    let (tx, rx) = crossbeam_channel::unbounded();

    let hid = HidPad::open(cfg.hid.vid, cfg.hid.pid)?;
    let hid = std::sync::Arc::new(hid);
    hid.clone().read_loop(tx.clone());

    hid.send_ready()?;
    hid.set_layer(cfg.demo.start_layer)?;
    let c = cfg.demo.start_color; hid.set_all_leds(Rgb{r:c.r,g:c.g,b:c.b})?;

    #[cfg(feature = "midi")]
    if let Some(hint) = cfg.midi.port_hint.as_deref() { let _ = spawn_midi_reader(hint, tx.clone()); }

    // Event printer task
    let printer = tokio::spawn(async move {
        loop { if let Ok(evt) = rx.recv() { println!("{:?}", evt); } }
    });

    // Simple stdin REPL omitted for brevity; parse and call hid methods
    printer.await?;
    Ok(())
}
```

---

## Tests

* **Unit**: parsing from raw HID buffers → `PadEvent` (golden vectors for each message type).
* **Mock feature**: fake HID backend that produces canned events and captures writes; use it in CI.
* **Round-trip**: send LED commands, assert they’re packetized correctly (bytes).
* **Throughput**: feed 10k encoder events, assert no panics and bounded latency.

---

## Troubleshooting & OS Notes

* **Linux**: Add a `udev` rule to allow access without `sudo` (should already be done, so no need)

* If HID reads return 0 frequently, switch to `read_timeout` with 10–20ms.

---

## Acceptance Criteria

* `cargo run --bin pad-tester` prints HID `Hello`, `Layer`, `Button`, `EncRotate`, `EncPress` when the device is used.
* REPL commands successfully change LEDs and layer.
* Optional MIDI input logs CC events when feature is enabled.
* No OS keystrokes leak when pressing the pad (verify in a text editor).
* Code is formatted, clippy-clean, and documented.

---

## Improvement Note (future-proofing)

* Add **protocol version negotiation** with graceful fallback (reject mismatched proto).
* Implement **encoder coalescing** (aggregate rapid deltas for ≤5ms for lower overhead).
* Add **heartbeats** and **reconnect** logic (device unplug ↔ replug).
* Provide an **egui test UI** (mini HUD) to mirror LEDs and states visually.
* Optional **CRC8/CRC16** per packet if we ever see line noise.

> Keep the crate focused and composable. The goal is to quickly iterate on the firmware/host interaction with a small, typed surface area, not to rebuild the trading app here.
