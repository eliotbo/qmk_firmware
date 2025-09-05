RGB_MATRIX_EFFECT(my_cool_effect)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

const uint8_t rgb_leds2[49] = {
    37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    12, 11, 10,  9,  8,  7,  6,  4,  3,  2,  1,  0
};


void apply_rgb_notes2(void) {
    for ( int i = 0; i < 4; i++ ) {
        for (int j = 0; j < 8; j++) {
            rgb_matrix_set_color(rgb_leds2[i*12 + j], RGB_PINK);
        }
        for (int j = 8; j < 12; j++) {
            rgb_matrix_set_color(rgb_leds2[i*12 + j], RGB_OFF);
        }
    }
}

static void my_cool_effect2_complex_init(effect_params_t* params) {
//   rgb_matrix_set_color_all(RGB_PINK);
  apply_rgb_notes2();
}

// e.g: A simple effect, self-contained within a single method
static bool my_cool_effect(effect_params_t* params) {
  RGB_MATRIX_USE_LIMITS(led_min, led_max);

  if (params->init) my_cool_effect2_complex_init(params);

//   for (uint8_t i = led_min; i < led_max; i++) {
    // rgb_matrix_set_color(i, 0xff, 0xff, 0x00);
//   }
  return rgb_matrix_check_finished_leds(led_max);
}

#endif
