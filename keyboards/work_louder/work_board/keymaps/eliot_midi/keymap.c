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
// 3. make work_louder/work_board:eliot_midi

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

    MI_CC_80 = MIDI_CHANNEL_MIN,
    MI_CC_81,
    MI_CC_82,
    MI_CC_83,
    MI_CC_84,
    MI_CC_85,
    MI_CC_86,
    MI_CC_87,
    MI_CC_88,
    MI_CC_89,
    MI_CC_90,
    MI_CC_91,
    MI_CC_92,
    MI_CC_93,
    MI_CC_94,
    MI_CC_95,
    MI_CC_96,
    MI_CC_97,
    MI_CC_98,
    MI_CC_99,
    MI_CC_100,
    MI_CC_101,
    MI_CC_102,
    MI_CC_103,
    MI_CC_104,
    MI_CC_105,
    MI_CC_106,
    MI_CC_107,
    MI_CC_108,
    MI_CC_109,
    MI_CC_110,
    MI_CC_111,
    MI_CC_112,
    MI_CC_113,
    MI_CC_114,
    MI_CC_115,
    MI_CC_116,
    MI_CC_117,
    MI_CC_118,
    MI_CC_119,
    MI_CC_120,
    MI_CC_121,
    MI_CC_122,
    MI_CC_123,
    MI_CC_124,
    MI_CC_125,
    MI_CC_126,
    MI_CC_127,
    MI_CC_128,
    MI_CC_129,
    MI_CC_130,
    MI_CC_131,
    MI_CC_132,

    // MI_CC_01,
    // MI_CC_02,
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

// MI_CH1 		Set channel to 1
// MI_CH2 		Set channel to 2
// MI_CH3 		Set channel to 3
// MI_CH4 		Set channel to 4
// MI_CH5 		Set channel to 5
// MI_CH6 		Set channel to 6
// MI_CH7 		Set channel to 7
// MI_CH8 		Set channel to 8
// MI_CH9 		Set channel to 9
// MI_CH10 		Set channel to 10
// MI_CH11 		Set channel to 11
// MI_CH12 		Set channel to 12
// MI_CH13 		Set channel to 13
// MI_CH14 		Set channel to 14
// MI_CH15 		Set channel to 15
// MI_CH16 		Set channel to 16
// MI_CHD 		Decrease channel
// MI_CHU 		Increase channel

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_NOTES] = LAYOUT(
        //
        MI_C_4,  MI_D_4, MI_E_4, MI_F_4, MI_G_4, MI_A_4, MI_B_4, MI_C_4,  RGB_TOG, RGB_MOD,   R_M_TOG, R_M_MOD, KC_NO,
        MI_C_3,  MI_D_3, MI_E_3, MI_F_3, MI_G_3, MI_A_3, MI_B_3, MI_C_4,  MI_D_4,  MI_E_4,  MI_G_4, MI_A_4,
        MI_C_2,  MI_D_2, MI_E_2, MI_F_2, MI_G_2, MI_A_2, MI_B_2, MI_C_3,  MI_D_3,  MI_E_3,  MI_G_3, MI_A_3,
        MI_PORT, MI_SOST, MI_SOFT, MI_LEG, MI_MOD, TO(1), TO(1), TO(2),  KC_TRNS,  MI_CHD,  KC_NO, MI_CHU
    ),

    [_CHANNEL] = LAYOUT(
        MI_CC_80, MI_CC_81, MI_CC_82, MI_CC_83, MI_CC_84, MI_CC_85, MI_CC_86, MI_CC_87, MI_CC_88, MI_CC_89, MI_CC_90, MI_CC_91, MI_CC_92,
        MI_CC_93, MI_CC_94, MI_CC_95, MI_CC_96, MI_CC_97, MI_CC_98, MI_CC_99, MI_CC_100, MI_CC_101, MI_CC_102, MI_CC_103, MI_CC_104,
        MI_CC_105, MI_CC_106, MI_CC_107, MI_CC_108, MI_CC_109, MI_CC_110, MI_CC_111, MI_CC_112, MI_CC_113, MI_CC_114, MI_CC_115, MI_CC_116,
        MI_CC_117, MI_CC_118, MI_CC_119, MI_CC_120, MI_CC_121,  TO(0),  TO(0), MI_CC_122, MI_CC_123, MI_CHD, MI_CC_125, MI_CHU
    ),

    // [_CHANNEL] = LAYOUT(
    //     MI_C_4,  MI_D_4,  MI_E_4,  MI_F_4,  MI_G_4,  MI_A_4,  MI_B_4,  MI_C_4,  MI_C_4,  MI_C_4,  MI_E_5,   MI_G_5,  KC_NO,
    //     MI_CH1,  MI_CH2,  MI_CH3,  MI_CH4,  MI_CH5,  MI_CH6,  MI_CH7,  MI_CH8,  MI_CH9,  MI_CH10, MI_CH11,  MI_CH12,
    //     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP,    KC_TRNS,
    //     TO(0),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN,  KC_RGHT
    // ),

    // [_SEQUENCER] = LAYOUT(
    //     SQ_ON,  SQ_RES_8,  MI_CH3,  MI_CH4,  MI_CH5,  MI_CH6,  MI_CH7,  MI_CH8,  MI_CH9,  MI_CH10, MI_CH11,  MI_CH12, MI_MOD,
    //     MACRO00, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,  KC_TRNS,
    //     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP,    KC_TRNS,
    //     TO(0),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN,  KC_RGHT
    // ),

    // [_ROUTER] = LAYOUT(
    //     TO(0),   TO(1),   TO(2),   TO(2),   TO(2),   TO(2),   KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC, KC_NO,
    //     MI_CH1,  MI_CH2, MI_CH3,  MI_CH4,  MI_CH5,  MI_CH6,  MI_CH7,  MI_CH8,  MI_CH9,  MI_CH10, MI_CH11,  MI_CH12,
    //     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP,   KC_ENT,
    //     TO(0),   KC_LCTL, KC_LALT, KC_LGUI, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SLSH, MI_CHD, KC_DOWN, MI_CHU
    // )

    [_ROUTER] = LAYOUT(
        _______, QK_BOOT,   _______, RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, KC_DEL , R_M_TOG,
        _______, _______, MU_MOD,  R_M_TOG, R_M_MOD, R_M_HUI, R_M_HUD, R_M_SAI, R_M_SAD, R_M_VAI, R_M_VAD, RGB_M_P,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_UP,   KC_ENT,
        TO(0),   KC_LCTL, KC_LALT, KC_LGUI, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_SLSH, MI_CHD, KC_DOWN, MI_CHU
    )
};

void keyboard_post_init_user(void) {
    // rgb_matrix_enable_noeeprom();
    rgb_matrix_enable();
    // rgblight_disable_noeeprom();
    rgb_matrix_mode(RGB_MATRIX_CUSTOM_my_cool_effect);
    // rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    // rgb_matrix_set_color_all(RGB_OFF);
}

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
    rgb_matrix_set_color_all(20, 50, 200);
    // rgb_matrix_indicators_user
    // if (keycode == USER09) {
    //     preprocess_tap_dance(TD(ENC_TAP), record);
    //     return process_tap_dance(TD(ENC_TAP), record);
    // }
        // TODO: ideally this would be generalized and refactored into
    // QMK core as advanced keycodes, until then, the simple case
    // can be available here to keyboards using VIA
    switch (keycode) {
        // case MI_CC_01:
        //     if (record->event.pressed) {
        //         midi_send_cc(&midi_device, midi_config.channel, 1, MIDI_CC_ON);
        //     } else {
        //         midi_send_cc(&midi_device, midi_config.channel, 1, MIDI_CC_OFF);
        //     }
        //     return true;
        // case MI_CC_02:
        //     if (record->event.pressed) {
        //         midi_send_cc(&midi_device, midi_config.channel, 2, MIDI_CC_ON);
        //     } else {
        //         midi_send_cc(&midi_device, midi_config.channel, 2, MIDI_CC_OFF);
        //     }
        //     return true;

        // case MI_VELU:
        //     if (record->event.pressed && midi_config.velocity < 127) {
        //         if (midi_config.velocity < 115) {
        //             midi_config.velocity += 13;
        //         } else {
        //             midi_config.velocity = 127;
        //         }
        //         dprintf("midi velocity %d\n", midi_config.velocity);
        //     }
        //     return false;
        // case MIDI_CHANNEL_MIN ... MIDI_CHANNEL_MAX:
        //     if (record->event.pressed) {
        //         midi_config.channel = keycode - MIDI_CHANNEL_MIN;
        //         dprintf("midi channel %d\n", midi_config.channel);
        //     }
        //     return false;

        case MI_CC_80:
            if (record->event.pressed) {
                midi_send_cc(&midi_device, midi_config.channel, 80, MIDI_CC_ON);
                rgb_matrix_set_color_all(50, 200, 25);
                writePinHigh(B2);
            } else {
                midi_send_cc(&midi_device, midi_config.channel, 80, MIDI_CC_OFF);
                rgb_matrix_set_color_all(20, 50, 200);
                writePinLow(B2);
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

// // TODO: use binary code using the three leds to indicate the current layer
// layer_state_t layer_state_set_user(layer_state_t state) {
//     writePinLow(B2);
//     writePinLow(B3);
//     writePinLow(B7);

//     switch (get_highest_layer(state)) {
//         case 1:
//             writePinHigh(B2);
//             break;
//         case 2:
//             writePinHigh(B3);
//             break;
//         case 3:
//             writePinHigh(B7);
//             break;
//     }

//     return state;
// }


