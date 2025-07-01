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

#include "hybrid_switch_matrix.h"
#include "socd_cleaner.h"
#include "action.h"
#include "print.h"
#include "via.h"

#ifdef VIA_ENABLE

void     ec_rescale_values(uint8_t item);
void     ec_save_threshold_data(uint8_t option);
void     ec_save_bottoming_reading(void);
void     ec_show_calibration_data(void);
void     ec_clear_bottoming_calibration_data(void);
uint16_t socd_pair_handler(bool mode, uint8_t pair_idx, uint8_t field, uint16_t value);

// Declaring enums for VIA config menu
enum via_enums {
    // clang-format off
    id_actuation_mode = 1,
    id_mode_0_actuation_threshold = 2,
    id_mode_0_release_threshold = 3,
    id_save_threshold_data = 4,
    id_mode_1_initial_deadzone_offset = 5,
    id_mode_1_actuation_offset = 6,
    id_mode_1_release_offset = 7,
    id_bottoming_calibration = 8,
    id_noise_floor_calibration = 9,
    id_show_calibration_data = 10,
    id_clear_bottoming_calibration_data = 11,
    id_switch_type = 12,
    id_socd_pair_1_enabled = 13,
    id_socd_pair_1_key_1 = 14,
    id_socd_pair_1_key_2 = 15,
    id_socd_pair_1_mode = 16,
    id_socd_pair_2_enabled = 17,
    id_socd_pair_2_key_1 = 18,
    id_socd_pair_2_key_2 = 19,
    id_socd_pair_2_mode = 20,
    id_socd_pair_3_enabled = 21,
    id_socd_pair_3_key_1 = 22,
    id_socd_pair_3_key_2 = 23,
    id_socd_pair_3_mode = 24,
    id_socd_pair_4_enabled = 25,
    id_socd_pair_4_key_1 = 26,
    id_socd_pair_4_key_2 = 27,
    id_socd_pair_4_mode = 28
    // clang-format on
};

// Handle the data received by the keyboard from the VIA menus
void via_config_set_value(uint8_t *data) {
    // data = [ value_id, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t *value_data = &(data[1]);

    switch (*value_id) {
        case id_switch_type: {
            eeprom_ec_config.switch_type = value_data[0];
            ec_config.switch_type        = eeprom_ec_config.switch_type;
            if (ec_config.switch_type == 0) {
                uprintf("#####################\n");
                uprintf("#  Switch Type: EC  #\n");
                uprintf("#####################\n");
            } else if (ec_config.switch_type == 1) {
                uprintf("###################\n");
                uprintf("# Switch Type: MX #\n");
                uprintf("###################\n");
            }
            eeconfig_update_kb_datablock_field(eeprom_ec_config, switch_type);
            break;
        }
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
            eeconfig_update_kb_datablock_field(eeprom_ec_config, actuation_mode);
            break;
        }
        case id_mode_0_actuation_threshold: {
            ec_config.mode_0_actuation_threshold = value_data[1] | (value_data[0] << 8);
            uprintf("APC Mode Actuation Threshold: %d\n", ec_config.mode_0_actuation_threshold);
            break;
        }
        case id_mode_0_release_threshold: {
            ec_config.mode_0_release_threshold = value_data[1] | (value_data[0] << 8);
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
            uprintf("Rapid Trigger Mode Actuation Offset: %d\n", ec_config.mode_1_actuation_offset);
            break;
        }
        case id_mode_1_release_offset: {
            ec_config.mode_1_release_offset = value_data[0];
            uprintf("Rapid Trigger Mode Release Offset: %d\n", ec_config.mode_1_release_offset);
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
                break;
            }
        }
        case id_show_calibration_data: {
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
            break;
        default: {
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

    switch (*value_id) {
        case id_switch_type: {
            value_data[0] = eeprom_ec_config.switch_type;
            break;
        }
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
                    ec_config.rescaled_mode_0_actuation_threshold[row][col] = rescale(ec_config.mode_0_actuation_threshold, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the APC mode release thresholds
        case 1:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_0_release_threshold[row][col] = rescale(ec_config.mode_0_release_threshold, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode initial deadzone offsets
        case 2:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_initial_deadzone_offset[row][col] = rescale(ec_config.mode_1_initial_deadzone_offset, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode actuation offsets
        case 3:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_actuation_offset[row][col] = rescale(ec_config.mode_1_actuation_offset, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
                }
            }
            break;
        // Rescale the Rapid Trigger mode release offsets
        case 4:
            for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
                for (uint8_t col = 0; col < MATRIX_COLS; col++) {
                    ec_config.rescaled_mode_1_release_offset[row][col] = rescale(ec_config.mode_1_release_offset, ec_config.noise_floor[row][col], eeprom_ec_config.bottoming_reading[row][col]);
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
        eeprom_ec_config.mode_1_actuation_offset        = ec_config.mode_1_actuation_offset;
        eeprom_ec_config.mode_1_release_offset          = ec_config.mode_1_release_offset;
        ec_rescale_values(2);
        ec_rescale_values(3);
        ec_rescale_values(4);
    }
    eeconfig_update_kb_datablock(&eeprom_ec_config, 0, EECONFIG_KB_DATA_SIZE);
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
    eeconfig_update_kb_datablock(&eeprom_ec_config, 0, EECONFIG_KB_DATA_SIZE);
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

// Handle the SOCD pairs configuration
uint16_t socd_pair_handler(bool mode, uint8_t pair_idx, uint8_t field, uint16_t value) {
    if (mode) { // set
        switch (field) {
            case 0: // enabled
                eeprom_ec_config.socd_opposing_pairs[pair_idx].resolution = value;
                socd_opposing_pairs[pair_idx].resolution                  = value;
                eeconfig_update_kb_datablock_field(eeprom_ec_config, socd_opposing_pairs);
                return 0;
            case 1: // key 1
                eeprom_ec_config.socd_opposing_pairs[pair_idx].keys[0] = value;
                socd_opposing_pairs[pair_idx].keys[0]                  = value;
                eeconfig_update_kb_datablock_field(eeprom_ec_config, socd_opposing_pairs);
                return 0;
            case 2: // key 2
                eeprom_ec_config.socd_opposing_pairs[pair_idx].keys[1] = value;
                socd_opposing_pairs[pair_idx].keys[1]                  = value;
                eeconfig_update_kb_datablock_field(eeprom_ec_config, socd_opposing_pairs);
                return 0;
            case 3: // mode/resolution
                eeprom_ec_config.socd_opposing_pairs[pair_idx].resolution = value;
                socd_opposing_pairs[pair_idx].resolution                  = value;
                eeconfig_update_kb_datablock_field(eeprom_ec_config, socd_opposing_pairs);
                return 0;
            default:
                return 0;
        }
    } else { // get
        switch (field) {
            case 0:
                return eeprom_ec_config.socd_opposing_pairs[pair_idx].resolution;
            case 1:
                return eeprom_ec_config.socd_opposing_pairs[pair_idx].keys[0];
            case 2:
                return eeprom_ec_config.socd_opposing_pairs[pair_idx].keys[1];
            case 3:
                return eeprom_ec_config.socd_opposing_pairs[pair_idx].resolution;
            default:
                return 0;
        }
    }
}

#endif // VIA_ENABLE
