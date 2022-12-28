/*
 * SI7021.h
 *
 *  Created on: Oct 11, 2022
 *      Author: bransoncamp
 */

#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_

/* Silicon Labs include statements */
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"
#include "em_i2c.h"
#include "i2c.h"
#include "HW_delay.h"
#include "brd_config.h"

#define SI7021_STARTUP_TIME   80
#define SI7021_DEVICE_ADDR    0x40
#define SI7021_HUM_CMD        0xF5
#define SI7021_TEMP_CMD       0xF3

#define SI7021_READ_USER_CMD    0xE7
#define SI7021_WRITE_USER_CMD   0xE6
#define SI7021_USER_SETTINGS    0b00111011

void si7021_i2c_open(uint32_t cb);
void si7021_read_humidity(uint32_t cb);
void si7021_read_temp(uint32_t cb);
float si7021_get_humidity();
float si7021_get_temp();
uint32_t si7021_get_user_settings();


#endif /* SRC_HEADER_FILES_SI7021_H_ */
