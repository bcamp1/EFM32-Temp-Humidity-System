/*
 * i2c.h
 *
 *  Created on: Oct 11, 2022
 *      Author: bransoncamp
 */

#ifndef SRC_HEADER_FILES_I2C_H_
#define SRC_HEADER_FILES_I2C_H_

/* Silicon Labs include statements */
#include "em_letimer.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"
#include "em_i2c.h"
#include "sleep_routines.h"
#include "scheduler.h"

#define I2C_EM  EM2
#define I2C_R 1u
#define I2C_W 0u

typedef enum {
  I2C_READ,
  I2C_WRITE,
} I2C_COMM_METHOD_TypeDef;

typedef struct {
  bool enable;
  bool master;
  uint32_t refFreq;
  uint32_t freq;
  I2C_ClockHLR_TypeDef clhr;

  uint32_t sda_route_pin;
  uint32_t scl_route_pin;

} I2C_OPEN_STRUCT;

typedef struct {
  bool which_i2c;
  I2C_COMM_METHOD_TypeDef comm_method;
  uint32_t device_address;
  uint32_t register_address;
  uint32_t num_bytes;
  uint32_t finished_callback;
  uint32_t* data;
  uint32_t num_register_bytes;
} I2C_START_STRUCT;

void i2c_open(I2C_TypeDef *i2cx, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_START_STRUCT *i2c_start);
// void i2c_isr(I2C_TypeDef *i2cx);

void I2C0_IRQHandler();
void I2C1_IRQHandler();

#endif /* SRC_HEADER_FILES_I2C_H_ */
