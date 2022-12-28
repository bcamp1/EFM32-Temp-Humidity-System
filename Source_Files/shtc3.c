/*****************************************************
 * @file shtc3.c
 * @author Branson Camp
 * @date 11/28/2022
 * @brief Sets up and configures the SHTC3 Temperature/Humidity sensor
 ****************************************************/

#include "shtc3.h"

static uint32_t output_bytes;

/***************************************************************************//**
 * @brief
 *   General helper function for writing data using I2C.
 *
 * @details
 *   Uses I2C to write data of a given number of bytes to the SHTC3.
 *
 * @note
 *   This function should only be called by functions in this file.
 *
 * @param[in] data
 *   Data to write (maximum 4 bytes at a time).
 *
 * @param[in] num_bytes
 *   Number of bytes of data to be written.
 ******************************************************************************/
static void shtc3_i2c_write(uint32_t data, uint32_t num_bytes) {
  I2C_START_STRUCT i2c_start_struct;
  i2c_start_struct.which_i2c = 1;
  i2c_start_struct.comm_method = I2C_WRITE;
  i2c_start_struct.device_address = SHTC3_DEVICE_ADDRESS;
  i2c_start_struct.register_address = 0x00;
  i2c_start_struct.num_bytes = num_bytes;
  i2c_start_struct.finished_callback = 0x00;
  i2c_start_struct.data = &data;
  i2c_start_struct.num_register_bytes = 0;

  i2c_start(&i2c_start_struct);
  timer_delay(10);
}

/***************************************************************************//**
 * @brief
 *   General helper function for reading data using I2C.
 *
 * @details
 *   Uses I2C to read data of a given number of bytes from the SHTC3.
 *
 * @note
 *   This function should only be called by functions in this file.
 *
 * @param[in] data
 *   Pointer to deposit the data upon successful read.
 *
 * @param[in] data_bytes
 *   Number of bytes expected to be read from the SHTC3
 *
 * @param[in] command
 *   Command to initiate the reading (AKA the register address).
 *
 * @param[in] cb
 *   Callback code referenced upon successful read completion
 *
 * @param[in] num_register_bytes
 *   Number of bytes in the command to write (AKA the register address).
 ******************************************************************************/
static void shtc3_i2c_read(uint32_t* data, uint32_t data_bytes, uint32_t command, uint32_t cb, uint32_t num_register_bytes) {
  I2C_START_STRUCT i2c_start_struct;
  i2c_start_struct.which_i2c = 1;
  i2c_start_struct.comm_method = I2C_READ;
  i2c_start_struct.device_address = SHTC3_DEVICE_ADDRESS;
  i2c_start_struct.register_address = command;
  i2c_start_struct.num_bytes = data_bytes;
  i2c_start_struct.finished_callback = cb;
  i2c_start_struct.data = data;
  i2c_start_struct.num_register_bytes = num_register_bytes;
  i2c_start(&i2c_start_struct);
  timer_delay(10);
}

/***************************************************************************//**
 * @brief
 *   Configures and opens the I2C peripheral to allow communication.
 *
 * @details
 *   Configures the chosen I2C peripheral to begin communication with the SHTC3.
 *
 * @note
 *   This function should be called before doing any read or writes with the SHTC3.
 ******************************************************************************/
void shtc3_i2c_open() {
  timer_delay(SHTC3_STARTUP_TIME);

  I2C_OPEN_STRUCT i2c_config;

  i2c_config.master = true;
  i2c_config.enable = true;
  i2c_config.freq = I2C_FREQ_FAST_MAX;
  i2c_config.clhr = i2cClockHLRAsymetric;
  i2c_config.scl_route_pin = I2C1_SCL_ROUTE;
  i2c_config.sda_route_pin = I2C1_SDA_ROUTE;

  i2c_open(I2C1, &i2c_config);
}

/***************************************************************************//**
 * @brief
 *   Separates the read bytes into the temperature and humidity chunks.
 *
 * @details
 *   The SHTC3 gives a six byte response. This function assigns the correct
 *   bytes to temperature and relative humidity.
 *
 * @note
 *   This function should be called once the raw SHTC3 response has been sent.
 *
 * @param[in] t
 *   Pointer to temperature bytes to be deposited.
 *
 * @param[in] t_crc
 *   Pointer to temperature checksum byte to be deposited.
 *
 * @param[in] rh
 *   Pointer to relative humidity bytes to be deposited.
 *
 * @param[in] rh_crc
 *   Pointer to relative humidity checksum byte to be deposited.
 ******************************************************************************/
static void parse_data(uint32_t* t, uint32_t* t_crc, uint32_t* rh, uint32_t* rh_crc) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  const uint32_t num_bytes = 6;
  uint32_t bytes[6];

  for (int i = 0; i < num_bytes; i++) {
      bytes[i] = (output_bytes >> (8*i)) & 0xff;
  }

  *t = (bytes[0] << 8) | bytes[1];
  *t_crc = bytes[2];

  *rh = (bytes[3] << 8) | bytes[4];
  *rh_crc = bytes[5];
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Reads the data and checksums of temperature and humidity using I2C
 *
 * @details
 *   Starts three separate I2C operations to handle the demands of the
 *   SHTC3. Uses the callback once complete.
 *
 * @note
 *   This function should be called after calling shtc3_i2c_open()
 *
 * @param[in] cb
 *   Callback code for completion of acquiring the data.
 ******************************************************************************/
void shtc3_read_data_and_crc(uint32_t cb) {
  // Wake up command
  shtc3_i2c_write(SHTC3_WAKEUP_CMD, 2);
  timer_delay(SHTC3_STARTUP_TIME);

  // Measure command
  output_bytes = 0;
  shtc3_i2c_read(&output_bytes, 6, SHTC3_MEASURE_CMD_T_FIRST, cb, 2);

  // Sleep command
  shtc3_i2c_write(SHTC3_SLEEP_CMD, 2);
}

/***************************************************************************//**
 * @brief
 *   Calculates the temperature as degrees Fahrenheit.
 *
 * @details
 *   Once the raw data has been attained, this function uses the formulas
 *   defined in the SHTC3 manual to obtain a temperature in degrees F.
 *
 * @note
 *   This function should be called after calling shtc3_read_data_and_crc().
 *
 * @return
 *   The temperature in degrees Fahrenheit.
 ******************************************************************************/
static float shtc3_calc_temp() {
  uint32_t t, t_crc, rh, rh_crc;
  parse_data(&t, &t_crc, &rh, &rh_crc);

  float fraction = ((float) t) / (65536.0);

  return -45.0 + (175.0 * fraction);
}

/***************************************************************************//**
 * @brief
 *   Calculates the relative humidity as a percent (%).
 *
 * @details
 *   Once the raw data has been attained, this function uses the formulas
 *   defined in the SHTC3 manual to obtain the relative humidity as a percent (%).
 *
 * @note
 *   This function should be called after calling shtc3_read_data_and_crc().
 *
 * @return
 *   The relative humidity as a percent (%).
 ******************************************************************************/
static float shtc3_calc_hum() {
    uint32_t t, t_crc, rh, rh_crc;
    parse_data(&t, &t_crc, &rh, &rh_crc);

    float fraction = ((float) rh) / (65536.0);

    return 100.0 * fraction;
}

/***************************************************************************//**
 * @brief
 *   Gets the temperature (F) and humidity (%) and drops them into pointers
 *   of your choosing.
 *
 * @details
 *   Uses shtc3_calc_temp() and shtc3_calc_hum() to fetch both results at once.
 *
 * @note
 *   This function should be used after shtc3_read_data_and_crc has been called
 *   and has been completed successfully.
 *
 * @param[in] temp
 *   Temperature pointer for temperature value to be deposited.
 *
 * @param[in] hum
 *   Humidity pointer for humidity value to be deposited.
 ******************************************************************************/
void shtc3_app_get_temp_and_hum(float* temp, float* hum) {
  *temp = shtc3_calc_temp();
  *hum = shtc3_calc_hum();
}


