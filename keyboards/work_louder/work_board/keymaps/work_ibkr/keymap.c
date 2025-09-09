/* Copyright 2025 Work IBKR Keymap
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

/* Work Board IBKR Trading Keyboard Layout
 * ========================================
 * 
 * Physical Layout (4x13 grid + 1 encoder):
 * 
 * [1] [2] [3] [4] [5] [6] [7] [8] [9] [0] [-] [=] [Enc]
 * [Q] [W] [E] [R] [T] [Y] [U] [I] [O] [P] [[] []]
 * [A] [S] [D] [F] [G] [H] [J] [K] [L] [;] ['] [Enter]
 * [Z] [X] [C] [V] [B] [Space] [N] [M] [,] [.] [/]
 *
 * Encoder Functions:
 * - Press: Switch between Layer 0 and Layer 1
 * - Turn: Send MIDI CC based on current layer
 *   - Layer 0: CC 20/21 (decrement/increment)
 *   - Layer 1: CC 22/23 (decrement/increment)
 *
 * Keys send Alt+Character combinations for trading functions
 */

// MIDI CC definitions for encoder
#define CC_LAYER0_DOWN  20
#define CC_LAYER0_UP    21
#define CC_LAYER1_DOWN  22
#define CC_LAYER1_UP    23

// MIDI CC values
#define MIDI_CC_OFF     0
#define MIDI_CC_ON      127

enum layers {
    _LAYER0,   // Layer 0: First set of Alt+Character bindings
    _LAYER1    // Layer 1: Second set of Alt+Character bindings
};

enum custom_keycodes {
    // Layer 0 Alt+Character combinations
    ALT_1 = SAFE_RANGE,
    ALT_2,
    ALT_3,
    ALT_4,
    ALT_5,
    ALT_6,
    ALT_7,
    ALT_8,
    ALT_9,
    ALT_0,
    ALT_MINUS,
    ALT_EQUAL,
    ALT_Q,
    ALT_W,
    ALT_E,
    ALT_R,
    ALT_T,
    ALT_Y,
    ALT_U,
    ALT_I,
    ALT_O,
    ALT_P,
    ALT_LBRC,
    ALT_RBRC,
    ALT_A,
    ALT_S,
    ALT_D,
    ALT_F,
    ALT_G,
    ALT_H,
    ALT_J,
    ALT_K,
    ALT_L,
    ALT_SCLN,
    ALT_QUOT,
    ALT_Z,
    ALT_X,
    ALT_C,
    ALT_V,
    ALT_B,
    ALT_N,
    ALT_M,
    ALT_COMM,
    ALT_DOT,
    ALT_SLSH,
    
    // Encoder press to toggle layers
    ENC_TOGGLE
};

// Track current layer for MIDI channel
uint8_t current_layer = _LAYER0;

// Encoder map for MIDI control - different CC per layer
#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_LAYER0] = { ENCODER_CCW_CW(KC_NO, KC_NO) },  // We'll handle this manually
    [_LAYER1] = { ENCODER_CCW_CW(KC_NO, KC_NO) }   // We'll handle this manually
};
#endif

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Layer 0
     * Alt+Character combinations for trading functions
     */
    [_LAYER0] = LAYOUT(
        ALT_1,    ALT_2,    ALT_3,    ALT_4,    ALT_5,    ALT_6,    ALT_7,    ALT_8,    ALT_9,    ALT_0,    ALT_MINUS, ALT_EQUAL, ENC_TOGGLE,
        ALT_Q,    ALT_W,    ALT_E,    ALT_R,    ALT_T,    ALT_Y,    ALT_U,    ALT_I,    ALT_O,    ALT_P,    ALT_LBRC,  ALT_RBRC,
        ALT_A,    ALT_S,    ALT_D,    ALT_F,    ALT_G,    ALT_H,    ALT_J,    ALT_K,    ALT_L,    ALT_SCLN, ALT_QUOT,  KC_ENT,
        ALT_Z,    ALT_X,    ALT_C,    ALT_V,    ALT_B,    KC_SPC,   KC_SPC,   ALT_N,    ALT_M,    ALT_COMM, ALT_DOT,   ALT_SLSH
    ),
    
    /* Layer 1
     * Same Alt+Character but could be mapped to different functions via software
     */
    [_LAYER1] = LAYOUT(
        ALT_1,    ALT_2,    ALT_3,    ALT_4,    ALT_5,    ALT_6,    ALT_7,    ALT_8,    ALT_9,    ALT_0,    ALT_MINUS, ALT_EQUAL, ENC_TOGGLE,
        ALT_Q,    ALT_W,    ALT_E,    ALT_R,    ALT_T,    ALT_Y,    ALT_U,    ALT_I,    ALT_O,    ALT_P,    ALT_LBRC,  ALT_RBRC,
        ALT_A,    ALT_S,    ALT_D,    ALT_F,    ALT_G,    ALT_H,    ALT_J,    ALT_K,    ALT_L,    ALT_SCLN, ALT_QUOT,  KC_ENT,
        ALT_Z,    ALT_X,    ALT_C,    ALT_V,    ALT_B,    KC_SPC,   KC_SPC,   ALT_N,    ALT_M,    ALT_COMM, ALT_DOT,   ALT_SLSH
    )
};

// Manual encoder handling for MIDI control
bool encoder_update_user(uint8_t index, bool clockwise) {
    // Send different MIDI CC based on current layer
    uint8_t midi_channel = 0;  // Using channel 1 (0-indexed)
    
    if (current_layer == _LAYER0) {
        if (clockwise) {
            midi_send_cc(&midi_device, midi_channel, CC_LAYER0_UP, MIDI_CC_ON);
        } else {
            midi_send_cc(&midi_device, midi_channel, CC_LAYER0_DOWN, MIDI_CC_ON);
        }
    } else {  // _LAYER1
        if (clockwise) {
            midi_send_cc(&midi_device, midi_channel, CC_LAYER1_UP, MIDI_CC_ON);
        } else {
            midi_send_cc(&midi_device, midi_channel, CC_LAYER1_DOWN, MIDI_CC_ON);
        }
    }
    
    return false;  // Don't process further
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        // Encoder press toggles between layers
        case ENC_TOGGLE:
            if (record->event.pressed) {
                if (current_layer == _LAYER0) {
                    layer_on(_LAYER1);
                    current_layer = _LAYER1;
                } else {
                    layer_off(_LAYER1);
                    current_layer = _LAYER0;
                }
            }
            return false;
            
        // Alt+Number keys
        case ALT_1:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_1);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_2:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_2);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_3:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_3);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_4:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_4);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_5:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_5);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_6:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_6);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_7:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_7);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_8:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_8);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_9:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_9);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_0:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_0);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_MINUS:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_MINUS);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_EQUAL:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_EQUAL);
                unregister_code(KC_LALT);
            }
            return false;
            
        // Alt+Letter keys
        case ALT_Q:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_Q);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_W:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_W);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_E:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_E);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_R:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_R);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_T:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_T);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_Y:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_Y);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_U:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_U);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_I:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_I);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_O:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_O);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_P:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_P);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_LBRC:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_LBRACKET);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_RBRC:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_RBRACKET);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_A:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_A);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_S:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_S);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_D:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_D);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_F:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_F);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_G:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_G);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_H:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_H);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_J:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_J);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_K:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_K);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_L:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_L);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_SCLN:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_SCOLON);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_QUOT:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_QUOTE);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_Z:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_Z);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_X:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_X);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_C:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_C);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_V:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_V);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_B:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_B);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_N:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_N);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_M:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_M);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_COMM:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_COMMA);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_DOT:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_DOT);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_SLSH:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_SLASH);
                unregister_code(KC_LALT);
            }
            return false;
    }
    
    return true;
}

// Set RGB colors based on current layer
layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
        case _LAYER0:
            // Blue for layer 0
            rgb_matrix_set_color_all(RGB_BLUE);
            break;
        case _LAYER1:
            // Green for layer 1
            rgb_matrix_set_color_all(RGB_GREEN);
            break;
    }
    
    return state;
}

// Initialize RGB on startup
void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_set_color_all(RGB_BLUE);  // Start with blue for layer 0
}

// RGB Matrix indicator - runs continuously to maintain layer colors
bool rgb_matrix_indicators_user(void) {
    switch (current_layer) {
        case _LAYER0:
            rgb_matrix_set_color_all(RGB_BLUE);
            break;
        case _LAYER1:
            rgb_matrix_set_color_all(RGB_GREEN);
            break;
    }
    return false;
}