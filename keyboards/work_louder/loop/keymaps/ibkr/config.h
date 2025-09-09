/* Copyright 2025 IBKR Trading Keymap
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

// Encoder Configuration
#define ENCODER_RESOLUTION 2  // Lower = more sensitive

// RGB Configuration for layer indication
#ifdef RGB_MATRIX_ENABLE
    #undef RGB_MATRIX_STARTUP_MODE
    #define RGB_MATRIX_STARTUP_MODE RGB_MATRIX_SOLID_COLOR
    #define RGB_MATRIX_STARTUP_HUE 0
    #define RGB_MATRIX_STARTUP_SAT 0
    #define RGB_MATRIX_STARTUP_VAL 150
    #define RGB_DISABLE_WHEN_USB_SUSPENDED
#endif

// Tapping Configuration
#define TAPPING_TERM 175
#define PERMISSIVE_HOLD

// Debounce for stability during rapid trading
#define DEBOUNCE 5