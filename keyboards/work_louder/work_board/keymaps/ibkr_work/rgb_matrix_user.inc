// IBKR Work keymap - Custom RGB Matrix effects
// We handle RGB control in keymap.c via rgb_matrix_indicators_user()
// This file is required by the Work Board's RGB_MATRIX_CUSTOM_USER setting

RGB_MATRIX_EFFECT(ibkr_work_effect)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

// Simple pass-through effect - actual RGB control is in keymap.c
static bool ibkr_work_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);
    // Return true to indicate the effect is handled
    // Actual LED control happens in rgb_matrix_indicators_user() in keymap.c
    return rgb_matrix_check_finished_leds(led_max);
}

#endif