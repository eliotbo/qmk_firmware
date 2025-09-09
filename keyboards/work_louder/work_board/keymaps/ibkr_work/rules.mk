# IBKR Work Trading Keymap Build Options

# Enable features
RAW_ENABLE = yes           # Enable Raw HID for LED control
ENCODER_ENABLE = yes       # Enable rotary encoder
ENCODER_MAP_ENABLE = no    # We handle encoder manually

# Disable unused features to save space
MOUSEKEY_ENABLE = no       # Mouse keys
EXTRAKEY_ENABLE = yes      # Audio control and System control
CONSOLE_ENABLE = no        # Console for debug
COMMAND_ENABLE = no        # Commands for debug and configuration
NKRO_ENABLE = yes         # Enable N-Key Rollover
BACKLIGHT_ENABLE = no     # Enable keyboard backlight functionality
AUDIO_ENABLE = no         # Audio output
MIDI_ENABLE = no          # MIDI support (not needed, using HID)
UNICODE_ENABLE = no       # Unicode
BLUETOOTH_ENABLE = no     # Enable Bluetooth
SWAP_HANDS_ENABLE = no    # Enable one-hand typing
TAP_DANCE_ENABLE = no     # Tap Dance

# Optimization
LTO_ENABLE = yes          # Link Time Optimization to reduce size
SPACE_CADET_ENABLE = no   # Save space

# RGB configuration is handled by the parent keyboard