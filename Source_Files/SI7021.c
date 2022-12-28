/*****************************************************
 * @file SI7021.c
 * @author Branson Camp
 * @date 10/27/2022
 * @brief Sets up and configures the SI7021 humidity sensor.
 *
 ****************************************************/

#include "SI7021.h"

static uint32_t hum_bytes;
static uint32_t temp_bytes;
static uint32_t user_settings_bytes;

/***************************************************************************//**
 * @brief
 *   Initializes the SI7021 sensor.
 *
 * @details
 *   Initializes the SI7021 sensor including clock tree configuration and GPIO setup.
 *   Also changes settings of the SI7021 and verifies it using I2C.
 *
 * @note
 *   This function should be called before read or write operations are called.
 *
 * @param[in] cb
 *   Callback code to verify user settings changes
 ******************************************************************************/
void si7021_i2c_open(uint32_t cb) {
  timer_delay(SI7021_STARTUP_TIME);

  I2C_OPEN_STRUCT i2c_config;

  i2c_config.master = true;
  i2c_config.enable = true;
  i2c_config.freq = I2C_FREQ_FAST_MAX;
  i2c_config.clhr = i2cClockHLRAsymetric;

  if (SI7021_WHICH_I2C == 0) {
    i2c_config.scl_route_pin = I2C0_SCL_ROUTE;
    i2c_config.sda_route_pin = I2C0_SDA_ROUTE;
    i2c_open(I2C0, &i2c_config);
  } else if (SI7021_WHICH_I2C == 1) {
    i2c_config.scl_route_pin = I2C1_SCL_ROUTE;
    i2c_config.sda_route_pin = I2C1_SDA_ROUTE;
    i2c_open(I2C1, &i2c_config);
  } else {
    EFM_ASSERT(false);
  }

  // Configure correct User settings by performing I2C write
  uint32_t data_to_write = SI7021_USER_SETTINGS;

  I2C_START_STRUCT i2c_start_struct;
  i2c_start_struct.which_i2c = SI7021_WHICH_I2C;
  i2c_start_struct.comm_method = I2C_WRITE;
  i2c_start_struct.device_address = SI7021_DEVICE_ADDR;
  i2c_start_struct.register_address = SI7021_WRITE_USER_CMD;
  i2c_start_struct.num_bytes = 1;
  i2c_start_struct.finished_callback = 0x00;
  i2c_start_struct.data = &data_to_write;
  i2c_start_struct.num_register_bytes = 1;

  i2c_start(&i2c_start_struct);
  timer_delay(10);


  // Configure correct User settings by performing I2C read

  user_settings_bytes = 0;

  i2c_start_struct.which_i2c = SI7021_WHICH_I2C;
  i2c_start_struct.comm_method = I2C_READ;
  i2c_start_struct.device_address = SI7021_DEVICE_ADDR;
  i2c_start_struct.register_address = SI7021_READ_USER_CMD;
  i2c_start_struct.num_bytes = 1;
  i2c_start_struct.finished_callback = cb;
  i2c_start_struct.data = &user_settings_bytes;
  i2c_start_struct.num_register_bytes = 1;

  i2c_start(&i2c_start_struct);
  timer_delay(10);

}

/***************************************************************************//**
 * @brief
 *   Reads relative humidity via I2C from SI7021
 *
 * @details
 *   Initiates an I2C read using the appropriate commands and schedules the callback
 *   upon completion
 *
 * @note
 *   This function should be called after SI7021 has been opened.
 *
 * @param[in] cb
 *  Callback event which is triggered upon read completion.
 ******************************************************************************/
void si7021_read_humidity(uint32_t cb) {
  hum_bytes = 0;

  I2C_START_STRUCT i2c_start_struct;
  i2c_start_struct.which_i2c = SI7021_WHICH_I2C;
  i2c_start_struct.comm_method = I2C_READ;
  i2c_start_struct.device_address = SI7021_DEVICE_ADDR;
  i2c_start_struct.register_address = SI7021_HUM_CMD;
  i2c_start_struct.num_bytes = 2;
  i2c_start_struct.finished_callback = cb;
  i2c_start_struct.data = &hum_bytes;
  i2c_start_struct.num_register_bytes = 1;

  i2c_start(&i2c_start_struct);
}

/***************************************************************************//**
 * @brief
 *   Reads temperature via I2C from SI7021
 *
 * @details
 *   Initiates an I2C read using the appropriate commands and schedules the callback
 *   upon completion
 *
 * @note
 *   This function should be called after SI7021 has been opened.
 *
 * @param[in] cb
 *  Callback event which is triggered upon read completion.
 ******************************************************************************/
void si7021_read_temp(uint32_t cb) {
  temp_bytes = 0;

  I2C_START_STRUCT i2c_start_struct;
  i2c_start_struct.which_i2c = SI7021_WHICH_I2C;
  i2c_start_struct.comm_method = I2C_READ;
  i2c_start_struct.device_address = SI7021_DEVICE_ADDR;
  i2c_start_struct.register_address = SI7021_TEMP_CMD;
  i2c_start_struct.num_bytes = 2;
  i2c_start_struct.finished_callback = cb;
  i2c_start_struct.data = &temp_bytes;
  i2c_start_struct.num_register_bytes = 1;

  i2c_start(&i2c_start_struct);
}

/***************************************************************************//**
 * @brief
 *   Returns the relative humidity as a percent (%)
 *
 * @details
 *   Takes the raw bytes from the sensor and converts it into a percent relative humidity
 *   value using a calibration equation from the SI7021 datasheet.
 *
 * @note
 *   This function should be called after si7021_read_humidity is called.
 *
 * @return
 *   The relative humidity as a percent (%)
 ******************************************************************************/
float si7021_get_humidity() {
  return ((125*(float)hum_bytes)/65536.0) - 6.0;
}

/***************************************************************************//**
 * @brief
 *   Returns the temperature in degrees Celcius.
 *
 * @details
 *   Takes the raw bytes from the sensor and converts it into degrees Celcius
 *   value using a calibration equation from the SI7021 datasheet.
 *
 * @note
 *   This function should be called after si7021_read_humidity is called.
 *
 * @return
 *   The temperature in degrees Celcius.
 ******************************************************************************/
float si7021_get_temp() {
  return ((175.72*(float)temp_bytes)/65536.0) - 46.85;
}

/***************************************************************************//**
 * @brief
 *   Returns the SI7021 User Settings byte after it has been read.
 *
 * @details
 *   The User Settings on the I2C controls the output bit resolution,
 *   on-board heater and more. This function returns the sensor's current
 *   configuration after the sensor has been opened.
 *
 * @note
 *   This function should be called after SI7021 has been successfully opened.
 *
 * @return
 *   The SI7021 User Settings byte
 ******************************************************************************/
uint32_t si7021_get_user_settings() {
  return user_settings_bytes;
}

