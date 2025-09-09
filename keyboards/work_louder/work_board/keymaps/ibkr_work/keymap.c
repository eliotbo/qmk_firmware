/* Copyright 2025 IBKR Work Trading Keymap
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
#include "raw_hid.h"

/* IBKR Work Board Trading Keyboard Layout
 * ========================================
 * 
 * Physical Layout (4x13 grid + 1 encoder):
 * 
 * [1] [2] [3] [4] [5] [6] [7] [8] [9] [0] [-] [=] [Enc]
 * [Q] [W] [E] [R] [T] [Y] [U] [I] [O] [P] [[] []]
 * [A] [S] [D] [F] [G] [H] [J] [K] [L] [;] ['] [Enter]
 * [Z] [X] [C] [V] [B] [Space] [N] [M] [,] [.] [/]
 *
 * Encoder Function:
 * - Press: Switch between Layer 0 and Layer 1 using TO(1) and TO(0)
 *
 * All keys send Alt+Character combinations for ticker bindings
 * App can send HID commands to control individual LED lights for ticker focus
 */

// Raw HID command definitions
enum hid_commands {
    CMD_SET_ALL  = 1, // [1, r, g, b] - Set all LEDs to a color
    CMD_SET_ONE  = 2, // [2, led_index, r, g, b] - Set specific LED
    CMD_CLEAR    = 3, // [3] - Clear all LEDs to default
    CMD_FOCUS    = 4, // [4, led_index] - Focus on one LED (light it, dim others)
};

// Store individual LED colors when in HID override mode
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

// Store LED colors for all keys (48 total on work_board)
led_color_t hid_led_colors[48] = {{0}};
bool hid_rgb_override = false;
uint8_t focused_led = 255;  // 255 means no focus

enum layers {
    _LAYER0,   // Layer 0: First set of tickers
    _LAYER1    // Layer 1: Second set of tickers
};

enum custom_keycodes {
    // All keys send Alt+Character combinations
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
    ALT_SLSH
};

// Track current layer
uint8_t current_layer = _LAYER0;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Layer 0
     * First set of ticker bindings via Alt+Character
     * Layout matches default QWERTY positions
     */
    [_LAYER0] = LAYOUT(
        ALT_1,    ALT_Q,    ALT_W,    ALT_E,    ALT_R,    ALT_T,    ALT_Y,    ALT_U,    ALT_I,    ALT_O,    ALT_P,    ALT_MINUS, TO(1),
        ALT_2,    ALT_A,    ALT_S,    ALT_D,    ALT_F,    ALT_G,    ALT_H,    ALT_J,    ALT_K,    ALT_L,    ALT_SCLN, ALT_QUOT,
        ALT_3,    ALT_Z,    ALT_X,    ALT_C,    ALT_V,    ALT_B,    ALT_N,    ALT_M,    ALT_COMM, ALT_DOT,  ALT_4,    ALT_5,
        ALT_6,    ALT_7,    ALT_8,    ALT_9,    ALT_0,    ALT_EQUAL, ALT_LBRC, ALT_RBRC, ALT_SLSH, ALT_A,    ALT_S,    ALT_D
    ),
    
    /* Layer 1
     * Second set of ticker bindings via Alt+Character
     * (Same keys but app will interpret them differently based on layer)
     */
    [_LAYER1] = LAYOUT(
        ALT_1,    ALT_Q,    ALT_W,    ALT_E,    ALT_R,    ALT_T,    ALT_Y,    ALT_U,    ALT_I,    ALT_O,    ALT_P,    ALT_MINUS, TO(0),
        ALT_2,    ALT_A,    ALT_S,    ALT_D,    ALT_F,    ALT_G,    ALT_H,    ALT_J,    ALT_K,    ALT_L,    ALT_SCLN, ALT_QUOT,
        ALT_3,    ALT_Z,    ALT_X,    ALT_C,    ALT_V,    ALT_B,    ALT_N,    ALT_M,    ALT_COMM, ALT_DOT,  ALT_4,    ALT_5,
        ALT_6,    ALT_7,    ALT_8,    ALT_9,    ALT_0,    ALT_EQUAL, ALT_LBRC, ALT_RBRC, ALT_SLSH, ALT_A,    ALT_S,    ALT_D
    )
};

// Raw HID receive handler for external LED control
void raw_hid_receive(uint8_t *data, uint8_t length) {
    switch (data[0]) {
        case CMD_SET_ALL:
            if (length >= 4) {
                hid_rgb_override = true;
                focused_led = 255;  // Clear focus
                uint8_t r = data[1];
                uint8_t g = data[2];
                uint8_t b = data[3];
                for (uint8_t i = 0; i < 48; i++) {
                    hid_led_colors[i].r = r;
                    hid_led_colors[i].g = g;
                    hid_led_colors[i].b = b;
                }
            }
            break;

        case CMD_SET_ONE:
            if (length >= 5) {
                hid_rgb_override = true;
                uint8_t led_index = data[1];
                if (led_index < 48) {
                    hid_led_colors[led_index].r = data[2];
                    hid_led_colors[led_index].g = data[3];
                    hid_led_colors[led_index].b = data[4];
                }
            }
            break;

        case CMD_CLEAR:
            hid_rgb_override = false;
            focused_led = 255;
            break;

        case CMD_FOCUS:
            if (length >= 2) {
                hid_rgb_override = true;
                uint8_t led_index = data[1];
                if (led_index < 48) {
                    focused_led = led_index;
                    // Set focused LED to bright white, others to dim
                    for (uint8_t i = 0; i < 48; i++) {
                        if (i == focused_led) {
                            // Bright white for focused ticker
                            hid_led_colors[i].r = 255;
                            hid_led_colors[i].g = 255;
                            hid_led_colors[i].b = 255;
                        } else {
                            // Dim blue for unfocused tickers
                            hid_led_colors[i].r = 0;
                            hid_led_colors[i].g = 0;
                            hid_led_colors[i].b = 30;
                        }
                    }
                }
            }
            break;
    }
    
    // Send acknowledgment back
    uint8_t send_data[32] = {0xFF, data[0]};  // Echo command back
    raw_hid_send(send_data, 32);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Get the actual matrix position from the key event
    uint8_t row = record->event.key.row;
    uint8_t col = record->event.key.col;
    uint8_t led_index = 255;
    
    // Based on g_led_config matrix from work_board.c:
    // Row 0: { 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 }
    // Row 1: { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25 }
    // Row 2: { 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 }
    // Row 3: { 12, 11, 10,  9,  8,  7,  5,  4,  3,  2,  1,  0 }
    
    // Create LED mapping table matching g_led_config
    static const uint8_t led_map[4][12] = {
        { 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 },
        { 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25 },
        { 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 },
        { 12, 11, 10,  9,  8,  7,  5,  4,  3,  2,  1,  0 }
    };
    
    // Get LED index from matrix position
    if (row < 4 && col < 12) {
        led_index = led_map[row][col];
    }
    
    // Send HID report to app about key press with LED index
    if (record->event.pressed && led_index != 255) {
        uint8_t send_data[32] = {0xFE, led_index, current_layer};  // Key press notification
        raw_hid_send(send_data, 32);
    }
    
    switch (keycode) {
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
                tap_code(KC_LBRC);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_RBRC:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_RBRC);
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
                tap_code(KC_SCLN);
                unregister_code(KC_LALT);
            }
            return false;
        case ALT_QUOT:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_QUOT);
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
                tap_code(KC_COMM);
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
                tap_code(KC_SLSH);
                unregister_code(KC_LALT);
            }
            return false;
    }
    
    return true;
}

// Layer state management
layer_state_t layer_state_set_user(layer_state_t state) {
    current_layer = get_highest_layer(state);
    
    // Send layer change notification via HID
    uint8_t send_data[32] = {0xFD, current_layer};  // Layer change notification
    raw_hid_send(send_data, 32);
    
    // If not in HID override mode, show layer color
    if (!hid_rgb_override) {
        switch (current_layer) {
            case _LAYER0:
                rgb_matrix_set_color_all(0, 0, 100);  // Blue for layer 0
                break;
            case _LAYER1:
                rgb_matrix_set_color_all(0, 100, 0);  // Green for layer 1
                break;
        }
    }
    
    return state;
}

// Initialize RGB on startup
void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_set_color_all(0, 0, 100);  // Start with blue
}

// RGB Matrix indicator - runs continuously to maintain LED states
void rgb_matrix_indicators_user(void) {
    if (hid_rgb_override) {
        // Apply stored LED colors from HID commands
        for (uint8_t i = 0; i < 48; i++) {
            rgb_matrix_set_color(i, hid_led_colors[i].r, hid_led_colors[i].g, hid_led_colors[i].b);
        }
    } else {
        // Show layer colors when not controlled by HID
        switch (current_layer) {
            case _LAYER0:
                rgb_matrix_set_color_all(0, 0, 100);  // Blue
                break;
            case _LAYER1:
                rgb_matrix_set_color_all(0, 100, 0);  // Green
                break;
        }
    }
}