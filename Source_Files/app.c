/****************************************************
 * @file app.c
 * @author Branson Camp
 * @date 9/18/2022
 * @brief Sets up, starts, and facilitates interaction
 * between peripherals.
 *
 ****************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Static / Private Variables
//***********************************************************************************
//static uint32_t humidity_result = 0;

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Opens and starts all relevant peripherals.
 *
 * @details
 *   Opens the CMU, GPIO, and LETIMER peripherals. Also starts the LETIMER.
 *
 * @note
 *   This function should run once at the start of the program.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
  scheduler_open(); // Initialize the scheduler
  sleep_open(); // Initialize sleep manager
  cmu_open();
  gpio_open();

  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  letimer_start(LETIMER0, true);   // letimer_start will inform the LETIMER0 peripheral to begin counting.
  si7021_i2c_open(SI7021_USER_CONFIRM);
  shtc3_i2c_open();
}

/***************************************************************************//**
 * @brief
 *  Opens the letimer provided the period and active period
 *
 * @details
 *  Opens the LETIMER. Sets it to PWM. Must provide period and
 *  active period. The output of the timer can be routed to many
 *  places, see the pearl gecko documentation.
 *
 * @note
 *  This function is normally called once.
 *
 * @param[in] period
 *  Period, in seconds
 *
 * @param[in] act_period
 *  Active period, in seconds
 *
 * @param[in] out0_route
 *   Sets the route number for LETIMER's OUT0
 *
 * @param[in] out1_route
 *   Sets the route number for LETIMER's OUT0
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements

  APP_LETIMER_PWM_TypeDef letimer_pwm_struct;
  letimer_pwm_struct.enable = false;
  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;
  letimer_pwm_struct.out_pin_0_en = false; // Was set to true for lab 3
  letimer_pwm_struct.out_pin_1_en = false;
  letimer_pwm_struct.period = period;
  letimer_pwm_struct.active_period = act_period;

  // Interrupts
  letimer_pwm_struct.comp0_irq_enable = false;
  letimer_pwm_struct.comp1_irq_enable = true;
  letimer_pwm_struct.uf_irq_enable = true;
  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;

  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 *   Callback function for the underflow interrupt.
 *
 * @details
 *   Not currently doing anything except assert that the event is no longer
 *   scheduled.
 *
 * @note
 *   This function runs once the scheduled task is checked in main.c
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb (void) {
  si7021_read_humidity(SI7021_READ_HUM_CB);
  si7021_read_temp(SI7021_READ_TEMP_CB);
  shtc3_read_data_and_crc(SHTC3_READ_CB);
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_UF_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the COMP0 interrupt.
 *
 * @details
 *   Not currently doing anything except assert that the event is no longer
 *   scheduled.
 *
 * @note
 *   This function runs once the scheduled task is checked in main.c
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb (void) {
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_COMP0_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the COMP1 interrupt.
 *
 * @details
 *   Not currently doing anything except assert that the event is no longer
 *   scheduled.
 *
 * @note
 *   This function runs once the scheduled task is checked in main.c
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb (void) {
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_COMP1_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the button 1 interrupt.
 *
 * @details
 *   Increments the current block energy mode. Wraps around to EM0 if
 *   current block energy mode is EM4
 *
 * @note
 *   This function runs when button 1 is pressed.
 *
 ******************************************************************************/
void scheduled_gpio_odd_irq_cb (void) {
  uint32_t current_block_em = current_block_energy_mode();
  sleep_unblock_mode(current_block_em);
  if (current_block_em < EM4) {
      sleep_block_mode(current_block_em + 1);
  } else {
      sleep_block_mode(EM0);
  }
  EFM_ASSERT(!(get_scheduled_events() & GPIO_ODD_IRQ_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the button 0 interrupt.
 *
 * @details
 *   Decrements the current block energy mode. Wraps around to EM4 if
 *   current block energy mode is EM0
 *
 * @note
 *   This function runs when button 0 is pressed.
 *
 ******************************************************************************/
void scheduled_gpio_even_irq_cb (void) {
  uint32_t current_block_em = current_block_energy_mode();
  sleep_unblock_mode(current_block_em);
  if (current_block_em > EM0) {
    sleep_block_mode(current_block_em - 1);
  } else {
    sleep_block_mode(EM4);
  }
  EFM_ASSERT(!(get_scheduled_events() & GPIO_EVEN_IRQ_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the SI7021's humidity read completion event.
 *
 * @details
 *   Gets the humidity as a percent value and activates an LED if the value is greater
 *   or equal to 30.0
 *
 * @note
 *   This function runs when the result from si7021_read_humidity is ready.
 *
 ******************************************************************************/
void scheduled_si7021_read_hum_cb(void) {
  float humidity_percent = si7021_get_humidity();
  if (humidity_percent >= HUMIDITY_COMPARE) {
      // Turn LED0 on
      GPIO->P[LED0_PORT].DOUT |= 1 << LED0_PIN;
  } else {
      // Turn LED0 off
      GPIO->P[LED0_PORT].DOUT &= ~(1 << LED0_PIN);
  }


  char hum_result[50];
  sprintf(hum_result, "%3.1f %% humidity", humidity_percent);

  EFM_ASSERT(!(get_scheduled_events() & SI7021_READ_HUM_CB));
}

/***************************************************************************//**
 * @brief
 *   Callback function for the SI7021's temperature read completion event.
 *
 * @details
 *   Gets the temperature in Fahrenheit and stores it in a string.
 *
 * @note
 *   This function runs when the result from si7021_read_temperature is ready.
 *
 ******************************************************************************/
void scheduled_si7021_read_temp_cb(void) {
  float temp_c = si7021_get_temp();
  float temp_f = (temp_c*1.8) + 32.0;
  EFM_ASSERT(!(get_scheduled_events() & SI7021_READ_TEMP_CB));

  char temp_result[50];
  sprintf(temp_result, "%3.1f F", temp_f);
}

/***************************************************************************//**
 * @brief
 *   Callback function for the SHTC3's temp and RH read completion
 *
 * @details
 *   Gets the temperature (F) and relative humidity (%) and displays them
 *   as strings.
 *
 * @note
 *   This function runs when the result from shtc3_read_data_and_crc is ready.
 *
 ******************************************************************************/
void scheduled_shtc3_read_irq_cb(void) {
  float temp = 0.0;
  float hum = 0.0;
  shtc3_app_get_temp_and_hum(&temp, &hum);
  float temp_f = (temp*1.8) + 32.0;
  char other_temp_result[50];
  sprintf(other_temp_result, "%3.1f F", temp_f);
  char other_hum_result[50];
  sprintf(other_hum_result, "%3.1f %% humidity", hum);
}

/***************************************************************************//**
 * @brief
 *   Callback after SI7021 user settings are read. Checks that user settings
 *   are correct.
 *
 * @details
 *   This function is used to check that writes to the SI7021 user settings
 *   were successful.
 *
 * @note
 *   This function runs when the result from the SI7021 I2C read of user settings
 *   is ready.
 *
 ******************************************************************************/
void scheduled_si7021_user_confirm(void) {
  uint32_t user_settings = si7021_get_user_settings();
  EFM_ASSERT(user_settings == SI7021_USER_SETTINGS);
}


