# CHANGELOG - RAW HID Migration

## Version 1.0.0 - RAW HID Protocol Implementation

### Overview
Migrated from F-key based communication to custom RAW HID protocol for the Work Louder Loop IBKR trading keymap. This change eliminates OS-level key event leaks and provides more reliable, vendor-specific communication between the keyboard and host applications.

### Breaking Changes
- **F-key emissions removed:** The keyboard no longer sends F-key combinations (Shift+F9-F35)
- **Host application required:** A HID-aware host application is now required to interpret button presses
- **MIDI disabled by default:** MIDI support is now optional (can be enabled with `MIDI_ENABLE=yes`)

### New Features
- **RAW HID Protocol v1:** Clean vendor-specific HID communication
- **Button events:** Press/release events with layer context
- **Encoder events:** Rotation and press events (short/long press detection)
- **Layer notifications:** Real-time layer change events
- **Handshake support:** Host ready acknowledgment
- **Boot announcement:** Device sends hello message on startup

### Migration Guide

#### For Users
1. **Update host application:** Ensure your IBKR integration software supports RAW HID protocol v1
2. **Flash new firmware:** Compile and flash the updated keymap
3. **Verify connection:** Look for the hello message (0x7E) on device boot

#### For Developers

##### Enabling Legacy F-key Mode (Not Recommended)
If you need temporary F-key compatibility during migration:

1. Edit `config.h`:
```c
#define LEGACY_FKEY_COMPAT 1  // Enable F-key fallback
```

2. Recompile firmware:
```bash
qmk compile -kb work_louder/loop -km ibkr
```

**Warning:** Legacy mode reintroduces focus leak risks. Use only during transition.

##### Re-enabling MIDI
To compile with MIDI support:
```bash
qmk compile -kb work_louder/loop -km ibkr -e MIDI_ENABLE=yes
```

### Protocol Benefits
- **No OS interference:** Events bypass OS keyboard handling
- **Layer awareness:** Host receives layer context with each event
- **Bidirectional:** Full duplex communication for LED control
- **Extensible:** Protocol versioning for future enhancements
- **Reliable:** No risk of keystrokes appearing in wrong applications

### Files Changed
- `keymap.c` - Complete HID implementation
- `keymap.c.bak` - Backup of original F-key version
- `rules.mk` - MIDI now optional, RAW_ENABLE confirmed
- `config.h` - Added HID configuration parameters
- `docs/RAW_HID_PROTOCOL.md` - Protocol specification
- `docs/CHANGELOG_HID.md` - This file

### Rollback Instructions
To revert to F-key version:
1. Restore from backup: `cp keymap.c.bak keymap.c`
2. Re-enable MIDI: Edit `rules.mk`, set `MIDI_ENABLE = yes`
3. Recompile and flash

### Known Issues
- Host application must send `CMD_HOST_READY` for full functionality
- LED commands only affect BASE layer (by design)

### Future Enhancements
- Event coalescing for rapid encoder rotation
- Heartbeat messages for connection monitoring
- LED theme presets (paper trading vs live)
- Extended encoder acceleration curves

### Support
For issues or questions about the HID migration, refer to:
- Protocol specification: `docs/RAW_HID_PROTOCOL.md`
- QMK RAW HID documentation: https://docs.qmk.fm/#/feature_rawhid

---
*Last updated: 2025*