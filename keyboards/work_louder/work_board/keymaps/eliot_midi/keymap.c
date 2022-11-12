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
//   _CHANNEL,
  _SESSION,
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

    MIDI_CC_01,
    MIDI_CC_02,
    MIDI_CC_03,
    MIDI_CC_04,
    MIDI_CC_05,
    MIDI_CC_06,
    MIDI_CC_07,
    MIDI_CC_08,
    MIDI_CC_09,
    MIDI_CC_10,
    MIDI_CC_11,
    MIDI_CC_12,
    MIDI_CC_13,
    MIDI_CC_14,
    MIDI_CC_15,
    MIDI_CC_16,
    MIDI_CC_17,
    MIDI_CC_18,
    MIDI_CC_19,
    MIDI_CC_20,
    MIDI_CC_21,
    MIDI_CC_22,
    MIDI_CC_23,
    MIDI_CC_24,
    MIDI_CC_25,
    MIDI_CC_26,
    MIDI_CC_27,
    MIDI_CC_28,
    MIDI_CC_29,
    MIDI_CC_30,
    MIDI_CC_31,
    MIDI_CC_32,
    MIDI_CC_33,
    MIDI_CC_34,
    MIDI_CC_35,
    MIDI_CC_36,
    MIDI_CC_37,
    MIDI_CC_38,
    MIDI_CC_39,
    MIDI_CC_40,
    MIDI_CC_41,
    MIDI_CC_42,
    MIDI_CC_43,
    MIDI_CC_44,
    MIDI_CC_45,
    MIDI_CC_46,
    MIDI_CC_47,
    MIDI_CC_48,
    MIDI_CC_49,
    MIDI_CC_50,
    MIDI_CC_51,
    MIDI_CC_52,
    MIDI_CC_53,
    MIDI_CC_54,
    MIDI_CC_55,
    MIDI_CC_56,
    MIDI_CC_57,
    MIDI_CC_58,
    MIDI_CC_59,
    MIDI_CC_60,
    MIDI_CC_61,
    MIDI_CC_62,
    MIDI_CC_63,
    MIDI_CC_64,
    MIDI_CC_65,
    MIDI_CC_66,
    MIDI_CC_67,
    MIDI_CC_68,
    MIDI_CC_69,
    MIDI_CC_70,
    MIDI_CC_71,
    MIDI_CC_72,
    MIDI_CC_73,
    MIDI_CC_74,
    MIDI_CC_75,
    MIDI_CC_76,
    MIDI_CC_77,
    MIDI_CC_78,
    MIDI_CC_79,
    MIDI_CC_80,
    MIDI_CC_81,
    MIDI_CC_82,
    MIDI_CC_83,
    MIDI_CC_84,
    MIDI_CC_85,
    MIDI_CC_86,
    MIDI_CC_87,
    MIDI_CC_88,
    MIDI_CC_89,
    MIDI_CC_90,
    MIDI_CC_91,
    MIDI_CC_92,
    MIDI_CC_93,
    MIDI_CC_94,
    MIDI_CC_95,
    MIDI_CC_96,
    MIDI_CC_97,
    MIDI_CC_98,
    MIDI_CC_99,
    MIDI_CC_100,
    MIDI_CC_101,
    MIDI_CC_102,
    MIDI_CC_103,
    MIDI_CC_104,
    MIDI_CC_105,
    MIDI_CC_106,
    MIDI_CC_107,
    MIDI_CC_108,
    MIDI_CC_109,
    MIDI_CC_110,
    MIDI_CC_111,
    MIDI_CC_112,
    MIDI_CC_113,
    MIDI_CC_114,
    MIDI_CC_115,
    MIDI_CC_116,
    MIDI_CC_117,
    MIDI_CC_118,
    MIDI_CC_119,
    MIDI_CC_120,
    MIDI_CC_121,
    MIDI_CC_122,
    MIDI_CC_123,
    MIDI_CC_124,
    MIDI_CC_125,
    MIDI_CC_126,
    MIDI_CC_127,
    MIDI_CC_128,

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

// bool is_unused_cc(new_keycodes key) {
//     switch (key) {
//         case MIDI_CC_3:
//         case MIDI_CC_9:
//         case MIDI_CC_14:
//         case MIDI_CC_15:
//         case MIDI_CC_20:
//         case MIDI_CC_21:
//         case MIDI_CC_22:
//         case MIDI_CC_23:
//         case MIDI_CC_24:
//         case MIDI_CC_25:
//         case MIDI_CC_26:
//         case MIDI_CC_27:
//         case MIDI_CC_28:
//         case MIDI_CC_29:
//         case MIDI_CC_30:
//         case MIDI_CC_31:
//         case MIDI_CC_85:
//         case MIDI_CC_86:
//         case MIDI_CC_87:
//         case MIDI_CC_89:
//         case MIDI_CC_90:
//         case MIDI_CC_102:
//         case MIDI_CC_103:
//         case MIDI_CC_104:
//         case MIDI_CC_105:
//         case MIDI_CC_106:
//         case MIDI_CC_107:
//         case MIDI_CC_108:
//         case MIDI_CC_109:
//         case MIDI_CC_110:
//         case MIDI_CC_111:
//         case MIDI_CC_112:
//         case MIDI_CC_113:
//         case MIDI_CC_114:
//         case MIDI_CC_115:
//         case MIDI_CC_116:
//         case MIDI_CC_117:
//         case MIDI_CC_118:
//         case MIDI_CC_119:
//             return true;
//         default:
//             return false;
//     }
// }


// // get the int value for all new_keycodes of type MIDI_CC_# in new_keycodes
// uint16_t get_num(new_keycodes key) {
//     switch (key) {
//         case MIDI_CC_14:
//             return 14;
//         case MIDI_CC_15:
//             return 15;
//         case MIDI_CC_20:
//             return 20;
//         case MIDI_CC_21:
//             return 21;
//         case MIDI_CC_22:
//             return 22;
//         case MIDI_CC_23:
//             return 23;
//         case MIDI_CC_24:
//             return 24;
//         case MIDI_CC_25:
//             return 25;
//         case MIDI_CC_26:
//             return 26;
//         case MIDI_CC_27:
//             return 27;
//         case MIDI_CC_28:
//             return 28;
//         case MIDI_CC_29:
//             return 29;
//         case MIDI_CC_30:
//             return 30;
//         case MIDI_CC_31:
//             return 31;
//         case MIDI_CC_85:
//             return 85;
//         case MIDI_CC_86:
//             return 86;
//         case MIDI_CC_87:
//             return 87;
//         case MIDI_CC_89:
//             return 89;
//         case MIDI_CC_90:
//             return 90;
//         case MIDI_CC_102:
//             return 102;
//         case MIDI_CC_103:
//             return 103;
//         case MIDI_CC_104:
//             return 104;
//         case MIDI_CC_105:
//             return 105;
//         case MIDI_CC_106:
//             return 106;
//         case MIDI_CC_107:
//             return 107;
//         case MIDI_CC_108:
//             return 108;
//         case MIDI_CC_109:
//             return 109;
//         case MIDI_CC_110:
//             return 110;
//         case MIDI_CC_111:
//             return 111;
//         case MIDI_CC_112:
//             return 112;
//         case MIDI_CC_113:
//             return 113;
//         case MIDI_CC_114:
//             return 114;
//         case MIDI_CC_115:
//             return 115;
//         case MIDI_CC_116:
//             return 116;
//         case MIDI_CC_117:
//             return 117;
//         case MIDI_CC_118:
//             return 118;
//         case MIDI_CC_119:
//             return 119;
//         default:
//             return 0;
//     }
// }


// uint16_t get_num(new_keycodes key) {
//     return key - MIDI_CC_80 + 80;
// }


#ifdef ENCODER_MAP_ENABLE
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    // [_QWERTY] = { ENCODER_CCW_CW(MI_VELD, MI_VELU) },
    // [_LOWER]  = { ENCODER_CCW_CW(KC_PGDN, KC_PGUP) },
    // [_RAISE]  = { ENCODER_CCW_CW(KC_VOLU, KC_VOLU) },
    [_NOTES] = { ENCODER_CCW_CW(MI_VELD, MI_VELU) },
    [_SESSION]  = { ENCODER_CCW_CW(MI_CHD, MI_CHU) },
    // [_CHANNEL]  = { ENCODER_CCW_CW(MI_MODSD, MI_MODSU) },
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


// uint16_t const YAA[] = {}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_NOTES] = LAYOUT(
        //
        MI_C_4,  MI_D_4, MI_E_4, MI_F_4, MI_G_4, MI_A_4, MI_B_4, MI_C_4,  RGB_TOG, RGB_MOD,   R_M_TOG, R_M_MOD, KC_NO,
        MI_C_3,  MI_D_3, MI_E_3, MI_F_3, MI_G_3, MI_A_3, MI_B_3, MI_C_4,  MI_D_4,  MI_E_4,  MI_G_4, MI_A_4,
        MI_C_2,  MI_D_2, MI_E_2, MI_F_2, MI_G_2, MI_A_2, MI_B_2, MI_C_3,  MI_D_3,  MI_E_3,  MI_G_3, MI_A_3,
        MI_PORT, MI_SOST, MI_SOFT, MI_LEG, MI_MOD, TO(1), TO(1), TO(2),  KC_TRNS,  MI_CHD,  KC_NO, MI_CHU
    ),

    [_SESSION] = LAYOUT(
        MIDI_CC_03,   MIDI_CC_09,   MIDI_CC_14,   MIDI_CC_15,   MIDI_CC_20,   MIDI_CC_21,   MIDI_CC_22,   MIDI_CC_23,   MIDI_CC_24,    MIDI_CC_25,   MIDI_CC_26,   MIDI_CC_27,   TO(0),
        MIDI_CC_28,   MIDI_CC_29,   MIDI_CC_30,   MIDI_CC_31,   MIDI_CC_85,   MIDI_CC_86,   MIDI_CC_87,   MIDI_CC_89,   MIDI_CC_90,   MIDI_CC_102,   MIDI_CC_103,  MIDI_CC_104,
        MIDI_CC_105,  MIDI_CC_106,  MIDI_CC_107,  MIDI_CC_108,  MIDI_CC_109,  MIDI_CC_110,  MIDI_CC_111,  MIDI_CC_112,  MIDI_CC_113,  MIDI_CC_114,   MIDI_CC_115,  MIDI_CC_116,
        MIDI_CC_117,  MIDI_CC_118,  MIDI_CC_119,  MIDI_CC_120,  MIDI_CC_121,  MIDI_CC_122,  MIDI_CC_122,  MIDI_CC_123,  MIDI_CC_124,  MIDI_CC_125,  MIDI_CC_126,   MIDI_CC_127
    ),

    // [_CHANNEL] = LAYOUT(
    //     MIDI_CC_80, MIDI_CC_81, MIDI_CC_82, MIDI_CC_83, MIDI_CC_84, MIDI_CC_85, MIDI_CC_86, MIDI_CC_87, MIDI_CC_88, MIDI_CC_89, MIDI_CC_90, MIDI_CC_91, MIDI_CC_92,
    //     MIDI_CC_93, MIDI_CC_94, MIDI_CC_95, MIDI_CC_96, MIDI_CC_97, MIDI_CC_98, MIDI_CC_99, MIDI_CC_100, MIDI_CC_101, MIDI_CC_102, MIDI_CC_103, MIDI_CC_104,
    //     MIDI_CC_105, MIDI_CC_106, MIDI_CC_107, MIDI_CC_108, MIDI_CC_109, MIDI_CC_110, MIDI_CC_111, MIDI_CC_112, MIDI_CC_113, MIDI_CC_114, MIDI_CC_115, MIDI_CC_116,
    //     MIDI_CC_117, MIDI_CC_118, MIDI_CC_119, MIDI_CC_120, MIDI_CC_121,  TO(0),  TO(0), MIDI_CC_122, MIDI_CC_123, MI_CHD, MIDI_CC_125, MI_CHU
    // ),

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
// // get the index of a new_keycodes in the _SESSION LAYOUT
// uint16_t get_index(enum new_keycodes key) {
//     switch (key) {
//         case MIDI_CC_03:
//             return 0;
//         case MIDI_CC_09:
//             return 1;
//         case MIDI_CC_14:
//             return 2;
//         case MIDI_CC_15:
//             return 3;
//         case MIDI_CC_20:
//             return 4;
//         case MIDI_CC_21:
//             return 5;
//         case MIDI_CC_22:
//             return 6;
//         case MIDI_CC_23:
//             return 7;
//         case MIDI_CC_24:
//             return 8;
//         case MIDI_CC_25:
//             return 9;
//         case MIDI_CC_26:
//             return 10;
//         case MIDI_CC_27:
//             return 11;
//         case MIDI_CC_28:
//             return 13;
//         case MIDI_CC_29:
//             return 14;
//         case MIDI_CC_30:
//             return 15;
//         case MIDI_CC_31:
//             return 16;
//         case MIDI_CC_85:
//             return 17;
//         case MIDI_CC_86:
//             return 18;
//         case MIDI_CC_87:
//             return 19;
//         case MIDI_CC_89:
//             return 20;
//         case MIDI_CC_90:
//             return 21;
//         case MIDI_CC_102:
//             return 22;
//         case MIDI_CC_103:
//             return 23;
//         case MIDI_CC_104:
//             return 24;
//         case MIDI_CC_105:
//             return 25;
//         case MIDI_CC_106:
//             return 26;
//         case MIDI_CC_107:
//             return 27;
//         case MIDI_CC_108:
//             return 28;
//         case MIDI_CC_109:
//             return 29;
//         case MIDI_CC_110:
//             return 30;
//         case MIDI_CC_111:
//             return 31;
//         case MIDI_CC_112:
//             return 32;
//         case MIDI_CC_113:
//             return 33;
//         case MIDI_CC_114:
//             return 34;
//         case MIDI_CC_115:
//             return 35;
//         case MIDI_CC_116:
//             return 36;
//         case MIDI_CC_117:
//             return 37;
//         case MIDI_CC_118:
//             return 38;
//         case MIDI_CC_119:
//             return 39;
//         case MIDI_CC_120:
//             return 40;
//         case MIDI_CC_121:
//             return 41;
//         case MIDI_CC_122:
//             return 42;
//         case MIDI_CC_123:
//             return 43;
//         case MIDI_CC_124:
//             return 44;
//         case MIDI_CC_125:
//             return 45;
//         case MIDI_CC_126:
//             return 46;
//         case MIDI_CC_127:
//             return 47;
//         case MIDI_CC_128:
//             return 48;
//         default:
//             return 0;
//     }
// }


// Get the index of a new_keycodes in the _SESSION LAYOUT.
// The convention we take here is
//
// 1,  2,  3,  4, 5, 6,  7,  8,  9,  10, 11, 12,
// 13, 14, 15, 16,...
// 25, 26, 27, 28,...
// 37, 38, 39, 40,   42, 43, 44, 45, 46, 47, 48
//
// Note the absence of 41 since its behind the two button
// spacebar.
//
// Only the unreserved midi cc are used. There happens to be
// as many unreserved midi ccs as there are keys on the work
// board. This is a coincidence.

// TODO: use this format for readability
// keycode_array[48] = {
//     0, 1,  2,  3,  4,  5,  6,  7,  8,  9,  10 ,11, 12,
//     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
//     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
//     37, 38, 39, 40, 42, 43, 44, 45, 46, 47, 48
// }

uint8_t get_keycode_index(enum new_keycodes key) {
    switch (key) {
        case MIDI_CC_03:
            return 0;
        case MIDI_CC_09:
            return 1;
        case MIDI_CC_14:
            return 2;
        case MIDI_CC_15:
            return 3;
        case MIDI_CC_20:
            return 4;
        case MIDI_CC_21:
            return 5;
        case MIDI_CC_22:
            return 6;
        case MIDI_CC_23:
            return 7;
        case MIDI_CC_24:
            return 8;
        case MIDI_CC_25:
            return 9;
        case MIDI_CC_26:
            return 10;
        case MIDI_CC_27:
            return 11;
        case MIDI_CC_28:
            return 12;
        case MIDI_CC_29:
            return 13;
        case MIDI_CC_30:
            return 14;
        case MIDI_CC_31:
            return 15;
        case MIDI_CC_85:
            return 16;
        case MIDI_CC_86:
            return 17;
        case MIDI_CC_87:
            return 18;
        case MIDI_CC_89:
            return 19;
        case MIDI_CC_90:
            return 20;
        case MIDI_CC_102:
            return 21;
        case MIDI_CC_103:
            return 22;
        case MIDI_CC_104:
            return 23;
        case MIDI_CC_105:
            return 24;
        case MIDI_CC_106:
            return 25;
        case MIDI_CC_107:
            return 26;
        case MIDI_CC_108:
            return 27;
        case MIDI_CC_109:
            return 28;
        case MIDI_CC_110:
            return 29;
        case MIDI_CC_111:
            return 30;
        case MIDI_CC_112:
            return 31;
        case MIDI_CC_113:
            return 32;
        case MIDI_CC_114:
            return 33;
        case MIDI_CC_115:
            return 34;
        case MIDI_CC_116:
            return 35;
        case MIDI_CC_117:
            return 36;
        case MIDI_CC_118:
            return 37;
        case MIDI_CC_119:
            return 38;
        case MIDI_CC_120:
            return 39;
        case MIDI_CC_121:
            return 40; // no 41, since it is under spacebar
        case MIDI_CC_122:
            return 42;
        case MIDI_CC_123:
            return 43;
        case MIDI_CC_124:
            return 44;
        case MIDI_CC_125:
            return 45;
        case MIDI_CC_126:
            return 46;
        case MIDI_CC_127:
            return 47;
        case MIDI_CC_128:
            return 48;
        default:
            return 0;
    }
}

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



// This is how the individual rgb matrix LEDs are plugged in: into a sort of
// S pattern starting from the bottom right.
//
const uint8_t rgb_leds[49] = {
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    12, 11, 10,  9,  8,  7,  6,  4,  3,  2,  1,  0
};

struct clip_t {
    uint8_t channel;
    uint8_t clip;
};

// Global state for which clip is currently selected in Ableton.
// We have no way of receiving information from Ableton unfortunately,
// so we have to trust that the user is solely using the keyboard
// for controlling Ableton clips in session view. Otherwise, this
// global state will not match the Ableton session state.
//
struct clip_t clips_playing[12] = {
    {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}
};


// for a given column index, return the set of indices inside that column.
// e.g. for column 0, return {0, 12, 24, 36}
// e.g. for column 1, return {1, 13, 25, 37}
// e.g. for column 2, return {2, 14, 26, 38}
//
int8_t * get_column_indices(uint8_t column_index) {
    static int8_t column_indices[4];
    int8_t column = column_index % 12;

    for (uint8_t i = 0; i < 4; i++) {
        column_indices[i] = column + i * 12;
    }

    return column_indices;
}


// const uint16_t rgb_leds[49] = {
//     37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
//     36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25,
//     13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
//     12, 11, 10,  9,  8,  7,  6,  4,  3,  2,  1,  0
// };

// Tap Dance definitions
qk_tap_dance_action_t tap_dance_actions[] = {
    [ENC_TAP] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_enc_finished, dance_enc_reset),
};


// (uint16_t, uint16_t, uint16_t) colors[8] = {

// };

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


struct Color colorz[8] = {
    {RGB_CYAN},
    {RGB_MAGENTA},
    {RGB_ORANGE},
    {RGB_GREEN},
    {RGB_YELLOW},
    {RGB_BLUE},
    {RGB_PURPLE},
    {RGB_WHITE}
};

void write_to_led_pins(void) {
    writePinLow(B2);
    writePinLow(B3);
    writePinLow(B7);

    rgb_matrix_set_color_all(0, 0, 0);

    struct Color color = colorz[midi_config.channel];




    for (uint8_t i = 0; i < 12; i++) {
        int8_t * column_indices = get_column_indices(i);
        uint8_t channel = clips_playing[i].channel;
        uint8_t clip = clips_playing[i].clip;

        if (midi_config.channel + 1 == channel) {
            rgb_matrix_set_color(rgb_leds[column_indices[clip]], color.r, color.g, color.b);
        }

    }

    switch (midi_config.channel) {

        case 0:

            return;

        case 1:
            writePinHigh(B2);
            // rgb_matrix_set_color_all(20, 50, 200);
            return;

        case 2:
            writePinHigh(B3);
            // rgb_matrix_set_color_all(150, 165, 17);
            return;

        case 3:
            writePinHigh(B7);
            // rgb_matrix_set_color_all(150, 165, 127);
            return;

        case 4:
            writePinHigh(B2);
            writePinHigh(B3);
            // rgb_matrix_set_color_all(111, 95, 127);
            return;

        case 5:
            writePinHigh(B2);
            writePinHigh(B7);
            // rgb_matrix_set_color_all(111, 200, 127);
            return;

        case 6:
            writePinHigh(B3);
            writePinHigh(B7);
            // rgb_matrix_set_color_all(111, 200, 211);
            return;

        case 7:
            writePinHigh(B2);
            writePinHigh(B3);
            writePinHigh(B7);
            // rgb_matrix_set_color_all(175, 200, 126);
            return;
    }

    return;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    // rgb_matrix_indicators_user
    // if (keycode == USER09) {
    //     preprocess_tap_dance(TD(ENC_TAP), record);
    //     return process_tap_dance(TD(ENC_TAP), record);
    // }
        // TODO: ideally this would be generalized and refactored into
    // QMK core as advanced keycodes, until then, the simple case
    // can be available here to keyboards using VIA
    switch (keycode) {
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
        case MI_CHU:
            if (record->event.pressed) {
                midi_config.channel++;

                if (midi_config.channel > 7) {
                    midi_config.channel = 0;
                }
                write_to_led_pins();
            }
            return false;
        case MI_CHD:
            if (record->event.pressed) {
                midi_config.channel--;
                if (midi_config.channel < 0) {
                    midi_config.channel = 15;
                }
                write_to_led_pins();
            }
            return false;

        // case MIDI_CC_80:
        //     if (record->event.pressed) {
        //         midi_send_cc(&midi_device, midi_config.channel, 80, MIDI_CC_ON);
        //         rgb_matrix_set_color_all(50, 200, 25);
        //     } else {
        //         midi_send_cc(&midi_device, midi_config.channel, 80, MIDI_CC_OFF);
        //         rgb_matrix_set_color_all(20, 50, 200);
        //     }
        //     return true;
        // case MIDI_CC_81:
        //     if (record->event.pressed) {
        //         midi_send_cc(&midi_device, midi_config.channel, 81, MIDI_CC_ON);
        //         rgb_matrix_set_color_all(50, 200, 25);
        //     } else {
        //         midi_send_cc(&midi_device, midi_config.channel, 81, MIDI_CC_OFF);
        //         rgb_matrix_set_color_all(20, 50, 200);
        //     }
        //     return true;

        case MIDI_CC_01 ... MIDI_CC_128:
            if (record->event.pressed) {
                midi_send_cc(&midi_device, midi_config.channel, keycode - MIDI_CC_01 + 1, MIDI_CC_ON);

                uint8_t keycode_index  = get_keycode_index(keycode);
                uint8_t led_index = rgb_leds[keycode_index];

                int8_t column = keycode_index % 12;
                int8_t row = keycode_index / 12;

                bool already_playing = (clips_playing[column].channel == (midi_config.channel + 1)) && (clips_playing[column].clip == row);

                // update global clips state. A row if 0 means, nothing is playing.
                // Rows 1-4 are the clips
                //
                if (already_playing) {
                    clips_playing[column] = (struct clip_t){0, 0};
                } else {
                    clips_playing[column] = (struct clip_t){midi_config.channel + 1, row};
                }


                // set all leds in the column to black
                int8_t * column_indices = get_column_indices(keycode_index);
                for (uint8_t i = 0; i < 4; i++) {
                    rgb_matrix_set_color(rgb_leds[column_indices[i]], 0, 0, 0);
                }


                struct Color color = colorz[midi_config.channel];

                if (!already_playing) {
                    rgb_matrix_set_color(led_index, color.r, color.g, color.b);
                }

                // rgb_matrix_set_color(led_index, color.r, color.g, color.b);



                // rgb_matrix_set_color(0, 255, 26, 15);
                // rgb_matrix_set_color(12, 255, 26, 15);
                // rgb_matrix_set_color(38, 255, 26, 15);
                // rgb_matrix_set_color(49, 255, 26, 15);
            } else {
                midi_send_cc(&midi_device, midi_config.channel, keycode - MIDI_CC_01 + 1, MIDI_CC_OFF);
            }
            return true;

        // case MI_CHD:
        // case MI_CHU:
        //     if (record->event.pressed && midi_config.channel > 0) {
        //         switch (midi_config.channel) {
        //             case 0:
        //                 writePinHigh(B2);
        //             case 1:
        //                 writePinHigh(B3);
        //             case 2:
        //                 writePinHigh(B7);
        //         }
        //     }
        //     return false;

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


