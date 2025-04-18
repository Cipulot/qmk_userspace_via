/* Copyright 2022 ryanbaekr
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

enum custom_keycodes {
    M2 = SAFE_RANGE,
    M3,
    M4,
    M5,
    M6,
    M7,
    M8,
    M9,
    M0
};

bool remember_last_key_user(uint16_t keycode, keyrecord_t* record, uint8_t* remembered_mods) {
    if (keycode >= M2 && keycode <= M0) {
        return false;
    }
    return true;
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= M2 && keycode <= M0) {
        if (record->event.pressed) {
            for (int i = M2 - 1; i <= keycode; i++) {
                tap_code16(get_last_keycode());
            }
        }
    }
    return true;
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_ESC,           KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_PSCR, KC_SCRL, KC_PAUS,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,           KC_BSPC, KC_INS,  KC_HOME,
        KC_TAB,           KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_DEL,  KC_END,
        MO(1),            KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,           KC_PGUP, KC_PGDN,
                 KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT,          KC_UP,   
        KC_LCTL,          KC_LGUI, KC_LALT,                            KC_SPC,                    KC_RALT,          KC_CAPS, KC_RCTL, KC_LEFT, KC_DOWN, KC_RGHT
    ),

    [1] = LAYOUT(
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______, QK_REP,  M2,      M3,      M4,      M5,      M6,      M7,      M8,      M9,      M0,      _______, _______,          _______, _______, _______,
        _______,          _______, _______, KC_END,  _______, _______, _______, _______, _______, _______, KC_UP,   _______, _______, _______, _______, _______,
        _______,          KC_HOME, _______, KC_DEL,  KC_RGHT, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
                 _______, _______, _______, _______, _______, KC_LEFT, KC_DOWN, _______, _______, _______, _______,          _______,          _______, 
        _______,          _______, _______,                            _______,                   _______,          _______, _______, _______, _______, _______
    )
};
