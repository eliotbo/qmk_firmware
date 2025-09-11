# IBKR Trading Keymap Build Options

# Optional: keep MIDI but disable by default (can be overridden)
MIDI_ENABLE ?= no

# Enable Raw HID for external LED control
RAW_ENABLE = yes

# Disable encoder map since we're handling encoders manually
ENCODER_MAP_ENABLE = no

# Enable RGB Matrix for layer indication
RGB_MATRIX_ENABLE = yes

# Disable unused features to save space
MOUSEKEY_ENABLE = no
EXTRAKEY_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
AUDIO_ENABLE = no