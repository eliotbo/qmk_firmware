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

// Sequencer example:
// https://github.com/jpskenn/qmk_firmware/blob/develop_Unison/keyboards/unison/keymaps/music/keymap.c

#include QMK_KEYBOARD_H
extern MidiDevice midi_device;
#include "qmk_midi.h"

enum planck_layers {
  _NOTES,
  _CHANNEL,
//   _SEQUENCER,
  _ROUTER,

};

enum tap_dances {
    ENC_TAP,
};

// #define LOWER MO(_LOWER)
// #define RAISE MO(_RAISE)


#define MIDI_CC_OFF 0
#define MIDI_CC_ON  127

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


enum new_keycodes {
    FN_MO13 = SAFE_RANGE,
    FN_MO23,
    MI_CC_01,
    MI_CC_02,
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
    MACRO00,
    MACRO01,
    MACRO02,
    MACRO03,
    MACRO04,
    MACRO05,
    MACRO06,//
    MACRO07,
    MACRO08,

};

#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    // [_QWERTY] = { ENCODER_CCW_CW(MI_VELD, MI_VELU) },
    // [_LOWER]  = { ENCODER_CCW_CW(KC_PGDN, KC_PGUP) },
    // [_RAISE]  = { ENCODER_CCW_CW(KC_VOLU, KC_VOLU) },
    [_NOTES] = { ENCODER_CCW_CW(MI_VELD, MI_VELU) },
    [_CHANNEL]  = { ENCODER_CCW_CW(MI_MODSD, MI_MODSU) },
    // [_SEQUENCER] = { ENCODER_CCW_CW(SQ_TMPD, SQ_TMPU) },
    [_ROUTER] = { ENCODER_CCW_CW(MI_VELD, MI_VELU) }
};
#endif



const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_NOTES] = LAYOUT(
        MI_C_4,  MI_D_4, MI_E_4, MI_F_4, MI_G_4, MI_A_4, MI_B_4, MI_C_4,  MI_C_4, MI_C_4,   MI_E_5, MI_G_5, MI_MOD,
        MI_C_3,  MI_D_3, MI_E_3, MI_F_3, MI_G_3, MI_A_3, MI_B_3, MI_C_4,  MI_D_4,  MI_E_4,  MI_G_4, MI_A_4,
        MI_C_2,  MI_D_2, MI_E_2, MI_F_2, MI_G_2, MI_A_2, MI_B_2, MI_C_3,  MI_D_3,  MI_E_3,  MI_G_3, MI_A_3,
        MI_PORT, MI_SOST, MI_SOFT, MI_LEG, MI_MOD, TO(3), TO(3), TO(3),  KC_TRNS,  MI_OCTD,  MI_OCTU, MI_SUS
    ),

    [_CHANNEL] = LAYOUT(
        MI_C_4,  MI_D_4,  MI_E_4,  MI_F_4,  MI_G_4,  MI_A_4,  MI_B_4,  MI_C_4,  MI_C_4,  MI_C_4,  MI_E_5,   MI_G_5,  MI_MOD,
        MI_CH1,  MI_CH2,  MI_CH3,  MI_CH4,  MI_CH5,  MI_CH6,  MI_CH7,  MI_CH8,  MI_CH9,  MI_CH10, MI_CH11,  MI_CH12,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP,    KC_TRNS,
        TO(0),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN,  KC_RGHT
    ),

    // [_SEQUENCER] = LAYOUT(
    //     SQ_ON,  SQ_RES_8,  MI_CH3,  MI_CH4,  MI_CH5,  MI_CH6,  MI_CH7,  MI_CH8,  MI_CH9,  MI_CH10, MI_CH11,  MI_CH12, MI_MOD,
    //     MACRO00, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
    //     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP,    KC_TRNS,
    //     TO(0),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN,  KC_RGHT
    // ),

    [_ROUTER] = LAYOUT(
        TO(0),   TO(1),   TO(2),   TO(3),   TO(4),   TO(5),   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC, KC_NO,
        KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP,   KC_ENT,
        TO(0),   KC_LCTL, KC_LALT, KC_LGUI, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SLSH, KC_LEFT, KC_DOWN, KC_RGHT
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
        case MI_CC_01:
            if (record->event.pressed) {
                midi_send_cc(&midi_device, midi_config.channel, 1, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_config.channel, 1, MIDI_CC_OFF);
            }
            return true;
        case MI_CC_02:
            if (record->event.pressed) {
                midi_send_cc(&midi_device, midi_config.channel, 2, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_config.channel, 2, MIDI_CC_OFF);
            }
            return true;
        case USER09:
            preprocess_tap_dance(TD(ENC_TAP), record);
            return process_tap_dance(TD(ENC_TAP), record);
        case MACRO00:
            if (record->event.pressed) {
                // if (is_sequencer_on()) {
                //     sequencer_set_all_steps_on();
                //     sequencer_activate_track(0);
                //     sequencer_set_tempo(122);
                //     writePinHigh(B2);
                //     writePinHigh(B3);
                //     writePinHigh(B7);
                // }

                // register_code(KC_A); //this means to send F22 down
                // unregister_code(KC_F22);
                // SEND_STRING(SS_LCTL("ac")); // this selects all and copies

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


