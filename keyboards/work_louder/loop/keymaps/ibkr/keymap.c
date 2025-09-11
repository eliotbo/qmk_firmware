#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include <string.h>

#ifdef MIDI_ENABLE
#include "midi.h"
#include "qmk_midi.h"
extern MidiDevice midi_device;
#endif

/* IBKR Trading Keyboard - RAW HID Protocol v1
 * ============================================
 * 
 * Physical Layout (3 encoders + 1x9 grid keys ):
 *  [Enc0] [Enc1] [Enc2] [0] [1] [2] [3] [4] [5] [6] [7] [8]
 *  
 *
 * Communication: RAW HID Protocol
 * - NO F-key emissions (clean HID events only)
 * - Button events: [0x10, layer, idx, press/release]
 * - Encoder rotation: [0x11, enc_idx, delta]
 * - Encoder press: [0x12, enc_idx, 1] (always simple press)
 * - Layer changes: [0x13, layer]
 * - Boot hello: [0x7E, proto_ver, fw_major, fw_minor]
 *
 * Host Commands:
 * - Set all LEDs: [0x01, r, g, b]
 * - Set one LED: [0x02, led_idx, r, g, b]
 * - Set layer: [0x03, layer]
 * - Host ready: [0x7D, proto_ver]
 *
 * Layers:
 * - 0: BASE (White LEDs, host-controllable)
 * - 1: BUY (Green LEDs)
 * - 2: SELL (Red LEDs)
 *
 * See docs/RAW_HID_PROTOCOL.md for full specification
 */

// MIDI CC definitions for encoders
#define CC_SHARES_DOWN  20
#define CC_SHARES_UP    21
#define CC_STOP_DOWN    22
#define CC_STOP_UP      23
#define CC_LIMIT_DOWN   24
#define CC_LIMIT_UP     25

// MIDI CC values
#define MIDI_CC_OFF     0
#define MIDI_CC_ON      127

// Raw HID protocol definitions
enum hid_device_to_host {
    EV_BTN      = 0x10,  // [0x10, layer, btn_idx, act] where act=1 press, 0 release
    EV_ENC      = 0x11,  // [0x11, enc_idx, delta] (+1 / -1 per detent)
    EV_ENC_P    = 0x12,  // [0x12, enc_idx, 1] (always simple press)
    EV_LAYER    = 0x13,  // [0x13, layer] (sent on layer changes)
    EV_HELLO    = 0x7E,  // [0x7E, proto_ver=1, fw_major, fw_minor] (sent on boot)
};

enum hid_host_to_device {
    CMD_SET_ALL     = 0x01,  // [0x01, r, g, b]
    CMD_SET_ONE     = 0x02,  // [0x02, led_index, r, g, b]
    CMD_SET_MODE    = 0x03,  // [0x03, mode] (e.g., 0=BASE,1=BUY,2=SELL)
    CMD_HOST_READY  = 0x7D,  // [0x7D, proto_ver=1] → device sets host_ready=true
};

// Store individual LED colors when in HID override mode
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_color_t;

// Store the "all LEDs" color when using SET_ALL
led_color_t hid_all_color = {0, 0, 0};


led_color_t hid_led_colors[9] = {{0}};  // Store colors for 9 LEDs
bool hid_individual_leds = false;  // Track if we're using individual LED control


// Track if we're in HID control mode (only affects BASE layer)
bool hid_rgb_override = false;

enum layers {
    _BASE,   // Layer 0: Base trading functions
    _BUY,    // Layer 1: Buy-specific actions
    _SELL    // Layer 2: Sell-specific actions
};

// Track current layer for RGB indication and MIDI channel
uint8_t current_layer = _BASE;

// Host readiness tracking
static bool host_ready = false;

// RAW HID send helper
static inline void hid_send(uint8_t id, const uint8_t *payload, uint8_t n) {
    uint8_t buf[RAW_EPSIZE] = {0};
    buf[0] = id;
    if (n > RAW_EPSIZE-1) n = RAW_EPSIZE-1;
    memcpy(&buf[1], payload, n);
    raw_hid_send(buf, RAW_EPSIZE);
}

// Send button event via HID
static void send_btn_event(uint8_t idx, bool pressed) {
    uint8_t p[3] = { current_layer, idx, pressed ? 1 : 0 };
    hid_send(EV_BTN, p, sizeof p);
}

// No F-key definitions needed - using pure HID protocol

enum custom_keycodes {
    // Base layer actions
    BASE_1 = SAFE_RANGE,
    BASE_2,
    BASE_3,
    BASE_4,
    BASE_5,
    BASE_6,
    BASE_7,
    BASE_8,
    BASE_9,

    // Buy layer actions (using Alt+Shift+F13-F20)
    BUY_1,
    BUY_2,
    BUY_3,
    BUY_4,
    BUY_5,
    BUY_6,
    BUY_7,
    BUY_8,
    BUY_9,
    
    // Sell layer actions (using Ctrl+Alt+F13-F20)
    SELL_1,
    SELL_2,
    SELL_3,
    SELL_4,
    SELL_5,
    SELL_6,
    SELL_7,
    SELL_8,
    SELL_9,
    
    // Encoder press actions
    ENC0_PRESS,  // Quick share presets
    ENC1_PRESS,  // Toggle stop type
    ENC2_PRESS   // Toggle order type
};

// Track encoder states for acceleration
uint8_t encoder_value[3] = {64, 64, 64};  // Start at midpoint
bool encoder_shift_mode = false;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base Layer
     * Common trading functions
     */
    [_BASE] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        BASE_1,     BASE_2,     BASE_3,
        BASE_4,     BASE_5,     BASE_6,
        BASE_7,     BASE_8,     BASE_9
    ),
    
    /* Buy Layer
     * Buy-specific actions
     */
    [_BUY] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        BUY_1,      BUY_2,      BUY_3,
        BUY_4,      BUY_5,      BUY_6,
        BUY_7,      BUY_8,      BUY_9
    ),
    
    /* Sell Layer
     * Sell-specific actions
     */
    [_SELL] = LAYOUT(
        ENC0_PRESS, ENC1_PRESS, ENC2_PRESS,
        SELL_1,     SELL_2,     SELL_3,
        SELL_4,     SELL_5,     SELL_6,
        SELL_7,     SELL_8,     SELL_9
    )
};

// Encoder handling via RAW HID
bool encoder_update_user(uint8_t index, bool clockwise) {
    // Send encoder event via HID
    int8_t d = clockwise ? +1 : -1;
    uint8_t p[2] = { index, (uint8_t)d };
    hid_send(EV_ENC, p, sizeof p);
    
#ifdef MIDI_ENABLE
    // Optional MIDI support if enabled
    uint8_t midi_channel = current_layer;  // 0 for BASE, 1 for BUY, 2 for SELL
    
    switch(index) {
        case 0:  // Share quantity encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_SHARES_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_SHARES_DOWN, MIDI_CC_ON);
            }
            break;
            
        case 1:  // Stop loss encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_STOP_DOWN, MIDI_CC_ON);
            }
            break;
            
        case 2:  // Limit price encoder
            if (clockwise) {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_UP, MIDI_CC_ON);
            } else {
                midi_send_cc(&midi_device, midi_channel, CC_LIMIT_DOWN, MIDI_CC_ON);
            }
            break;
    }
#endif
    
    return false;  // Don't process further
}

// Raw HID receive handler for external LED control and host handshake
void raw_hid_receive(uint8_t *data, uint8_t length) {
    switch (data[0]) {
        case CMD_HOST_READY:
            if (length >= 2 && data[1] == 1) {
                host_ready = true;
            }
            break;
            
        case CMD_SET_ALL:
            if (length >= 4 && current_layer == _BASE) {
                hid_rgb_override = true;
                hid_individual_leds = false;  // We're using SET_ALL, not individual
                hid_all_color.r = data[1];
                hid_all_color.g = data[2];
                hid_all_color.b = data[3];
                // Reset individual LED colors when using SET_ALL
                for (uint8_t i = 0; i < 9; i++) {
                    hid_led_colors[i] = hid_all_color;
                }
            }
            break;

        case CMD_SET_ONE:
            if (length >= 5 && current_layer == _BASE) {
                hid_rgb_override = true;
                hid_individual_leds = true;
                uint8_t led_index = data[1];
                if (led_index < 9) {
                    hid_led_colors[led_index].r = data[2];
                    hid_led_colors[led_index].g = data[3];
                    hid_led_colors[led_index].b = data[4];
                }
            }
            break;

        case CMD_SET_MODE:
            if (length >= 2) {
                uint8_t mode = data[1];
                if (mode <= 2) {
                    layer_move(mode);
                    // Only clear HID override when leaving BASE layer
                    if (mode != _BASE) {
                        hid_rgb_override = false;
                    }
                }
            }
            break;

    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Check if shift is held for encoder acceleration
    if (record->event.pressed) {
        if (get_mods() & MOD_MASK_SHIFT) {
            encoder_shift_mode = true;
        } else {
            encoder_shift_mode = false;
        }
    }
    
    switch (keycode) {
        // Base layer keys - Send HID events only
        case BASE_1:
            send_btn_event(0, record->event.pressed);
            return false;
        case BASE_2:
            send_btn_event(1, record->event.pressed);
            return false;
        case BASE_3:
            send_btn_event(2, record->event.pressed);
            return false;
        case BASE_4:
            send_btn_event(3, record->event.pressed);
            return false;
        case BASE_5:
            send_btn_event(4, record->event.pressed);
            return false;
        case BASE_6:
            send_btn_event(5, record->event.pressed);
            return false;
        case BASE_7:
            send_btn_event(6, record->event.pressed);
            return false;
        case BASE_8:
            send_btn_event(7, record->event.pressed);
            return false;
        case BASE_9:
            send_btn_event(8, record->event.pressed);
            return false;
            
        // Buy layer keys - Send HID events only
        case BUY_1:
            send_btn_event(0, record->event.pressed);
            return false;
        case BUY_2:
            send_btn_event(1, record->event.pressed);
            return false;
        case BUY_3:
            send_btn_event(2, record->event.pressed);
            return false;
        case BUY_4:
            send_btn_event(3, record->event.pressed);
            return false;
        case BUY_5:
            send_btn_event(4, record->event.pressed);
            return false;
        case BUY_6:
            send_btn_event(5, record->event.pressed);
            return false;
        case BUY_7:
            send_btn_event(6, record->event.pressed);
            return false;
        case BUY_8:
            send_btn_event(7, record->event.pressed);
            return false;
        case BUY_9:
            send_btn_event(8, record->event.pressed);
            return false;

        // Sell layer keys - Send HID events only
        case SELL_1:
            send_btn_event(0, record->event.pressed);
            return false;
        case SELL_2:
            send_btn_event(1, record->event.pressed);
            return false;
        case SELL_3:
            send_btn_event(2, record->event.pressed);
            return false;
        case SELL_4:
            send_btn_event(3, record->event.pressed);
            return false;
        case SELL_5:
            send_btn_event(4, record->event.pressed);
            return false;
        case SELL_6:
            send_btn_event(5, record->event.pressed);
            return false;
        case SELL_7:
            send_btn_event(6, record->event.pressed);
            return false;
        case SELL_8:
            send_btn_event(7, record->event.pressed);
            return false;
        case SELL_9:
            send_btn_event(8, record->event.pressed);
            return false;

        // Encoder press actions - Send simple HID events (no long press detection)
        case ENC0_PRESS:
            if (!record->event.pressed) {
                // Only send on release (simple press)
                uint8_t p[2] = { 0, 1 };  // encoder 0, simple press (kind=1)
                hid_send(EV_ENC_P, p, sizeof p);
            }
            return false;
            
        case ENC1_PRESS:
            if (!record->event.pressed) {
                // Only send on release (simple press)
                uint8_t p[2] = { 1, 1 };  // encoder 1, simple press (kind=1)
                hid_send(EV_ENC_P, p, sizeof p);
            }
            return false;
            
        case ENC2_PRESS:
            if (!record->event.pressed) {
                // Only send on release (simple press)
                uint8_t p[2] = { 2, 1 };  // encoder 2, simple press (kind=1)
                hid_send(EV_ENC_P, p, sizeof p);
            }
            return false;
    }
    
    return true;
}

// Set RGB colors based on current layer and send layer event
layer_state_t layer_state_set_user(layer_state_t state) {
    current_layer = get_highest_layer(state);
    
    // Send layer change event via HID
    uint8_t p[1] = { current_layer };
    hid_send(EV_LAYER, p, sizeof p);
    
    // Clear HID override when leaving BASE layer
    if (current_layer != _BASE) {
        hid_rgb_override = false;
    }
    
    switch (current_layer) {
        case 0:
            // White for base layer (unless overridden by HID)
            if (!hid_rgb_override) {
                rgb_matrix_set_color_all(RGB_WHITE);
            }
            break;
        case 1:
            // Green for buy layer
            rgb_matrix_set_color_all(RGB_GREEN);
            break;
        case 2:
            // Red for sell layer
            rgb_matrix_set_color_all(RGB_RED);
            break;
    }
    
    return state;
}

// Initialize RGB on startup and send hello message
void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_set_color_all(RGB_WHITE);
    
    // Announce ourselves via HID
    uint8_t hello[3] = {1 /*proto*/, 1 /*fw_major*/, 0 /*fw_minor*/};
    hid_send(EV_HELLO, hello, sizeof hello);
}

void rgb_matrix_indicators_user(void) {
    if (hid_rgb_override) {
        if (hid_individual_leds) {
            for (uint8_t i = 0; i < 9; i++) {
                rgb_matrix_set_color(i, hid_led_colors[i].r, hid_led_colors[i].g, hid_led_colors[i].b);
              }
          } else {
              rgb_matrix_set_color_all(hid_all_color.r, hid_all_color.g, hid_all_color.b);
          }
          return;
      }

      // Only apply layer colors when NOT in HID override mode
      switch (current_layer) {
          case 0:
              rgb_matrix_set_color_all(RGB_WHITE);
              break;
          case 1:
              rgb_matrix_set_color_all(RGB_GREEN);
              break;
          case 2:
              rgb_matrix_set_color_all(RGB_RED);
              break;
      }
  }
