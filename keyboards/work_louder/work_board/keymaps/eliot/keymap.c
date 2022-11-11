/* Copyright 2015-2017 Jack Humbert
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

// 1. open QMK MSYS
// 2. cd qmk_firmware
// 3. make work_louder/work_board:eliot

// converting a VIA json file to a keymap.json file
// qmk via2json -kb work_louder/work_board -o not_via.json /c/Users/eliotbolduc/Documents/keyboards/workboard_5.json

#include QMK_KEYBOARD_H

enum planck_layers {
  _QWERTY,
  _LOWER,
  _RAISE,
  _ADJUST,
  _CHOU,
  _CHOUNETTE,
};

enum tap_dances {
    ENC_TAP,
};

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)


// TODO: occupy safe range:
// What happens here is this: We first define a new custom keycode in the range not occupied by any other keycodes.
// see https://github.com/qmk/qmk_firmware/blob/master/docs/feature_macros.md

// generate json for a keymap (current directory should be  ./qmk_firmware ):
//
// qmk c2json -km eliot -kb work_louder/work_board ./keyboards/work_louder/work_board/keymaps/eliot/keymap.c -o eliot.json
//
// generate the c format for the keymap layout
//
// qmk json2c -o eliotc.c eliot.json

// TODO: linux?
// Keep practicing the linux way, it's really not that bad. After its all setup you can build and flash with one line.
// install avrdude

// enum custom_keycodes {
//     QMKBEST = SAFE_RANGE,
// };

// enum new_keycodes {
//     FN_MO13 = 0x5F10,
//     FN_MO23,
//     MACRO00,
//     MACRO01,
//     MACRO02,
//     MACRO03,
//     MACRO04,
//     MACRO05,
//     MACRO06,
//     MACRO07,
//     MACRO08,
//     MACRO09,
//     MACRO10,
//     MACRO11,
//     MACRO12,
//     MACRO13,
//     MACRO14,
//     MACRO15,
// };

// enum user_keycodes {
//     USER00 = 0x5F80,
//     USER01,
//     USER02,
//     USER03,
//     USER04,
//     USER05,
//     USER06,
//     USER07,
//     USER08,
//     USER09,
//     USER10,
//     USER11,
//     USER12,
//     USER13,
//     USER14,
//     USER15,
// };

enum new_keycodes {
    FN_MO13 = SAFE_RANGE,
    FN_MO23,
    USER00,
    USER01,
    USER02,
    USER03,
    USER04,
    USER05,
    USER06,
    USER07,
    USER08,
    USER09,
    USER10,
    USER11,
    USER12,
    USER13,
    USER14,
    USER15,
    MACRO00,
    MACRO01,
    MACRO02,
    MACRO03,
    MACRO04,
    MACRO05,
    MACRO06,
    MACRO07,
    MACRO08,
    MACRO09,
    MACRO10,
    MACRO11,
    MACRO12,
    MACRO13,
    MACRO14,
    MACRO15,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_QWERTY] = LAYOUT(
        KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,       KC_T,       KC_Y,       KC_U,       KC_I,    KC_O,      KC_P,    KC_BSPC, USER09,
        KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,       KC_G,       KC_H,       KC_J,       KC_K,    KC_L,      KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,       KC_B,       KC_N,       KC_M,       KC_COMM, KC_DOT,    KC_SLSH, KC_RSFT,
        TO(5),   KC_LCTL, KC_LALT, FN_MO23, FN_MO13,    KC_SPC,     KC_SPC,     FN_MO13,    FN_MO23, KC_LEFT,   KC_DOWN, KC_RGHT
    ),

    [_LOWER] = LAYOUT(
        KC_TILD,    KC_EXLM,    KC_AT,      KC_HASH,    KC_DLR,     KC_PERC,    KC_CIRC,    KC_AMPR,    KC_ASTR,    KC_UNDS,    KC_PEQL,    KC_DEL,  KC_TRNS,
        KC_GRV,     KC_LBRC,    KC_RBRC,    KC_LT,      KC_GT,      KC_LPRN,    KC_RPRN,    KC_LCBR,    KC_RCBR,    KC_COLN,    KC_PMNS,    KC_ENT,
        KC_CAPS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_UP,      KC_BSLS,
        TO(0),      KC_LGUI,    KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_NO,      KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_LEFT,    KC_DOWN,    KC_RGHT
    ),

    [_RAISE] = LAYOUT(
        KC_GRV,     KC_PSLS,    KC_PAST,    KC_PMNS,    KC_PSCR,    KC_HOME,    KC_END,     KC_P7,      KC_P8,      KC_P9,      KC_PMNS,    KC_DEL,  KC_TRNS,
        KC_DEL,     KC_PEQL,    KC_CIRC,    KC_PPLS,    KC_NO,      KC_DEL,     KC_PGUP,    KC_P4,      KC_P5,      KC_P6,      KC_PPLS,    KC_ENT,
        MACRO01,    KC_NO,      KC_NO,      KC_NO,      KC_NO,      KC_DEL,     KC_PGDN,    KC_P1,      KC_P2,      KC_P3,      KC_UP,      KC_ENT,
        TO(0),      KC_NO,      KC_TRNS,    KC_TRNS,    KC_TRNS,    KC_SPC,     KC_TRNS,    KC_TRNS,    KC_P0,      KC_PDOT,    KC_DOWN,    KC_RGHT
    ),

    [_ADJUST] = LAYOUT(
        KC_A,    QK_BOOT, KC_NO,   RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, KC_DEL , R_M_TOG,
        KC_NO,   KC_NO,   MU_MOD,  R_M_TOG, R_M_MOD, R_M_HUI, R_M_HUD, R_M_SAI, R_M_SAD, R_M_VAI, R_M_VAD, KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        TO(0),   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO
    ),

    [_CHOU] = LAYOUT(
        KC_B,    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MACRO04,   MACRO05, USER00,
        KC_TRNS, MACRO07, KC_TRNS, KC_TRNS, MACRO02, MACRO03, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MACRO06,  KC_ENT,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP,    KC_ENT,
        TO(0),   KC_TRNS, KC_TRNS, TG(2),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TO(0),   KC_LEFT, KC_DOWN,  RESET
    ),

    [_CHOUNETTE] = LAYOUT(
        TO(0),   TO(1),   TO(2),   TO(3),   TO(4),   TO(5),   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC, KC_NO,
        KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP,   KC_ENT,
        TO(0),   KC_LCTL, KC_LALT, KC_LGUI, LOWER,   KC_SPC,  KC_SPC,  RAISE,   KC_SLSH, KC_LEFT, KC_DOWN, KC_RGHT
    )
};

void dance_enc_finished(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        register_code(KC_MPLY);
    } else if (state->count == 2) {
        register_code(KC_MNXT);
    } else {
        register_code(KC_MPRV);
    }
}

void dance_enc_reset(qk_tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_MPLY);
    } else if (state->count == 2) {
        unregister_code(KC_MNXT);
    } else {
        unregister_code(KC_MPRV);
    }
}

// Tap Dance definitions
qk_tap_dance_action_t tap_dance_actions[] = {
    [ENC_TAP] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_enc_finished, dance_enc_reset),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // if (keycode == USER09) {
    //     preprocess_tap_dance(TD(ENC_TAP), record);
    //     return process_tap_dance(TD(ENC_TAP), record);
    // }
        // TODO: ideally this would be generalized and refactored into
    // QMK core as advanced keycodes, until then, the simple case
    // can be available here to keyboards using VIA
    switch (keycode) {
        case USER09:
            preprocess_tap_dance(TD(ENC_TAP), record);
            return process_tap_dance(TD(ENC_TAP), record);
        case MACRO00:
            if (record->event.pressed) {
                register_code(KC_A); //this means to send F22 down
                // unregister_code(KC_F22);
                SEND_STRING(SS_LCTL("ac"));
                // SEND_STRING("yep");
            }
            return false;
            break;
        case FN_MO13:
            if (record->event.pressed) {
                layer_on(1);
                update_tri_layer(1, 2, 3);
            } else {
                layer_off(1);
                update_tri_layer(1, 2, 3);
            }
            return false;
            break;
        case FN_MO23:
            if (record->event.pressed) {
                layer_on(2);
                update_tri_layer(1, 2, 3);
            } else {
                layer_off(2);
                update_tri_layer(1, 2, 3);
            }
            return false;
            break;
    }
    return true;
}

// TODO: use binary code using the three leds to indicate the current layer
layer_state_t layer_state_set_user(layer_state_t state) {
    writePinLow(B2);
    writePinLow(B3);
    writePinLow(B7);

    switch (get_highest_layer(state)) {
        case 1:
            writePinHigh(B2);
            break;
        case 2:
            writePinHigh(B3);
            break;
        case 3:
            writePinHigh(B7);
            break;
    }

    return state;
}

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [_QWERTY] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [_LOWER]  = { ENCODER_CCW_CW(KC_PGDN, KC_PGUP) },
    [_RAISE]  = { ENCODER_CCW_CW(R_M_RMOD, R_M_MOD) },
    [_ADJUST] = { ENCODER_CCW_CW(R_M_HUI, R_M_HUD) },
    [_CHOU]  = { ENCODER_CCW_CW(R_M_RMOD, R_M_MOD) },
    [_CHOUNETTE] = { ENCODER_CCW_CW(R_M_HUI, R_M_HUD) },
};
#endif


