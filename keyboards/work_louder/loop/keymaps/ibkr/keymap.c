/* Copyright 2025 IBKR Trading Keymap
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "midi.h"
#include "qmk_midi.h"

extern MidiDevice midi_device;

/* IBKR Trading Keyboard Layout
 * ============================
 * 
 * Physical Layout (3x3 grid + 3 encoders):
 * 
 *  [Enc0]    [Enc1]    [Enc2]
 *  
 *  [0] [1] [2]
 *  [3] [4] [5]  
 *  [6] [7] [8]
 *
 * Encoder Functions (via MIDI CC):
 * - Encoder 0: Share quantity - MIDI CC 20/21 (decrement/increment)
 * - Encoder 1: Stop loss - MIDI CC 22/23 (decrease/increase)
 * - Encoder 2: Limit price - MIDI CC 24/25 (down/up)
 *
 * Key [8] is used for layer switching (Base -> Buy -> Sell -> Base)
 * Keys [0-7] send unique combinations for each layer
 * 
 * MIDI Channel Assignment:
 * - Channel 1: Base layer
 * - Channel 2: Buy layer  
 * - Channel 3: Sell layer
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
    
    // Buy layer actions (using Alt+Shift+F13-F20)
    BUY_1,
    BUY_2,
    BUY_3,
    BUY_4,
    BUY_5,
    BUY_6,
    BUY_7,
    BUY_8,
    
    // Sell layer actions (using Ctrl+Alt+F13-F20)
    SELL_1,
    SELL_2,
    SELL_3,
    SELL_4,
    SELL_5,
    SELL_6,
    SELL_7,
    SELL_8,
    
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
        BASE_7,     BASE_8,     TO(1)
    ),
    
    /* Buy Layer
     * Buy-specific actions
     */
    [_BUY] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        BUY_1,      BUY_2,      BUY_3,
        BUY_4,      BUY_5,      BUY_6,
        BUY_7,      BUY_8,      TO(2)
    ),
    
    /* Sell Layer
     * Sell-specific actions
     */
    [_SELL] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        SELL_1,     SELL_2,     SELL_3,
        SELL_4,     SELL_5,     SELL_6,
        SELL_7,     SELL_8,     TO(0)
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
                // Send multiple CCs for acceleration
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_SHARES_UP, MIDI_CC_ON);
                    }
                }
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_SHARES_DOWN, MIDI_CC_ON);
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_SHARES_DOWN, MIDI_CC_ON);
                    }
                }
            }
            break;
            
        case 1:  // Stop loss encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_UP, MIDI_CC_ON);
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_STOP_UP, MIDI_CC_ON);
                    }
                }
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_DOWN, MIDI_CC_ON);
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_STOP_DOWN, MIDI_CC_ON);
                    }
                }
            }
            break;
            
        case 2:  // Limit price encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_UP, MIDI_CC_ON);
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_LIMIT_UP, MIDI_CC_ON);
                    }
                }
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_DOWN, MIDI_CC_ON);
                if (encoder_shift_mode) {
                    for (int i = 0; i < 9; i++) {
                        midi_send_cc(&midi_device, midi_channel, CC_LIMIT_DOWN, MIDI_CC_ON);
                    }
                }
            }
            break;
    }
    
    return false;  // Don't process further
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
                tap_code(KC_F13);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F14);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F15);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F16);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F17);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F18);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_7:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F19);
                unregister_code(KC_LSFT);
            }
            return false;
        case BASE_8:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F20);
                unregister_code(KC_LSFT);
            }
            return false;
            
        // Buy layer keys - Shift+F21 through Shift+F28
        case BUY_1:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F21);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F22);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F23);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F24);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F25);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F26);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_7:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F27);
                unregister_code(KC_LSFT);
            }
            return false;
        case BUY_8:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F28);
                unregister_code(KC_LSFT);
            }
            return false;
            
        // Sell layer keys - Shift+F29 through Shift+F35, last one already has Shift
        case SELL_1:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F29);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_2:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F30);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_3:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F31);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_4:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F32);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_5:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F33);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_6:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F34);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_7:
            if (record->event.pressed) {
                register_code(KC_LSFT);
                tap_code(KC_F35);
                unregister_code(KC_LSFT);
            }
            return false;
        case SELL_8:
            if (record->event.pressed) {
                // Use Ctrl+Shift+F35 for uniqueness
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                tap_code(KC_F35);
                unregister_code(KC_LSFT);
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
                        tap_code(KC_F1);  // Base: Ctrl+Shift+F1
                        break;
                    case _BUY:
                        tap_code(KC_F2);  // Buy: Ctrl+Shift+F2
                        break;
                    case _SELL:
                        tap_code(KC_F3);  // Sell: Ctrl+Shift+F3
                        break;
                }
                unregister_code(KC_LSFT);
                unregister_code(KC_LCTL);
            }
            return false;
            
        case ENC1_PRESS:  // Toggle stop loss type ($/%)
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                switch(current_layer) {
                    case _BASE:
                        tap_code(KC_F4);  // Base: Ctrl+Shift+F4
                        break;
                    case _BUY:
                        tap_code(KC_F5);  // Buy: Ctrl+Shift+F5
                        break;
                    case _SELL:
                        tap_code(KC_F6);  // Sell: Ctrl+Shift+F6
                        break;
                }
                unregister_code(KC_LSFT);
                unregister_code(KC_LCTL);
            }
            return false;
            
        case ENC2_PRESS:  // Toggle order type (market/limit)
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                switch(current_layer) {
                    case _BASE:
                        tap_code(KC_F7);  // Base: Ctrl+Shift+F7
                        break;
                    case _BUY:
                        tap_code(KC_F8);  // Buy: Ctrl+Shift+F8
                        break;
                    case _SELL:
                        tap_code(KC_F9);  // Sell: Ctrl+Shift+F9
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
    
    switch (current_layer) {
        case 0:
            // White for base layer
            rgb_matrix_set_color_all(RGB_WHITE);
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

// RGB Matrix indicator - runs continuously to maintain layer colors
void rgb_matrix_indicators_user(void) {
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