/* Copyright 2025 Cipulot
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

#include "eeprom_tools.h"
#include "mx_wk.h"
#include "print.h"
#include "via.h"

#ifdef VIA_ENABLE

// Declaring enums for VIA config menu
enum via_enums {
    // clang-format off
    id_ind1_enabled = 1,
    id_ind1_brightness = 2,
    id_ind1_color = 3,
    id_ind1_func1 = 4,
    id_ind2_enabled = 5,
    id_ind2_brightness = 6,
    id_ind2_color = 7,
    id_ind2_func1 = 8
    // clang-format on
};

int indi_index;
int data_index;

// Handle the data received by the keyboard from the VIA menus
void via_config_set_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    indi_index                            = ((int)(*value_id) - 1) / 4;
    data_index                            = (int)(*value_id) - indi_index * 4;
    indicator_config *current_indicator_p = get_indicator_p(indi_index);
    uprintf("--> value_id: %X\n", (int)(*value_id));

    switch (data_index) {
        case 1: {
            current_indicator_p->enabled = value_data[0];
            if (indi_index == 0) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind1.enabled);
            } else if (indi_index == 1) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind2.enabled);
            }
            break;
        }
        case 2: {
            current_indicator_p->v = value_data[0];
            if (indi_index == 0) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind1.v);
            } else if (indi_index == 1) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind2.v);
            }
            break;
        }
        case 3: {
            current_indicator_p->h = value_data[0];
            current_indicator_p->s = value_data[1];
            if (indi_index == 0) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind1.h);
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind1.s);
            } else if (indi_index == 1) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind2.h);
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind2.s);
            }
            break;
        }
        case 4: {
            current_indicator_p->func = (current_indicator_p->func & 0xF0) | (uint8_t)value_data[0];
            if (indi_index == 0) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind1.func);
            } else if (indi_index == 1) {
                EEPROM_KB_PARTIAL_UPDATE(eeprom_mx_wk_config, ind2.func);
            }
            break;
        }
        default: {
            // Unhandled value.
            break;
        }
    }
    indicators_callback();
}

// Handle the data sent by the keyboard to the VIA menus
void via_config_get_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    indi_index                            = ((int)(*value_id) - 1) / 4;
    data_index                            = (int)(*value_id) - indi_index * 4;
    indicator_config *current_indicator_p = get_indicator_p(indi_index);

    switch (data_index) {
        case 1: {
            value_data[0] = current_indicator_p->enabled;
            break;
        }
        case 2: {
            value_data[0] = current_indicator_p->v;
            break;
        }
        case 3: {
            value_data[0] = current_indicator_p->h;
            value_data[1] = current_indicator_p->s;
            break;
        }
        case 4: {
            value_data[0] = current_indicator_p->func & 0x0F;
            uprintf("--> Current func: %x, current func with bitwise or: %X\n", current_indicator_p->func, current_indicator_p->func % 0x0F);
            break;
        }
        default: {
            // Unhandled value.
            break;
        }
    }
}

// Handle the commands sent and received by the keyboard with VIA
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value: {
                via_config_set_value(value_id_and_data);
                break;
            }
            case id_custom_get_value: {
                via_config_get_value(value_id_and_data);
                break;
            }
            case id_custom_save: {
                // Bypass the save function in favor of pinpointed saves
                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }
        return;
    }

    *command_id = id_unhandled;
}

#endif // VIA_ENABLE
