/* Copyright 2023 Cipulot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // clang-format off
    [0] = LAYOUT(
        KC_ESC,  _______,  _______,  MO(1),
        KC_Z,    KC_X,     KC_C,     KC_SPACE),

    [1] = LAYOUT(
        KC_VOLD,  KC_VOLU,  KC_MUTE,  _______,
        _______,  _______,  _______,  MO(2)),

    [2] = LAYOUT(
        QK_BOOT,  NK_TOGG,  _______,  _______,
        _______,  _______,  _______,  _______)
    // clang-format on
};