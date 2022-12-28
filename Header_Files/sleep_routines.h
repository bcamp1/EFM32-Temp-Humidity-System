/*
 * sleep_routines.h
 *
 *  Created on: Sep 22, 2022
 *      Author: bransoncamp
 */

#ifndef SRC_HEADER_FILES_SLEEP_ROUTINES_H_
#define SRC_HEADER_FILES_SLEEP_ROUTINES_H_

#include "em_emu.h"
#include "em_core.h"
#include "em_assert.h"

void sleep_open();
void sleep_block_mode(uint32_t EM);
void sleep_unblock_mode(uint32_t EM);
void enter_sleep();
uint32_t current_block_energy_mode(void);

#define EM0               0
#define EM1               1
#define EM2               2
#define EM3               3
#define EM4               4
#define MAX_ENERGY_MODES  5


#endif /* SRC_HEADER_FILES_SLEEP_ROUTINES_H_ */
