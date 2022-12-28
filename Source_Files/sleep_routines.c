/**************************************************************************
 * @file sleep.c
 * @author Branson Camp
 * @date 10/03/22
 * @brief Manages the energy state of the processor by
 * choosing the lowest energy state that can be used by
 * all peripherals in use.
 ***************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
**************************************************************************/

#include "sleep_routines.h"

static int lowest_energy_mode[MAX_ENERGY_MODES];

/***************************************************************************//**
 * @brief
 *   Initializes this module.
 *
 * @details
 *   Initializes this module by resetting the lowest energy mode array. After
 *   this function is called, no sleep mode will be blocked.
 *
 * @note
 *   This function should be called before you start using this module.
 *
 ******************************************************************************/
void sleep_open() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  for (int i = 0; i<MAX_ENERGY_MODES; i++) {
      lowest_energy_mode[i] = 0;
  }
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Blocks from entering specified sleep mode
 *
 * @details
 *   Blocks entering specified sleep mode, from EM0 to EM4
 *
 * @note
 *   Once the peripheral is no longer used, the same energy mode must be unblocked
 *   (See sleep_unblock_mode)
 *
 * @param[in] EM
 *   Desired energy mode to unblock
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  lowest_energy_mode[EM]++;
  CORE_EXIT_CRITICAL();
  EFM_ASSERT (lowest_energy_mode[EM] < 5);
}

/***************************************************************************//**
 * @brief
 *   Unblocks previous block from specified sleep mode
 *
 * @details
 *   Unblocks previous block from specified sleep mode, from EM0 to EM4
 *
 * @note
 *   Specified sleep mode must have been blocked in the past in order to unblock.
 *
 * @param[in] EM
 *   Desired energy mode to unblock
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  lowest_energy_mode[EM]--;
  CORE_EXIT_CRITICAL();
  EFM_ASSERT (lowest_energy_mode[EM] >= 0);
}

/***************************************************************************//**
 * @brief
 *   Enters the lowest energy mode allowed.
 *
 * @details
 *   Enters the lowest energy mode that has not been blocked by a peripheral.
 *
 * @note
 *   This function should be called when there are no scheduled events.
 *
 ******************************************************************************/
void enter_sleep() {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (lowest_energy_mode[EM0]) {
      CORE_EXIT_CRITICAL();
      return;
  }

  if (lowest_energy_mode[EM1]) {
      CORE_EXIT_CRITICAL();
      return;
  }

  if (lowest_energy_mode[EM2]) {
      EMU_EnterEM1();
      CORE_EXIT_CRITICAL();
      return;
  }

  if (lowest_energy_mode[EM3]) {
      EMU_EnterEM2(true);
      CORE_EXIT_CRITICAL();
      return;
  }

  EMU_EnterEM3(true);
  CORE_EXIT_CRITICAL();
  return;
}

/***************************************************************************//**
 * @brief
 *   Get the highest energy mode that is currently blocked.
 *
 * @details
 *   Returns the first nonzero block energy count in the block_energy_modes array.
 *
 * @note
 *   This function can be called at any time.
 *
 * @return
 *   The index of the highest energy mode that is currently blocked.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  uint32_t selected_em = EM4;
  bool done = false;
  for (int i = 0; i < MAX_ENERGY_MODES; i++) {
      if (!done) {
          if (lowest_energy_mode[i] > 0) {
              done = true;
              selected_em = i;
          }
      }
  }
  CORE_EXIT_CRITICAL();
  return selected_em;
}

