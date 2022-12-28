/*****************************************************
 * @file cmu.c
 * @author Branson Camp
 * @date 9/18/2022
 * @brief Sets up and configures the CMU peripheral
 *
 *****************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Initializes and starts the CMU.
 *
 * @details
 *   Opens the CMU (Clock Management Unit) peripheral. Sets the appropriate clock
 *   frequencies. Also routes the ULFRCO frequency to the LETIMER's clock branch.
 *
 * @note
 *   This function should run once at the start of the program.
 *
 ******************************************************************************/
void cmu_open(void){

    CMU_ClockEnable(cmuClock_HFPER, true);

    // By default, Low Frequency Resistor Capacitor Oscillator, LFRCO, is enabled,
    // Disable the LFRCO oscillator
    CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);  // What is the enumeration required for LFRCO?

    // Disable the Low Frequency Crystal Oscillator, LFXO
    CMU_OscillatorEnable(cmuOsc_LFXO, false, false);  // What is the enumeration required for LFXO?

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

    // Route LF clock to LETIMER0 clock tree
    CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO); // What clock tree does the LETIMER0 reside on?

    // Now, you must ensure that the global Low Frequency is enabled
    CMU_ClockEnable(cmuClock_CORELE, true); //This enumeration is found in the Lab 2 assignment

    //CMU_ClockSelectorSet(cmuClock_I2C0, cmuSelect_HFRCO);
}

