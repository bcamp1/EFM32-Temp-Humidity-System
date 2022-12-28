/*****************************************************
* @file scheduler.c
 * @author Branson Camp
 * @date 10/03/2022
 * @brief Manages the setting and removing of events
 * from any peripheral that needs an event.
 *
 ****************************************************/

#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"

#include "scheduler.h"

static uint32_t event_scheduled;


/***************************************************************************//**
 * @brief
 *   Configures/Resets the scheduler
 *
 * @details
 *   Clears the event scheduled variable so that no events are scheduled.
 *
 * @note
 *   This function should be called when initializing the scheduler.
 *
 ******************************************************************************/
void scheduler_open(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = 0;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Adds event to be scheduled.
 *
 * @details
 *   Adds an event to be scheduled. The event should be one bit shifted x places,
 *   where x is the event id.
 *
 * @note
 *   This function should be called whenever you want to schedule an event.
 *
 * @param[in] event
 *   Desired event to be scheduled.
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled |= event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Removes a scheduled event, if exists.
 *
 * @details
 *   Removes a scheduled event. The event should be one bit shifted x places,
 *   where x is the event id.
 *
 * @note
 *   This function should be called whenever you want to remove an event. Once
 *   an event is handled, it's usually necessary to remove it.
 *
 * @param[in] event
 *   Desired event to be removed from the scheduler.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled &= ~event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Returns scheduled events.
 *
 * @details
 *   Returns scheduled events as an integer. Each bit that's enabled is a different
 *   scheduled event.
 *
 * @note
 *   This function should be called whenever you want to view/check for scheduled
 *   events.
 *
 * @return
 *    The scheduled events.
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void) {
  return event_scheduled;
}




