/* Copyright 2021 rate
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
#include QMK_KEYBOARD_H

#include <stdio.h>

#ifdef USE_BME280
 #include "lib/bme280.h"
#endif


const char code_to_name[60] = {
    ' ', ' ', ' ', ' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    'R', 'E', 'B', 'T', '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`', ',', '.', '/', ' ', ' ', ' '};

void set_keylog(uint16_t keycode, keyrecord_t *record);
#ifdef USE_BME280
void print_airstate(void);
#endif

extern rgblight_config_t rgblight_config;
int RGB_current_mode;

// Defines names for use in layer keycodes and the keymap
enum layer_names {
    _BASE,
    _FN
};


enum custom_keycodes {
  RGB_RST = SAFE_RANGE
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
    [_BASE] = LAYOUT(
        RGB_VAI,  RGB_MODE_FORWARD,
        RGB_VAD,  RGB_MODE_REVERSE
    ),
    [_FN] = LAYOUT(
        RGB_HUI,  RGB_MODE_FORWARD,
        RGB_HUD,  RGB_MODE_REVERSE
    )
};

#ifdef USE_BME280
void keyboard_post_init_user(void) {
    bme280_init();
}

void housekeeping_task_user(void) {
    bme280_exec();
}

void print_airstate(void) {
    char airstate_str[32] = {};
    double temp;
    double press;
    double hum;
    temp = bme280_getTemp();
    press = bme280_getPress();
    hum = bme280_getHum();

    snprintf(airstate_str, sizeof(airstate_str), "%d,  %d,  %d\n", (int)temp, (int)press, (int)hum );
    oled_write(airstate_str, false);
}
#endif

void oled_render_layer_state(void) {;}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (record->event.pressed) {
    set_keylog(keycode, record);
  }

  return true;
}

char keylog_str[32] = {};
void set_keylog(uint16_t keycode, keyrecord_t *record) {
  char name = ' ';
    if ((keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) ||
        (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX)) { keycode = keycode & 0xFF; }
  if (keycode < 60) {
    name = code_to_name[keycode];
  }

  // update keylog
  snprintf(keylog_str, sizeof(keylog_str), "%dx%d, k%2d : %c",
           record->event.key.row, record->event.key.col,
           keycode, name);
}

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

void oled_task_user(void) {
    oled_write_ln_P(PSTR("Hello TrialBoard!"), false);

#ifdef USE_BME280
    print_airstate();
#endif
    oled_write(keylog_str, false);
}



