#ifndef BRD_CONFIG_HG
#define BRD_CONFIG_HG

//***********************************************************************************
// Include files
//***********************************************************************************
/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_gpio.h"

/* The developer's include statements */


//***********************************************************************************
// defined files
//***********************************************************************************

// GPIO pin setup
#define STRONG_DRIVE

// LED 0 pin is
#define LED0_PORT         gpioPortF
#define LED0_PIN          04u
#define LED0_DEFAULT      false   // Default false (0) = off, true (1) = on
#define LED0_GPIOMODE     gpioModePushPull

// LED 1 pin is
#define LED1_PORT         gpioPortF
#define LED1_PIN          05u
#define LED1_DEFAULT      false // Default false (0) = off, true (1) = on
#define LED1_GPIOMODE     gpioModePushPull

#ifdef STRONG_DRIVE
  #define LED0_DRIVE_STRENGTH   gpioDriveStrengthStrongAlternateStrong
  #define LED1_DRIVE_STRENGTH   gpioDriveStrengthStrongAlternateStrong
#else
  #define LED0_DRIVE_STRENGTH   gpioDriveStrengthWeakAlternateWeak
  #define LED1_DRIVE_STRENGTH   gpioDriveStrengthWeakAlternateWeak
#endif


// System Clock setup
#define MCU_HFXO_FREQ     cmuHFRCOFreq_32M0Hz


// LETIMER PWM Configuration
#define   PWM_ROUTE_0     28
#define   PWM_ROUTE_1     0

// Button Configuration
#define BUTTON_0_PORT   gpioPortF
#define BUTTON_0_PIN    06u
#define BUTTON_0_CONFIG gpioModeInput // gpio pin mode as input
#define BUTTON_1_PORT   gpioPortF
#define BUTTON_1_PIN    07u
#define BUTTON_1_CONFIG gpioModeInput // gpio pin mode as input
#define BUTTON_DEFAULT  true          // input filter enabled

// Button interrupt configuration
#define BUTTON_0_INT_NUM      BUTTON_0_PIN
#define BUTTON_0_INT_RISING   false
#define BUTTON_0_INT_FALLING  true
#define BUTTON_0_INT_ENABLE   true
#define BUTTON_1_INT_NUM      BUTTON_1_PIN // Pin # for button 1
#define BUTTON_1_INT_RISING   false // Do not trigger interrupt on rising edge
#define BUTTON_1_INT_FALLING  true // Trigger interrupt on falling edge
#define BUTTON_1_INT_ENABLE   true // enable interrupt

// SI7021 Configuration
#define SI7021_SCL_PORT       gpioPortC
#define SI7021_SCL_PIN        11u
#define SI7021_SDA_PORT       gpioPortC
#define SI7021_SDA_PIN        10u
#define SI7021_SENSOR_EN_PORT gpioPortB
#define SI7021_SENSOR_EN_PIN  10u
#define SI7021_SENSOR_DEFAULT true
#define SI7021_SENSOR_CONFIG  gpioModeWiredAnd
#define SI7021_WHICH_I2C      0

// SHTC3 Configuration
#define SHTC3_WHICH_I2C       1
#define SHTC3_SCL_PORT        gpioPortB
#define SHTC3_SCL_PIN         7u
#define SHTC3_SDA_PORT        gpioPortB
#define SHTC3_SDA_PIN         6u

// I2C Route Pins
#define I2C0_SDA_ROUTE I2C_ROUTELOC0_SDALOC_LOC15
#define I2C0_SCL_ROUTE I2C_ROUTELOC0_SCLLOC_LOC15
//#define I2C1_SDA_ROUTE I2C_ROUTELOC0_SDALOC_LOC19
//#define I2C1_SCL_ROUTE I2C_ROUTELOC0_SCLLOC_LOC19

#define I2C1_SDA_ROUTE I2C_ROUTELOC0_SDALOC_LOC6
#define I2C1_SCL_ROUTE I2C_ROUTELOC0_SCLLOC_LOC6

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************

#endif
