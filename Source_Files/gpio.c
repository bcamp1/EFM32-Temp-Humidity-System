/*****************************************************
 * @file gpio.c
 * @author Branson Camp
 * @date 9/18/2022
 * @brief Sets up and configures the GPIO peripheral
 *
 ****************************************************/


//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t gpio_even_irq_cb = 0b01000;
static uint32_t gpio_odd_irq_cb = 0b10000;

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Initializes the GPIO peripheral.
 *
 * @details
 *   Opens the GPIO (General Purpose Input Output) peripheral. Sets the relevant
 *   clocks and configures the two LEDs for output.
 *
 * @note
 *   This function should run once at the start of the program.
 *
 ******************************************************************************/
void gpio_open(void){

  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure LED pins
  GPIO_DriveStrengthSet(LED0_PORT, LED0_DRIVE_STRENGTH);
  GPIO_PinModeSet(LED0_PORT, LED0_PIN, LED0_GPIOMODE, LED0_DEFAULT);

  GPIO_DriveStrengthSet(LED1_PORT, LED1_DRIVE_STRENGTH);
  GPIO_PinModeSet(LED1_PORT, LED1_PIN, LED1_GPIOMODE, LED1_DEFAULT);

  // Configure buttons
  GPIO_PinModeSet(BUTTON_0_PORT, BUTTON_0_PIN, BUTTON_0_CONFIG, BUTTON_DEFAULT);
  GPIO_PinModeSet(BUTTON_1_PORT, BUTTON_1_PIN, BUTTON_1_CONFIG, BUTTON_DEFAULT);

  // Enable Button Interrupts
  GPIO_ExtIntConfig(BUTTON_0_PORT, BUTTON_0_PIN, BUTTON_0_INT_NUM, BUTTON_0_INT_RISING, BUTTON_0_INT_FALLING, BUTTON_0_INT_ENABLE);
  GPIO_ExtIntConfig(BUTTON_1_PORT, BUTTON_1_PIN, BUTTON_1_INT_NUM, BUTTON_1_INT_RISING, BUTTON_1_INT_FALLING, BUTTON_1_INT_ENABLE);

  // NVIC Enable Interrupts
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  // Configure Sensor Enable Pin
  GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull, 1);

  // Configure SI7021 SDA SCL
  GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN,  SI7021_SENSOR_CONFIG, SI7021_SENSOR_DEFAULT);
  GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, SI7021_SENSOR_CONFIG, SI7021_SENSOR_DEFAULT);

  // Configure SI7021 SDA SCL
  GPIO_PinModeSet(SHTC3_SCL_PORT, SHTC3_SCL_PIN,  SI7021_SENSOR_CONFIG, SI7021_SENSOR_DEFAULT);
  GPIO_PinModeSet(SHTC3_SDA_PORT, SHTC3_SDA_PIN, SI7021_SENSOR_CONFIG, SI7021_SENSOR_DEFAULT);
}

/***************************************************************************//**
 * @brief
 *   IRQ Handler for the button 1 interrupt.
 *
 * @details
 *   Automatically called when the button 1 interrupt is activated. Once called,
 *   it clears the interrupt and adds the button 1 event to the scheduler.
 *
 * @note
 *   This function runs when button 1 is pressed.
 *
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void) {
  uint32_t int_flag = GPIO->IF & GPIO->IEN;
  GPIO->IFC = int_flag; // Clear IF register
  add_scheduled_event(gpio_odd_irq_cb);
}

/***************************************************************************//**
 * @brief
 *   IRQ Handler for the button 0 interrupt.
 *
 * @details
 *   Automatically called when the button 0 interrupt is activated. Once called,
 *   it clears the interrupt and adds the button 0 event to the scheduler.
 *
 * @note
 *   This function runs when button 0 is pressed.
 *
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void) {
  uint32_t int_flag = GPIO->IF & GPIO->IEN;
  GPIO->IFC = int_flag; // Clear IF register
  add_scheduled_event(gpio_even_irq_cb);
}
