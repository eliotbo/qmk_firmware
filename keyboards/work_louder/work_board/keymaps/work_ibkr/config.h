/* Copyright 2025 Work IBKR Keymap Config
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

#pragma once

// MIDI Configuration
#define MIDI_BASIC
#define MIDI_ADVANCED

// Encoder resolution (optional - adjust if encoder feels too sensitive or not sensitive enough)
// #define ENCODER_RESOLUTION 4

// RGB Matrix Configuration
#ifdef RGB_MATRIX_ENABLE
    // Set default brightness
    #define RGB_MATRIX_STARTUP_VAL 128  // Max is 255
    
    // Set default mode to solid color
    #define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_SOLID_COLOR
    
    // Disable some RGB effects to save space if needed
    #undef ENABLE_RGB_MATRIX_ALPHAS_MODS
    #undef ENABLE_RGB_MATRIX_GRADIENT_UP_DOWN
    #undef ENABLE_RGB_MATRIX_GRADIENT_LEFT_RIGHT
    #undef ENABLE_RGB_MATRIX_BREATHING
    #undef ENABLE_RGB_MATRIX_BAND_SAT
    #undef ENABLE_RGB_MATRIX_BAND_VAL
#endif

// Tapping term for any tap dance or mod-tap features (if needed later)
#define TAPPING_TERM 200

// Prevent stuck modifiers
#define PREVENT_STUCK_MODIFIERS

// USB polling rate (1000Hz)
#define USB_POLLING_INTERVAL_MS 1