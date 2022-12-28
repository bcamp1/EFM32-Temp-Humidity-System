//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "SI7021.h"
#include "shtc3.h"
#include "scheduler.h"

#include <stdio.h>


//***********************************************************************************
// defined files
//***********************************************************************************
#define   PWM_PER       3   // PWM period in seconds
#define   PWM_ACT_PER     0.25  // PWM active period in seconds
//#define   PWM_ROUTE_0    28
//#define   PWM_ROUTE_1    0

#define LETIMER0_COMP0_CB   0x001
#define LETIMER0_COMP1_CB   0x002
#define LETIMER0_UF_CB      0x004
#define GPIO_EVEN_IRQ_CB    0x008
#define GPIO_ODD_IRQ_CB     0x010
#define SI7021_READ_HUM_CB  0x020
#define SI7021_READ_TEMP_CB 0x040
#define SHTC3_READ_CB       0x080
#define SI7021_USER_CONFIRM 0x100

#define HUMIDITY_COMPARE  30.0


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);

void scheduled_letimer0_uf_cb (void);
void scheduled_letimer0_comp0_cb (void);
void scheduled_letimer0_comp1_cb (void);

void scheduled_gpio_odd_irq_cb (void);
void scheduled_gpio_even_irq_cb (void);

void scheduled_si7021_read_hum_cb(void);
void scheduled_si7021_read_temp_cb(void);

void scheduled_shtc3_read_irq_cb(void);

void scheduled_si7021_user_confirm(void);

#endif
