/*****************************************************
 * @file i2c.c
 * @author Branson Camp
 * @date 10/27/2022
 * @brief Sets up and configures both I2C peripherals
 *
 ****************************************************/
#include "i2c.h"

typedef enum {
  initialize,
  send_register,
  request_read,
  read_data,
  write_data,
  send_stop,
  end_process,
} DEFINED_STATES;

typedef struct {
  DEFINED_STATES current_state; // Which state the state machine is in
  bool which_i2c; // false = I2C0, true = I2C1
  uint32_t device_address; // Slave address
  uint32_t register_address; // I2c register command
  uint32_t num_bytes; // Number of bytes to read or write
  uint32_t finished_callback; // event cb after i2c is finished
  I2C_COMM_METHOD_TypeDef comm_method; // read or write
  bool busy; // Is the state machine busy?
  uint32_t* data;
  uint32_t byte_counter; // Counts down from num_bytes

  uint32_t num_register_bytes;
  uint32_t register_byte_counter;

} I2C_STATE_MACHINE_STRUCT;

static I2C_STATE_MACHINE_STRUCT i2c0_state_machine;
static I2C_STATE_MACHINE_STRUCT i2c1_state_machine;

/***************************************************************************//**
 * @brief
 *   Service routine for when an ACK is received from a slave
 *
 * @details
 *   This function is called when an ACK is received from a slave. It updates
 *   the I2C's state based on the previous state
 *
 *
 * @note
 *   This function should not be called from outside the I2C module.
 *
 * @param[in] i2c_sm
 *  I2C State Machine struct
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 *
 ******************************************************************************/
static void i2c_ack_sm(I2C_STATE_MACHINE_STRUCT *i2c_sm, I2C_TypeDef *i2cx) {
  switch (i2c_sm->current_state) {
    case initialize:
      i2c_sm->current_state = send_register;
      if (i2c_sm->register_byte_counter > 0) {
          // Sends data from most significant to least significant bytes
          uint32_t data_copy = i2c_sm->register_address;

          // Which byte to send: 0 = LSB, max = MSB
          uint32_t which_byte = i2c_sm->register_byte_counter - 1;
          uint32_t byte_mask = 0xFF << (8 * which_byte); // Masks the correct byte
          data_copy &= byte_mask;
          data_copy >>= 8 * which_byte; // Consolidate to one byte for writing
          i2cx->TXDATA = data_copy; // Write the byte
          i2c_sm->register_byte_counter--; // Decrement byte counter

          if (i2c_sm->register_byte_counter > 0) {
              i2c_sm->current_state = initialize;
          }
      } else {
          // Sends data from most significant to least significant bytes
          uint32_t data_copy = *i2c_sm->data;

          // Which byte to send: 0 = LSB, max = MSB
          uint32_t which_byte = i2c_sm->byte_counter - 1;
          uint32_t byte_mask = 0xFF << (8 * which_byte); // Masks the correct byte
          data_copy &= byte_mask;
          data_copy >>= 8 * which_byte; // Consolidate to one byte for writing
          i2cx->TXDATA = data_copy; // Write the byte
          i2c_sm->byte_counter--; // Decrement byte counter
      }
      break;
    case send_register:
      if (i2c_sm->comm_method == I2C_READ) {
        i2c_sm->current_state = request_read;
        i2cx->CMD = I2C_CMD_START; // Repeated Start
        i2cx->TXDATA = (i2c_sm->device_address << 1) | I2C_R; // Device Addr + R
      } else {
        i2c_sm->current_state = write_data;
        // Sends data from most significant to least significant bytes
        uint32_t data_copy = *i2c_sm->data;

        // Which byte to send: 0 = LSB, max = MSB
        uint32_t which_byte = i2c_sm->byte_counter - 1;
        uint32_t byte_mask = 0xFF << (8 * which_byte); // Masks the correct byte
        data_copy &= byte_mask;
        data_copy >>= 8 * which_byte; // Consolidate to one byte for writing
        i2cx->TXDATA = data_copy; // Write the byte
        i2c_sm->byte_counter--; // Decrement byte counter

      }
      break;
    case request_read:
      EFM_ASSERT(i2c_sm->comm_method == I2C_READ);
      i2c_sm->current_state = read_data;
        //i2cx->CMD = I2C_CMD_STOP;
        //i2c_sm->current_state = send_stop;
      break;
    case read_data:
      EFM_ASSERT(false); // Not in our design ladder
      break;
    case write_data:
      EFM_ASSERT(i2c_sm->comm_method == I2C_WRITE);

      if (i2c_sm->byte_counter > 0) {
        // Sends data from most significant to least significant bytes
        uint32_t data_copy = *i2c_sm->data;

        // Which byte to send: 0 = LSB, max = MSB
        uint32_t which_byte = i2c_sm->byte_counter - 1;
        uint32_t byte_mask = 0xFF << (8 * which_byte); // Masks the correct byte
        data_copy &= byte_mask;
        data_copy >>= 8 * which_byte; // Consolidate to one byte for writing
        i2cx->TXDATA = data_copy; // Write the byte
        i2c_sm->byte_counter--; // Decrement byte counter
      } else {
        // Writing is done. Send Stop Command.
        i2cx->CMD = I2C_CMD_STOP;
        i2c_sm->current_state = send_stop;
      }
      break;
    case send_stop:
      EFM_ASSERT(false); // Not in our design ladder
      break;
    case end_process:
      EFM_ASSERT(false); // Not in our design ladder
      break;
    default:
      EFM_ASSERT(false); // Not in our design ladder
      break;
  }
}

/***************************************************************************//**
 * @brief
 *   Service routine for when an NACK is received from a slave
 *
 * @details
 *   This function is called when an NACK is received from a slave. It resends
 *   the previous command until an ACK is reveived.
 *
 *
 * @note
 *   This function should not be called from outside the I2C module.
 *
 * @param[in] i2c_sm
 *  I2C State Machine struct
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 *
 ******************************************************************************/
static void i2c_nack_sm(I2C_STATE_MACHINE_STRUCT *i2c_sm, I2C_TypeDef *i2cx) {
  EFM_ASSERT(i2c_sm->current_state == request_read);
  i2cx->CMD = I2C_CMD_START; // Repeated Start
  i2cx->TXDATA = (i2c_sm->device_address << 1) | I2C_R; // Device Addr + R
}

/***************************************************************************//**
 * @brief
 *   Service routine for when an STOP is executed on the bus
 *
 * @details
 *   This function takes care of freeing the state machine's busy lock
 *   and scheduling the event that the operation is complete
 *
 * @note
 *   This function should not be called from outside the I2C module.
 *
 * @param[in] i2c_sm
 *  I2C State Machine struct
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 *
 ******************************************************************************/
static void i2c_stop_sm(I2C_STATE_MACHINE_STRUCT *i2c_sm, I2C_TypeDef *i2cx) {
  EFM_ASSERT(i2c_sm->current_state == send_stop);
  i2c_sm->current_state = end_process;
  add_scheduled_event(i2c_sm->finished_callback);
  i2c_sm->busy = false;
  sleep_unblock_mode(I2C_EM);
}

/***************************************************************************//**
 * @brief
 *   Service routine for when RX data is received from the slave.
 *
 * @details
 *   This function is called when data is recieved from a slave. Data is
 *   allocated one byte at a time.
 *
 * @note
 *   This function should not be called from outside the I2C module.
 *
 * @param[in] i2c_sm
 *  I2C State Machine struct
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 *
 ******************************************************************************/
static void i2c_rxdatav_sm(I2C_STATE_MACHINE_STRUCT *i2c_sm, I2C_TypeDef *i2cx) {
  uint32_t rxdata = i2cx->RXDATA;
  if (i2c_sm->byte_counter > 0) {
    *(i2c_sm->data) = *(i2c_sm->data) << 8;
    *(i2c_sm->data) |= rxdata;
    i2c_sm->byte_counter--;
    i2cx->CMD = I2C_CMD_ACK;
  } else {
    i2cx->CMD = I2C_CMD_NACK;
    i2cx->CMD = I2C_CMD_STOP;
    i2c_sm->current_state = send_stop;
  }
}

/***************************************************************************//**
 * @brief
 *   Resets the I2C bus.
 *
 * @details
 *   Performs an I2C bus reset. Sends stop and abort commands.
 *
 * @note
 *   Call this function to reset the bus.
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 *
 ******************************************************************************/
static void i2c_bus_reset(I2C_TypeDef *i2cx) {
  uint32_t IEN_state = i2cx->IEN;
  i2cx->IEN = 0; // Disable all interupts
  i2cx->IFC = ~0; // Clear all interrupts
  i2cx->CMD = I2C_CMD_CLEARTX; // Clear transmit buffer

  // Simultaneous start/stop to reset
  i2cx->CMD = I2C_CMD_START;
  i2cx->CMD = I2C_CMD_STOP;

  // Stall until MSTOP is set
  while(!(i2cx->IF & I2C_IF_MSTOP));
  i2cx->IFC = ~0; // Clear all interrupts

  i2cx->CMD = I2C_CMD_ABORT; // Set abort bit
  i2cx->IEN = IEN_state; // Restore IEN state
}

/***************************************************************************//**
 * @brief
 *   Starts the i2c process and sends the first commands to the slave.
 *
 * @details
 *   Starts the read or write operation and prepares to receive an ACK.
 *
 * @note
 *   This function should be called after this module is initialized with i2c_open
 *
 * @param[in] i2c_start
 *  I2C Start struct which includes necessary information for the I2C
 *  state machine such as device address, register address, data bytes, etc.
 ******************************************************************************/
void i2c_start(I2C_START_STRUCT *i2c_start) {
  I2C_STATE_MACHINE_STRUCT *i2cx_state_machine;
  I2C_TypeDef *i2cx;
  if (i2c_start->which_i2c) {
      i2cx_state_machine = &i2c1_state_machine;
      i2cx = I2C1;
  } else {
      i2cx_state_machine = &i2c0_state_machine;
      i2cx = I2C0;
  }

  // Wait till i2c is not busy
  while(i2cx_state_machine->busy);

  EFM_ASSERT((i2cx->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

  // Block appropriate sleep mode
  sleep_block_mode(I2C_EM);

  // Fill state machine struct
  i2cx_state_machine->busy = true;
  i2cx_state_machine->current_state = initialize;
  i2cx_state_machine->which_i2c = i2c_start->which_i2c;
  i2cx_state_machine->device_address = i2c_start->device_address;
  i2cx_state_machine->register_address = i2c_start->register_address;
  i2cx_state_machine->num_bytes = i2c_start->num_bytes;
  i2cx_state_machine->finished_callback = i2c_start->finished_callback;
  i2cx_state_machine->comm_method = i2c_start->comm_method;
  i2cx_state_machine->data = i2c_start->data;
  i2cx_state_machine->byte_counter = i2c_start->num_bytes;
  i2cx_state_machine->num_register_bytes = i2c_start->num_register_bytes;
  i2cx_state_machine->register_byte_counter = i2c_start->num_register_bytes;

  i2cx->CMD = I2C_CMD_START; // Start
  i2cx->TXDATA = (i2c_start->device_address << 1) | I2C_W; // Give device address + W
}

/***************************************************************************//**
 * @brief
 *   Initialized the I2C module and prepares for the operation.
 *
 * @details
 *   Initializes the I2C including setting the clock tree and interrupts
 *
 * @note
 *   Call this function before starting any I2C operations
 *
 * @param[in] I2Cx
 *  I2C peripheral struct
 *
 * @param[in] i2c_setup
 *  Struct containing the necessary properties to initialize this module.
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *I2Cx, I2C_OPEN_STRUCT *i2c_setup) {
  // Enable Clock
  if (I2Cx == I2C0) {
      CMU_ClockEnable(cmuClock_I2C0, true);
  } else if (I2Cx == I2C1) {
      CMU_ClockEnable(cmuClock_I2C1, true);
  }

  // Test IF Register
  if ((I2Cx->IF & 0x01) == 0) { I2Cx->IFS = 0x01;
    EFM_ASSERT(I2Cx->IF & 0x01);
    I2Cx->IFC = 0x01;
  } else {
    I2Cx->IFC = 0x01;
    EFM_ASSERT(!(I2Cx->IF & 0x01));
  }

  // Init I2Cx
  I2C_Init_TypeDef i2c_init;
  i2c_init.master = i2c_setup->master;
  i2c_init.enable = i2c_setup->enable;
  i2c_init.clhr = i2c_setup->clhr;
  i2c_init.freq = i2c_setup->freq;
  i2c_init.refFreq = i2c_setup->refFreq;

  I2C_Init(I2Cx, &i2c_init);

  // Pin Routing
  // Route Locations
  I2Cx->ROUTELOC0 |= i2c_setup->scl_route_pin;
  I2Cx->ROUTELOC0 |= i2c_setup->sda_route_pin;

  // Route Enables
  I2Cx->ROUTEPEN |= I2C_ROUTEPEN_SCLPEN;
  I2Cx->ROUTEPEN |= I2C_ROUTEPEN_SDAPEN;

  // NVIC Enable Interrupts
  if (I2Cx == I2C0) {
      NVIC_EnableIRQ(I2C0_IRQn);
      i2c0_state_machine.busy = false;
  } else if (I2Cx == I2C1) {
      NVIC_EnableIRQ(I2C1_IRQn);
      i2c1_state_machine.busy = false;
  }

  // Interrupt Enables
  I2Cx->IEN |= (I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_MSTOP | I2C_IEN_RXDATAV);

  // Perform Bus Reset
  i2c_bus_reset(I2Cx);
}



/***************************************************************************//**
 * @brief
 *   Service routine for I2C interrupts
 *
 * @details
 *   Service routine for when either I2C0 or I2C1 receives an interrupt and directs
 *   those interrupts to the corresponding state machine function.
 *
 * @note
 *   This function should not be called from outside the I2C module.
 *
 * @param[in] i2cx
 *  I2C peripheral struct
 ******************************************************************************/
static void i2c_isr(I2C_TypeDef *i2cx) {
  I2C_STATE_MACHINE_STRUCT *i2c_sm;
  if (i2cx == I2C0) {
      i2c_sm = &i2c0_state_machine;
  } else if (i2cx == I2C1) {
      i2c_sm = &i2c1_state_machine;
  } else {
      EFM_ASSERT(false);
  }

  uint32_t int_flag = i2cx->IF & i2cx->IEN;
  i2cx->IFC = int_flag; // Clear IF register


  if (int_flag & I2C_IF_ACK) {
      EFM_ASSERT(!(i2cx->IF & I2C_IF_ACK));
      i2c_ack_sm(i2c_sm, i2cx);

  }

  if (int_flag & I2C_IF_NACK) {
      EFM_ASSERT(!(i2cx->IF & I2C_IF_NACK));
      i2c_nack_sm(i2c_sm, i2cx);
  }

  if (int_flag & I2C_IF_RXDATAV) {
      i2c_rxdatav_sm(i2c_sm, i2cx);
  }

  if (int_flag & I2C_IF_MSTOP) {
      EFM_ASSERT(!(i2cx->IF & I2C_IF_MSTOP));
      i2c_stop_sm(i2c_sm, i2cx);
  }
}

/***************************************************************************//**
 * @brief
 *   Interrupt handler for I2C0.
 *
 * @details
 *   Directs the interrupt to the interrupt service routine.
 *
 * @note
 *   This function is automatically called when I2C0 has an interrupt.
 ******************************************************************************/
void I2C0_IRQHandler() {
  i2c_isr(I2C0);
}

/***************************************************************************//**
 * @brief
 *   Interrupt handler for I2C1.
 *
 * @details
 *   Directs the interrupt to the interrupt service routine.
 *
 * @note
 *   This function is automatically called when I2C1 has an interrupt.
 ******************************************************************************/
void I2C1_IRQHandler() {
  i2c_isr(I2C1);
}



