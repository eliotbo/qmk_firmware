use anyhow::Result;
use crossbeam_channel::{bounded, Receiver, Sender};
use log::{debug, error, info};
use std::sync::Arc;
use std::thread;
use std::time::Duration;

use super::protocol::{parse_hid_event, PadEvent, Rgb};

pub const HID_BUFFER_SIZE: usize = 32;

pub struct HidPad {
    #[allow(dead_code)]
    cmd_tx: Sender<HidCommand>,
    cmd_rx: Option<Receiver<HidCommand>>,
    #[cfg(feature = "hid")]
    device: Option<hidapi::HidDevice>,
}

enum HidCommand {
    Write { id: u8, payload: Vec<u8> },
    Shutdown,
}

impl HidPad {
    #[cfg(feature = "hid")]
    pub fn open(
        vid: u16,
        pid: u16,
        usage_page: Option<u16>,
        usage: Option<u16>,
    ) -> Result<Self> {
        let api = hidapi::HidApi::new()?;
        let devices: Vec<_> = api
            .device_list()
            .filter(|info| {
                info.vendor_id() == vid
                    && info.product_id() == pid
                    && (usage_page.is_none() || info.usage_page() == usage_page.unwrap())
                    && (usage.is_none() || info.usage() == usage.unwrap())
            })
            .collect();

        if devices.is_empty() {
            return Err(anyhow::anyhow!(
                "No HID device found with VID={:04X}, PID={:04X}",
                vid, pid
            ));
        }

        let device = devices[0].open_device(&api)?;
        device.set_blocking_mode(false)?;
        info!("Opened HID device: VID={:04X}, PID={:04X}", vid, pid);

        let (cmd_tx, cmd_rx) = bounded(100);
        Ok(Self {
            cmd_tx,
            cmd_rx: Some(cmd_rx),
            device: Some(device),
        })
    }

    #[cfg(not(feature = "hid"))]
    pub fn open(
        _vid: u16,
        _pid: u16,
        _usage_page: Option<u16>,
        _usage: Option<u16>,
    ) -> Result<Self> {
        let (cmd_tx, cmd_rx) = bounded(100);
        Ok(Self {
            cmd_tx,
            cmd_rx: Some(cmd_rx),
            #[cfg(feature = "hid")]
            device: None,
        })
    }

    pub fn read_loop(mut self, tx: Sender<PadEvent>) -> Arc<HidPad> {
        #[cfg(feature = "hid")]
        let device = self.device.take();
        let cmd_rx = self.cmd_rx.take().expect("cmd_rx already taken");

        thread::spawn(move || {
            info!("Starting HID read loop");
            let mut buf = [0u8; HID_BUFFER_SIZE];

            loop {
                if let Ok(cmd) = cmd_rx.try_recv() {
                    match cmd {
                        HidCommand::Write { id, payload } => {
                            #[cfg(feature = "hid")]
                            if let Some(ref device) = device {
                                if let Err(e) = write_packet_internal(device, id, &payload) {
                                    error!("Failed to write HID packet: {}", e);
                                }
                            }
                            #[cfg(not(feature = "hid"))]
                            {
                                debug!("[MOCK] Would send HID packet id=0x{:02X}", id);
                            }
                        }
                        HidCommand::Shutdown => {
                            info!("Shutting down HID thread");
                            break;
                        }
                    }
                }

                #[cfg(feature = "hid")]
                if let Some(ref device) = device {
                    match device.read_timeout(&mut buf, 10) {
                        Ok(n) if n > 0 => {
                            debug!("Read {} bytes from HID: {:02X?}", n, &buf[..n.min(8)]);
                            if let Some(evt) = parse_hid_event(&buf[..n]) {
                                debug!("Parsed event: {:?}", evt);
                                if let Err(e) = tx.send(evt) {
                                    error!("Failed to send event: {}", e);
                                    break;
                                }
                            } else {
                                debug!("Could not parse HID event from {} bytes", n);
                            }
                        }
                        Ok(_) => {}
                        Err(e) => {
                            error!("HID read error: {}", e);
                            thread::sleep(Duration::from_millis(100));
                        }
                    }
                }

                thread::sleep(Duration::from_millis(1));
            }
        });

        Arc::new(HidPad {
            cmd_tx: self.cmd_tx,
            cmd_rx: None,
            #[cfg(feature = "hid")]
            device: None,
        })
    }

    pub fn send_ready(&self) -> Result<()> {
        self.write_packet(0x7D, &[1])
    }

    pub fn set_all_leds(&self, rgb: Rgb) -> Result<()> {
        self.write_packet(0x01, &[rgb.r, rgb.g, rgb.b])
    }

    pub fn set_led(&self, idx: u8, rgb: Rgb) -> Result<()> {
        self.write_packet(0x02, &[idx, rgb.r, rgb.g, rgb.b])
    }

    pub fn set_layer(&self, layer: u8) -> Result<()> {
        self.write_packet(0x03, &[layer])
    }

    fn write_packet(&self, id: u8, payload: &[u8]) -> Result<()> {
        self.cmd_tx
            .send(HidCommand::Write {
                id,
                payload: payload.to_vec(),
            })
            .map_err(|e| anyhow::anyhow!("Failed to send HID command: {}", e))
    }
}

#[cfg(feature = "hid")]
fn write_packet_internal(device: &hidapi::HidDevice, id: u8, payload: &[u8]) -> Result<()> {
    let mut pkt = [0u8; HID_BUFFER_SIZE];
    pkt[0] = id;
    let n = payload.len().min(HID_BUFFER_SIZE - 1);
    pkt[1..1 + n].copy_from_slice(&payload[..n]);

    #[cfg(target_os = "windows")]
    {
        let mut wpkt = [0u8; HID_BUFFER_SIZE + 1];
        wpkt[0] = 0x00;
        wpkt[1..=HID_BUFFER_SIZE].copy_from_slice(&pkt);
        device.write(&wpkt)?;
        debug!("Sent HID packet (Windows): {:02X?}", &wpkt[..5]);
    }

    #[cfg(not(target_os = "windows"))]
    {
        device.write(&pkt)?;
        debug!("Sent HID packet: {:02X?}", &pkt[..5]);
    }

    Ok(())
}

