#include QMK_KEYBOARD_H
#include "midi.h"
#include "qmk_midi.h"
#include "raw_hid.h"

extern MidiDevice midi_device;

/* IBKR Trading Keyboard Layout
 * see LOOP_IBKR_FLASHING_GUIDE.md in root
 * ============================
 * 
 * Physical Layout (3x3 grid + 3 encoders):
 * 
 *  [Enc0] [Enc1] [Enc2] [0] [1] [2] [3] [4] [5] [6] [7] [8] [9]
 *  
 *
 * Encoder Functions (via MIDI CC):
 * - Encoder 0: MIDI CC 20/21 (decrement/increment)
 * - Encoder 1: MIDI CC 22/23 (decrease/increase)
 * - Encoder 2: MIDI CC 24/25 (down/up)
 *
* MIDI Channel Assignment:
 * - Channel 1: Base layer
 * - Channel 2: Buy layer  
 * - Channel 3: Sell layer
 *
 * Encoder Press Functions -> Ctrl + Shift + :
 * - Encoder 0: F9/F12/F15 (Base/Buy/Sell layer)
 * - Encoder 1: F10/F13/F16 (Base/Buy/Sell layer)
 * - Encoder 2: F11/F14/F17 (Base/Buy/Sell layer)
 *
 * 

 */

// MIDI CC definitions for encoders
#define CC_SHARES_DOWN  20
#define CC_SHARES_UP    21
#define CC_STOP_DOWN    22
#define CC_STOP_UP      23
#define CC_LIMIT_DOWN   24
#define CC_LIMIT_UP     25

// MIDI CC values
#define MIDI_CC_OFF     0
#define MIDI_CC_ON      127

// Raw HID command definitions
enum hid_commands {
    CMD_SET_ALL  = 1, // [1, r, g, b]
    CMD_SET_ONE  = 2, // [2, led_index, r, g, b]
    CMD_SET_MODE = 3, // [3, mode] (e.g., 0=BASE,1=BUY,2=SELL)
};

// Store individual LED colors when in HID override mode
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

// Store the "all LEDs" color when using SET_ALL
led_color_t hid_all_color = {0, 0, 0};


led_color_t hid_led_colors[9] = {{0}};  // Store colors for 9 LEDs
bool hid_individual_leds = false;  // Track if we're using individual LED control


// Track if we're in HID control mode (only affects BASE layer)
bool hid_rgb_override = false;

enum layers {
    _BASE,   // Layer 0: Base trading functions
    _BUY,    // Layer 1: Buy-specific actions
    _SELL    // Layer 2: Sell-specific actions
};

// Define F25-F35 since QMK doesn't have them by default
// F24 is 0x73, so F25 starts at 0x74
#define KC_F25 0x74
#define KC_F26 0x75
#define KC_F27 0x76
#define KC_F28 0x77
#define KC_F29 0x78
#define KC_F30 0x79
#define KC_F31 0x7A
#define KC_F32 0x7B
#define KC_F33 0x7C
#define KC_F34 0x7D
#define KC_F35 0x7E

enum custom_keycodes {
    // Base layer actions
    BASE_1 = SAFE_RANGE,
    BASE_2,
    BASE_3,
    BASE_4,
    BASE_5,
    BASE_6,
    BASE_7,
    BASE_8,
    BASE_9,

    // Buy layer actions (using Alt+Shift+F13-F20)
    BUY_1,
    BUY_2,
    BUY_3,
    BUY_4,
    BUY_5,
    BUY_6,
    BUY_7,
    BUY_8,
    BUY_9,
    
    // Sell layer actions (using Ctrl+Alt+F13-F20)
    SELL_1,
    SELL_2,
    SELL_3,
    SELL_4,
    SELL_5,
    SELL_6,
    SELL_7,
    SELL_8,
    SELL_9,
    
    // Encoder press actions
    ENC0_PRESS,  // Quick share presets
    ENC1_PRESS,  // Toggle stop type
    ENC2_PRESS   // Toggle order type
};

// Track current layer for RGB indication and MIDI channel
uint8_t current_layer = _BASE;

// Track encoder states for acceleration
uint8_t encoder_value[3] = {64, 64, 64};  // Start at midpoint
bool encoder_shift_mode = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base Layer
     * Common trading functions
     */
    [_BASE] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        BASE_1,     BASE_2,     BASE_3,
        BASE_4,     BASE_5,     BASE_6,
        BASE_7,     BASE_8,     BASE_9
    ),
    
    /* Buy Layer
     * Buy-specific actions
     */
    [_BUY] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        BUY_1,      BUY_2,      BUY_3,
        BUY_4,      BUY_5,      BUY_6,
        BUY_7,      BUY_8,      BUY_9
    ),
    
    /* Sell Layer
     * Sell-specific actions
     */
    [_SELL] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        SELL_1,     SELL_2,     SELL_3,
        SELL_4,     SELL_5,     SELL_6,
        SELL_7,     SELL_8,     SELL_9
    )
};

// Manual encoder handling for MIDI control
bool encoder_update_user(uint8_t index, bool clockwise) {
    // Get the MIDI channel based on current layer (0-indexed internally, 1-indexed for MIDI)
    uint8_t midi_channel = current_layer;  // 0 for BASE, 1 for BUY, 2 for SELL
    
    switch(index) {
        case 0:  // Share quantity encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_SHARES_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_SHARES_DOWN, MIDI_CC_ON);
            }
            break;
            
        case 1:  // Stop loss encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_DOWN, MIDI_CC_ON);
            }
            break;
            
        case 2:  // Limit price encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_DOWN, MIDI_CC_ON);
            }
            break;
    }
    
    return false;  // Don't process further
}

// Raw HID receive handler for external LED control
void raw_hid_receive(uint8_t *data, uint8_t length) {
    switch (data[0]) {
        case CMD_SET_ALL:
            if (length >= 4 && current_layer == _BASE) {
                hid_rgb_override = true;
                hid_individual_leds = false;  // We're using SET_ALL, not individual
                hid_all_color.r = data[1];
                hid_all_color.g = data[2];
                hid_all_color.b = data[3];
                // Reset individual LED colors when using SET_ALL
                for (uint8_t i = 0; i < 9; i++) {
                    hid_led_colors[i] = hid_all_color;
                }
            }
            break;

        case CMD_SET_ONE:
            if (length >= 5 && current_layer == _BASE) {
                hid_rgb_override = true;
                hid_individual_leds = true;
                uint8_t led_index = data[1];
                if (led_index < 9) {
                    hid_led_colors[led_index].r = data[2];
                    hid_led_colors[led_index].g = data[3];
                    hid_led_colors[led_index].b = data[4];
                }
            }
            break;

        case CMD_SET_MODE:
            if (length >= 2) {
                uint8_t mode = data[1];
                if (mode <= 2) {
                    layer_move(mode);
                    // Only clear HID override when leaving BASE layer
                    if (mode != _BASE) {
                        hid_rgb_override = false;
                    }
                }
            }
            break;

    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Check if shift is held for encoder acceleration
    if (record->event.pressed) {
        if (get_mods() & MOD_MASK_SHIFT) {
            encoder_shift_mode = true;
        } else {
            encoder_shift_mode = false;
        }
    }
    
    switch (keycode) {
        // Base layer keys - Shift+F13-F20
        case BASE_1:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F9);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F10);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F11);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F12);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F13);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F14);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_7:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F15);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_8:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F16);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_9:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F17);
                unregister_code(KC_LSFT);
            }
            return false;
            
        // Buy layer keys - Shift+F21 through Shift+F28
        case BUY_1:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F18);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F19);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F20);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F21);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F22);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F23);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_7:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F24);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_8:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F25);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_9:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F26);
                unregister_code(KC_LSFT);
            }
            return false;

        // Sell layer keys - Shift+F29 through Shift+F35, last one already has Shift
        case SELL_1:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F27);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F28);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F29);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F30);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F31);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F32);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_7:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                tap_code(KC_F33);
                unregister_code(KC_LCTL);
            }
            return false;
        case SELL_8:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F34);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_9:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                tap_code(KC_F35);
                unregister_code(KC_LCTL);
            }
            return false;

        // Encoder press actions - Send Ctrl+Shift+F-keys based on layer
        case ENC0_PRESS:  // Cycle through preset share quantities
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                switch(current_layer) {
                    case _BASE:
                        tap_code(KC_F9);
                        break;
                    case _BUY:
                        tap_code(KC_F12);
                        break;
                    case _SELL:
                        tap_code(KC_F15);
                        break;
                }
                unregister_code(KC_LSFT);
                unregister_code(KC_LCTL);
            }
            return false;
            
        case ENC1_PRESS:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                switch(current_layer) {
                    case _BASE:
                        tap_code(KC_F10);
                        break;
                    case _BUY:
                        tap_code(KC_F13);
                        break;
                    case _SELL:
                        tap_code(KC_F16);
                        break;
                }
                unregister_code(KC_LSFT);
                unregister_code(KC_LCTL);
            }
            return false;
            
        case ENC2_PRESS:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                switch(current_layer) {
                    case _BASE:
                        tap_code(KC_F11);
                        break;
                    case _BUY:
                        tap_code(KC_F14);
                        break;
                    case _SELL:
                        tap_code(KC_F17);
                        break;
                }
                unregister_code(KC_LSFT);
                unregister_code(KC_LCTL);
            }
            return false;
    }
    
    return true;
}

// Set RGB colors based on current layer
layer_state_t layer_state_set_user(layer_state_t state) {
    current_layer = get_highest_layer(state);
    
    // Clear HID override when leaving BASE layer
    if (current_layer != _BASE) {
        hid_rgb_override = false;
    }
    
    switch (current_layer) {
        case 0:
            // White for base layer (unless overridden by HID)
            if (!hid_rgb_override) {
                rgb_matrix_set_color_all(RGB_WHITE);
            }
            break;
        case 1:
            // Green for buy layer
            rgb_matrix_set_color_all(RGB_GREEN);
            break;
        case 2:
            // Red for sell layer
            rgb_matrix_set_color_all(RGB_RED);
            break;
    }
    
    return state;
}

// Initialize RGB on startup
void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_set_color_all(RGB_WHITE);
}

void rgb_matrix_indicators_user(void) {
    if (hid_rgb_override) {
        if (hid_individual_leds) {
            for (uint8_t i = 0; i < 9; i++) {
                rgb_matrix_set_color(i, hid_led_colors[i].r, hid_led_colors[i].g, hid_led_colors[i].b);
              }
          } else {
              rgb_matrix_set_color_all(hid_all_color.r, hid_all_color.g, hid_all_color.b);
          }
          return;
      }

      // Only apply layer colors when NOT in HID override mode
      switch (current_layer) {
          case 0:
              rgb_matrix_set_color_all(RGB_WHITE);
              break;
          case 1:
              rgb_matrix_set_color_all(RGB_GREEN);
              break;
          case 2:
              rgb_matrix_set_color_all(RGB_RED);
              break;
      }
  }
