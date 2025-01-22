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

#include "keyboards/cipulot/common/eeprom_tools.h"
#include "ec_switch_matrix.h"
#include "action.h"
#include "print.h"
#include "via.h"

#ifdef VIA_ENABLE

void ec_rescale_values(uint8_t item);
void ec_save_threshold_data(uint8_t option);
void ec_save_bottoming_reading(void);
void ec_show_calibration_data(void);
void ec_clear_bottoming_calibration_data(void);

// Declaring enums for VIA config menu
enum via_enums {
    // clang-format off
    id_ind1_enabled = 1,
    id_ind1_brightness = 2,
    id_ind1_color = 3,
    id_ind1_func1 = 4,
    id_ind1_func2 = 5,
    id_ind2_enabled = 6,
    id_ind2_brightness = 7,
    id_ind2_color = 8,
    id_ind2_func1 = 9,
    id_ind2_func2 = 10,
    id_ind3_enabled = 11,
    id_ind3_brightness = 12,
    id_ind3_color = 13,
    id_ind3_func1 = 14,
    id_ind3_func2 = 15,
    id_actuation_mode = 16,
    id_mode_0_actuation_threshold = 17,
    id_mode_0_release_threshold = 18,
    id_save_threshold_data = 19,
    id_mode_1_initial_deadzone_offset = 20,
    id_mode_1_actuation_offset = 21,
    id_mode_1_release_offset = 22,
    id_bottoming_calibration = 23,
    id_noise_floor_calibration = 24,
    id_show_calibration_data = 25,
    id_clear_bottoming_calibration_data = 26
    // clang-format on
};

int indi_index;
int data_index;

// Handle the data received by the keyboard from the VIA menus
void via_config_set_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    if ((*value_id) < id_actuation_mode) {
        indi_index                            = ((int)(*value_id) - 1) / 5;
        data_index                            = (int)(*value_id) - indi_index * 5;
        indicator_config *current_indicator_p = get_indicator_p(indi_index);
        uprintf("--> value_id: %X\n", (int)(*value_id));

        switch (data_index) {
            case 1: {
                current_indicator_p->enabled = value_data[0];
                if (indi_index == 0) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.enabled);
                } else if (indi_index == 1) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.enabled);
                } else if (indi_index == 2) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.enabled);
                }
                break;
            }
            case 2: {
                current_indicator_p->v = value_data[0];
                if (indi_index == 0) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.v);
                } else if (indi_index == 1) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.v);
                } else if (indi_index == 2) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.v);
                }
                break;
            }
            case 3: {
                current_indicator_p->h = value_data[0];
                current_indicator_p->s = value_data[1];
                if (indi_index == 0) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.h);
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.s);
                } else if (indi_index == 1) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.h);
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.s);
                } else if (indi_index == 2) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.h);
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.s);
                }
                break;
            }
            case 4: {
                current_indicator_p->func = (current_indicator_p->func & 0xF0) | (uint8_t)value_data[0];
                if (indi_index == 0) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.func);
                } else if (indi_index == 1) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.func);
                } else if (indi_index == 2) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.func);
                }
                break;
            }
            case 5: {
                current_indicator_p->func = (current_indicator_p->func & 0x0F) | ((uint8_t)value_data[0] << 4);
                if (indi_index == 0) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind1.func);
                } else if (indi_index == 1) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind2.func);
                } else if (indi_index == 2) {
                    EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, ind3.func);
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
            case id_actuation_mode: {
                eeprom_ec_config.actuation_mode = value_data[0];
                ec_config.actuation_mode        = eeprom_ec_config.actuation_mode;
                if (ec_config.actuation_mode == 0) {
                    uprintf("#########################\n");
                    uprintf("#  Actuation Mode: APC  #\n");
                    uprintf("#########################\n");
                } else if (ec_config.actuation_mode == 1) {
                    uprintf("#################################\n");
                    uprintf("# Actuation Mode: Rapid Trigger #\n");
                    uprintf("#################################\n");
                }
                EEPROM_KB_PARTIAL_UPDATE(eeprom_ec_config, actuation_mode);
                break;
            }
            case id_mode_0_actuation_threshold: {
                ec_config.mode_0_actuation_threshold = value_data[1] | (value_data[0] << 8);
                uprintf("APC Mode Actuation Threshold: %d\n", ec_config.mode_0_actuation_threshold);
                break;
            }
            case id_mode_0_release_threshold: {
                eeprom_ec_config.mode_0_release_threshold = value_data[1] | (value_data[0] << 8);
                ec_config.mode_0_release_threshold        = eeprom_ec_config.mode_0_release_threshold;
                uprintf("APC Mode Release Threshold: %d\n", ec_config.mode_0_release_threshold);
                break;
            }
            case id_mode_1_initial_deadzone_offset: {
                ec_config.mode_1_initial_deadzone_offset = value_data[1] | (value_data[0] << 8);
                uprintf("Rapid Trigger Mode Initial Deadzone Offset: %d\n", ec_config.mode_1_initial_deadzone_offset);
                break;
            }
            case id_mode_1_actuation_offset: {
                ec_config.mode_1_actuation_offset = value_data[0];
                uprintf("Rapid Trigger Mode Actuation Sensitivity: %d\n", ec_config.mode_1_actuation_offset);
                break;
            }
            case id_mode_1_release_offset: {
                ec_config.mode_1_release_offset = value_data[0];
                uprintf("Rapid Trigger Mode Release Sensitivity: %d\n", ec_config.mode_1_release_offset);
                break;
            }
            case id_bottoming_calibration: {
                if (value_data[0] == 1) {
                    ec_config.bottoming_calibration = true;
                    uprintf("##############################\n");
                    uprintf("# Bottoming calibration mode #\n");
                    uprintf("##############################\n");
                } else {
                    ec_config.bottoming_calibration = false;
                    ec_save_bottoming_reading();
                    uprintf("## Bottoming calibration done ##\n");
                    ec_show_calibration_data();
                }
                break;
            }
            case id_save_threshold_data: {
                ec_save_threshold_data(value_data[0]);
                break;
            }
            case id_noise_floor_calibration: {
                if (value_data[0] == 0) {
                    ec_noise_floor();
                    ec_rescale_values(0);
                    ec_rescale_values(1);
                    ec_rescale_values(2);
                    ec_rescale_values(3);
                    ec_rescale_values(4);
                    uprintf("#############################\n");
                    uprintf("# Noise floor data acquired #\n");
                    uprintf("#############################\n");
                }
                break;
            }
            case id_show_calibration_data: {
                // Show calibration data once if the user toggle the switch
                if (value_data[0] == 0) {
                    ec_show_calibration_data();
                }
                break;
            }
            case id_clear_bottoming_calibration_data: {
                if (value_data[0] == 0) {
                    ec_clear_bottoming_calibration_data();
                }
                break;
            }
            default: {
                // Unhandled value.
                break;
            }
        }
    }
}

// Handle the data sent by the keyboard to the VIA menus
void via_config_get_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    if ((*value_id) < id_actuation_mode) {
        indi_index                            = ((int)(*value_id) - 1) / 5;
        data_index                            = (int)(*value_id) - indi_index * 5;
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
            case 5: {
                value_data[0] = (current_indicator_p->func & 0xF0) >> 4;
                uprintf("--> Current func: %x, current func with bitwise or: %X\n", current_indicator_p->func, current_indicator_p->func % 0xF0);
                break;
            }
            default: {
                // Unhandled value.
                break;
            }
        }
    } else {
        switch (*value_id) {
            case id_actuation_mode: {
                value_data[0] = eeprom_ec_config.actuation_mode;
                break;
            }
            case id_mode_0_actuation_threshold: {
                value_data[0] = eeprom_ec_config.mode_0_actuation_threshold >> 8;
                value_data[1] = eeprom_ec_config.mode_0_actuation_threshold & 0xFF;
                break;
            }
            case id_mode_0_release_threshold: {
                value_data[0] = eeprom_ec_config.mode_0_release_threshold >> 8;
                value_data[1] = eeprom_ec_config.mode_0_release_threshold & 0xFF;
                break;
            }
            case id_mode_1_initial_deadzone_offset: {
                value_data[0] = eeprom_ec_config.mode_1_initial_deadzone_offset >> 8;
                value_data[1] = eeprom_ec_config.mode_1_initial_deadzone_offset & 0xFF;
                break;
            }
            case id_mode_1_actuation_offset: {
                value_data[0] = eeprom_ec_config.mode_1_actuation_offset;
                break;
            }
            case id_mode_1_release_offset: {
                value_data[0] = eeprom_ec_config.mode_1_release_offset;
                break;
            }
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

// Rescale the values received by VIA to fit the new range
void ec_rescale_values(uint8_t item) {
    switch (item) {
        // Rescale the APC mode actuation thresholds
        case 0:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_0_actuation_threshold[row][col] = rescale(ec_config.mode_0_actuation_threshold, 0, 1023, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the APC mode release thresholds
        case 1:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_0_release_threshold[row][col] = rescale(ec_config.mode_0_release_threshold, 0, 1023, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode initial deadzone offsets
        case 2:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_initial_deadzone_offset[row][col] = rescale(ec_config.mode_1_initial_deadzone_offset, 0, 1023, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode actuation offsets
        case 3:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_actuation_offset[row][col] = rescale(ec_config.mode_1_actuation_offset, 0, 1023, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode release offsets
        case 4:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_release_offset[row][col] = rescale(ec_config.mode_1_release_offset, 0, 1023, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;

        default:
            // Unhandled item.
            break;
    }
}

void ec_save_threshold_data(uint8_t option) {
    // Save APC mode thresholds and rescale them for runtime usage
    if (option == 0) {
        eeprom_ec_config.mode_0_actuation_threshold = ec_config.mode_0_actuation_threshold;
        eeprom_ec_config.mode_0_release_threshold   = ec_config.mode_0_release_threshold;
        ec_rescale_values(0);
        ec_rescale_values(1);
    }
    // Save Rapid Trigger mode thresholds and rescale them for runtime usage
    else if (option == 1) {
        eeprom_ec_config.mode_1_initial_deadzone_offset = ec_config.mode_1_initial_deadzone_offset;
        ec_rescale_values(2);
        ec_rescale_values(3);
        ec_rescale_values(4);
    }
    eeconfig_update_kb_datablock(&eeprom_ec_config);
    uprintf("####################################\n");
    uprintf("# New thresholds applied and saved #\n");
    uprintf("####################################\n");
}

// Save the bottoming reading
void ec_save_bottoming_reading(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            // If the calibration starter flag is still set on the key, it indicates that the key was skipped during the scan because it is not physically present.
            // If the flag is not set, it means a bottoming reading was taken. If this reading doesn't exceed the noise floor by the BOTTOMING_CALIBRATION_THRESHOLD, it likely indicates one of the following:
            // 1. The key is part of an alternative layout and is not being pressed.
            // 2. The key is in the current layout but is not being pressed.
            // In both conditions we should set the bottoming reading to the maximum value to avoid false positives.
            if (ec_config.bottoming_calibration_starter[row][col] || ec_config.bottoming_reading[row][col] < (ec_config.noise_floor[row][col] + BOTTOMING_CALIBRATION_THRESHOLD)) {
                eeprom_ec_config.bottoming_reading[row][col] = 1023;
            } else {
                eeprom_ec_config.bottoming_reading[row][col] = ec_config.bottoming_reading[row][col];
            }
        }
    }
    // Rescale the values to fit the new range for runtime usage
    ec_rescale_values(0);
    ec_rescale_values(1);
    ec_rescale_values(2);
    ec_rescale_values(3);
    ec_rescale_values(4);
    eeconfig_update_kb_datablock(&eeprom_ec_config);
}

// Show the calibration data
void ec_show_calibration_data(void) {
    uprintf("\n###############\n");
    uprintf("# Noise Floor #\n");
    uprintf("###############\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS - 1; col++) {
            uprintf("%4d,", ec_config.noise_floor[row][col]);
        }
        uprintf("%4d\n", ec_config.noise_floor[row][MATRIX_COLS - 1]);
    }

    uprintf("\n######################\n");
    uprintf("# Bottoming Readings #\n");
    uprintf("######################\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS - 1; col++) {
            uprintf("%4d,", eeprom_ec_config.bottoming_reading[row][col]);
        }
        uprintf("%4d\n", eeprom_ec_config.bottoming_reading[row][MATRIX_COLS - 1]);
    }

    uprintf("\n######################################\n");
    uprintf("# Rescaled APC Mode Actuation Points #\n");
    uprintf("######################################\n");
    uprintf("Original APC Mode Actuation Point: %4d\n", ec_config.mode_0_actuation_threshold);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS - 1; col++) {
            uprintf("%4d,", ec_config.rescaled_mode_0_actuation_threshold[row][col]);
        }
        uprintf("%4d\n", ec_config.rescaled_mode_0_actuation_threshold[row][MATRIX_COLS - 1]);
    }

    uprintf("\n######################################\n");
    uprintf("# Rescaled APC Mode Release Points   #\n");
    uprintf("######################################\n");
    uprintf("Original APC Mode Release Point: %4d\n", ec_config.mode_0_release_threshold);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS - 1; col++) {
            uprintf("%4d,", ec_config.rescaled_mode_0_release_threshold[row][col]);
        }
        uprintf("%4d\n", ec_config.rescaled_mode_0_release_threshold[row][MATRIX_COLS - 1]);
    }

    uprintf("\n#######################################################\n");
    uprintf("# Rescaled Rapid Trigger Mode Initial Deadzone Offset #\n");
    uprintf("#######################################################\n");
    uprintf("Original Rapid Trigger Mode Initial Deadzone Offset: %4d\n", ec_config.mode_1_initial_deadzone_offset);
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS - 1; col++) {
            uprintf("%4d,", ec_config.rescaled_mode_1_initial_deadzone_offset[row][col]);
        }
        uprintf("%4d\n", ec_config.rescaled_mode_1_initial_deadzone_offset[row][MATRIX_COLS - 1]);
    }
    print("\n");
}

// Clear the calibration data
void ec_clear_bottoming_calibration_data(void) {
    // Clear the EEPROM data
    eeconfig_init_kb();

    // Reset the runtime values to the EEPROM values
    keyboard_post_init_kb();

    uprintf("######################################\n");
    uprintf("# Bottoming calibration data cleared #\n");
    uprintf("######################################\n");
}

#endif // VIA_ENABLE