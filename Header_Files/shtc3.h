/*
 * shtc3.h
 *
 *  Created on: Nov 15, 2022
 *      Author: bransoncamp
 */

#ifndef SRC_HEADER_FILES_SHTC3_H_
#define SRC_HEADER_FILES_SHTC3_H_

/* Silicon Labs include statements */
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"
#include "em_i2c.h"
#include "i2c.h"
#include "HW_delay.h"
#include "brd_config.h"

#define SHTC3_STARTUP_TIME 240
#define SHTC3_DEVICE_ADDRESS 0x70
#define SHTC3_WAKEUP_CMD 0x3517
#define SHTC3_SLEEP_CMD 0xB098
#define SHTC3_MEASURE_CMD_T_FIRST 0x7866
//#define SHTC3_MEASURE_CMD_T_FIRST 0x58E0

void shtc3_i2c_open();
void shtc3_app_get_temp_and_hum(float* temp, float* hum);
void shtc3_read_data_and_crc(uint32_t cb);

#endif /* SRC_HEADER_FILES_SHTC3_H_ */
