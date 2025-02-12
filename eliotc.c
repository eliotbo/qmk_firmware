#include QMK_KEYBOARD_H


/* THIS FILE WAS GENERATED!
 *
 * This file was generated by qmk json2c. You may or may not want to
 * edit it directly.
 */

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
	[0] = LAYOUT(KC_ESC, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC, USER09, KC_TAB, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT, TO(5), KC_LCTL, KC_LALT, FN_MO23, FN_MO13, KC_SPC, KC_SPC, FN_MO13, FN_MO23, KC_LEFT, KC_DOWN, KC_RGHT),
	[1] = LAYOUT(KC_TILD, KC_EXLM, KC_AT, KC_HASH, KC_DLR, KC_PERC, KC_CIRC, KC_AMPR, KC_ASTR, KC_UNDS, KC_PEQL, KC_DEL, KC_TRNS, KC_GRV, KC_LBRC, KC_RBRC, KC_LT, KC_GT, KC_LPRN, KC_RPRN, KC_LCBR, KC_RCBR, KC_COLN, KC_PMNS, KC_ENT, KC_CAPS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP, KC_BSLS, TO(0), KC_LGUI, KC_TRNS, KC_TRNS, KC_TRNS, KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_LEFT, KC_DOWN, KC_RGHT),
	[2] = LAYOUT(KC_GRV, KC_PSLS, KC_PAST, KC_PMNS, KC_PSCR, KC_HOME, KC_END, KC_P7, KC_P8, KC_P9, KC_PMNS, KC_DEL, KC_TRNS, KC_DEL, KC_PEQL, KC_CIRC, KC_PPLS, KC_NO, KC_DEL, KC_PGUP, KC_P4, KC_P5, KC_P6, KC_PPLS, KC_ENT, MACRO01, KC_NO, KC_NO, KC_NO, KC_NO, KC_DEL, KC_PGDN, KC_P1, KC_P2, KC_P3, KC_UP, KC_ENT, TO(0), KC_NO, KC_TRNS, KC_TRNS, KC_TRNS, KC_SPC, KC_TRNS, KC_TRNS, KC_P0, KC_PDOT, KC_DOWN, KC_RGHT),
	[3] = LAYOUT(KC_A, QK_BOOT, KC_NO, RGB_TOG, RGB_MOD, RGB_HUI, RGB_HUD, RGB_SAI, RGB_SAD, RGB_VAI, RGB_VAD, KC_DEL, R_M_TOG, KC_NO, KC_NO, MU_MOD, R_M_TOG, R_M_MOD, R_M_HUI, R_M_HUD, R_M_SAI, R_M_SAD, R_M_VAI, R_M_VAD, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, TO(0), KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO),
	[4] = LAYOUT(KC_B, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MACRO04, MACRO05, USER00, KC_TRNS, MACRO07, KC_TRNS, KC_TRNS, MACRO02, MACRO03, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MACRO06, KC_ENT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_UP, KC_ENT, TO(0), KC_TRNS, KC_TRNS, TG(2), KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TO(0), KC_LEFT, KC_DOWN, RESET),
	[5] = LAYOUT(TO(0), TO(1), TO(2), TO(3), TO(4), TO(5), KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC, KC_NO, KC_TAB, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_QUOT, KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_UP, KC_ENT, TO(0), KC_LCTL, KC_LALT, KC_LGUI, MO(_LOWER), KC_SPC, KC_SPC, MO(_RAISE), KC_SLSH, KC_LEFT, KC_DOWN, KC_RGHT)
};

    // [_ABLETON] = LAYOUT(
    //     MI_C_4, MI_D_4, MI_E_4, MI_F_4, MI_G_4, MI_A_4, MI_B_4, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    //     MI_C_5, MI_D_5, MI_E_5, MI_F_5, MI_G_5, MI_A_5, MI_B_5, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    //     KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
    //     TO(0),   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS
    // )
