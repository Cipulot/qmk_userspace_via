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

#include "mx_wk.h"
#include "socd_cleaner.h"
#include "print.h"
#include "via.h"

#ifdef VIA_ENABLE

uint16_t socd_pair_handler(bool mode, uint8_t pair_idx, uint8_t field, uint16_t value);

// Declaring enums for VIA config menu
enum via_enums {
    // clang-format off
    id_ind1_enabled = 1,
    id_ind1_brightness = 2,
    id_ind1_color = 3,
    id_ind1_func = 4,
    id_ind2_enabled = 5,
    id_ind2_brightness = 6,
    id_ind2_color = 7,
    id_ind2_func = 8,
    id_socd_pair_1_enabled = 9,
    id_socd_pair_1_key_1 = 10,
    id_socd_pair_1_key_2 = 11,
    id_socd_pair_1_mode = 12,
    id_socd_pair_2_enabled = 13,
    id_socd_pair_2_key_1 = 14,
    id_socd_pair_2_key_2 = 15,
    id_socd_pair_2_mode = 16,
    id_socd_pair_3_enabled = 17,
    id_socd_pair_3_key_1 = 18,
    id_socd_pair_3_key_2 = 19,
    id_socd_pair_3_mode = 20,
    id_socd_pair_4_enabled = 21,
    id_socd_pair_4_key_1 = 22,
    id_socd_pair_4_key_2 = 23,
    id_socd_pair_4_mode = 24
    // clang-format on
};

int indi_index;
int data_index;

// Handle the data received by the keyboard from the VIA menus
void via_config_set_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    if ((*value_id) < id_socd_pair_1_enabled) {
        indi_index                            = ((int)(*value_id) - 1) / 4;
        data_index                            = (int)(*value_id) - indi_index * 4;
        indicator_config *current_indicator_p = get_indicator_p(indi_index);

        switch (data_index) {
            case 1: {
                current_indicator_p->enabled = value_data[0];
                if (indi_index == 0) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind1.enabled);
                } else if (indi_index == 1) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind2.enabled);
                }
                break;
            }
            case 2: {
                current_indicator_p->v = value_data[0];
                if (indi_index == 0) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind1.v);
                } else if (indi_index == 1) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind2.v);
                }
                break;
            }
            case 3: {
                current_indicator_p->h = value_data[0];
                current_indicator_p->s = value_data[1];
                if (indi_index == 0) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind1.h);
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind1.s);
                } else if (indi_index == 1) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind2.h);
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind2.s);
                }
                break;
            }
            case 4: {
                current_indicator_p->func = (current_indicator_p->func & 0xF0) | (uint8_t)value_data[0];
                if (indi_index == 0) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind1.func);
                } else if (indi_index == 1) {
                    eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, ind2.func);
                }
                break;
            }
            default: {
                // Unhandled value.
                break;
            }
        }
        indicators_callback();
    } else {
        switch (*value_id) {
            case id_socd_pair_1_enabled:
                socd_pair_handler(1, 0, 0, value_data[0]);
                break;
            case id_socd_pair_1_key_1:
                socd_pair_handler(1, 0, 1, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_1_key_2:
                socd_pair_handler(1, 0, 2, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_1_mode:
                socd_pair_handler(1, 0, 3, value_data[0]);
                break;
            case id_socd_pair_2_enabled:
                socd_pair_handler(1, 1, 0, value_data[0]);
                break;
            case id_socd_pair_2_key_1:
                socd_pair_handler(1, 1, 1, value_data[0]);
                break;
            case id_socd_pair_2_key_2:
                socd_pair_handler(1, 1, 2, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_2_mode:
                socd_pair_handler(1, 1, 3, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_3_enabled:
                socd_pair_handler(1, 2, 0, value_data[0]);
                break;
            case id_socd_pair_3_key_1:
                socd_pair_handler(1, 2, 1, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_3_key_2:
                socd_pair_handler(1, 2, 2, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_3_mode:
                socd_pair_handler(1, 2, 3, value_data[0]);
                break;
            case id_socd_pair_4_enabled:
                socd_pair_handler(1, 3, 0, value_data[0]);
                break;
            case id_socd_pair_4_key_1:
                socd_pair_handler(1, 3, 1, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_4_key_2:
                socd_pair_handler(1, 3, 2, value_data[1] | (value_data[0] << 8));
                break;
            case id_socd_pair_4_mode:
                socd_pair_handler(1, 3, 3, value_data[0]);
            default:
                // Unhandled value.
                break;
        }
    }
}

// Handle the data sent by the keyboard to the VIA menus
void via_config_get_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);
    uint16_t socd_pair_result;

    if ((*value_id) < id_socd_pair_1_enabled) {
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
                break;
            }
            default: {
                // Unhandled value.
                break;
            }
        }
    } else {
        switch (*value_id) {
            case id_socd_pair_1_enabled:
                value_data[0] = socd_pair_handler(0, 0, 0, 0);
                break;
            case id_socd_pair_1_key_1:
                socd_pair_result = socd_pair_handler(0, 0, 1, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_1_key_2:
                socd_pair_result = socd_pair_handler(0, 0, 2, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_1_mode:
                value_data[0] = socd_pair_handler(0, 0, 3, 0);
                break;
            case id_socd_pair_2_enabled:
                value_data[0] = socd_pair_handler(0, 1, 0, 0);
                break;
            case id_socd_pair_2_key_1:
                socd_pair_result = socd_pair_handler(0, 1, 1, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_2_key_2:
                socd_pair_result = socd_pair_handler(0, 1, 2, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_2_mode:
                value_data[0] = socd_pair_handler(0, 1, 3, 0);
                break;
            case id_socd_pair_3_enabled:
                value_data[0] = socd_pair_handler(0, 2, 0, 0);
                break;
            case id_socd_pair_3_key_1:
                socd_pair_result = socd_pair_handler(0, 2, 1, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_3_key_2:
                socd_pair_result = socd_pair_handler(0, 2, 2, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_3_mode:
                value_data[0] = socd_pair_handler(0, 2, 3, 0);
                break;
            case id_socd_pair_4_enabled:
                value_data[0] = socd_pair_handler(0, 3, 0, 0);
                break;
            case id_socd_pair_4_key_1:
                socd_pair_result = socd_pair_handler(0, 3, 1, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_4_key_2:
                socd_pair_result = socd_pair_handler(0, 3, 2, 0);
                value_data[0]    = socd_pair_result >> 8;
                value_data[1]    = socd_pair_result & 0xFF;
                break;
            case id_socd_pair_4_mode:
                value_data[0] = socd_pair_handler(0, 3, 3, 0);
                break;
            default: {
                // Unhandled value.
                break;
            }
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

// Handle the SOCD pairs configuration
uint16_t socd_pair_handler(bool mode, uint8_t pair_idx, uint8_t field, uint16_t value) {
    if (mode) { // set
        switch (field) {
            case 0: // enabled
                eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].resolution = value;
                socd_opposing_pairs[pair_idx].resolution                  = value;
                eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, socd_opposing_pairs);
                return 0;
            case 1: // key 1
                eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].keys[0] = value;
                socd_opposing_pairs[pair_idx].keys[0]                  = value;
                eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, socd_opposing_pairs);
                return 0;
            case 2: // key 2
                eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].keys[1] = value;
                socd_opposing_pairs[pair_idx].keys[1]                  = value;
                eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, socd_opposing_pairs);
                return 0;
            case 3: // mode/resolution
                eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].resolution = value;
                socd_opposing_pairs[pair_idx].resolution                  = value;
                eeconfig_update_kb_datablock_field(eeprom_mx_wk_config, socd_opposing_pairs);
                return 0;
            default:
                return 0;
        }
    } else { // get
        switch (field) {
            case 0:
                return eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].resolution;
            case 1:
                return eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].keys[0];
            case 2:
                return eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].keys[1];
            case 3:
                return eeprom_mx_wk_config.socd_opposing_pairs[pair_idx].resolution;
            default:
                return 0;
        }
    }
}

#endif // VIA_ENABLE
