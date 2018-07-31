#include "main.h"
#include "Driver_CAN.h"
#include "RTE_Components.h"
#include "queue.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "rl_usb.h"                     /* RL-USB function prototypes         */

extern queue_t *q_frame_pool;
extern queue_t *q_from_host;
extern queue_t *q_to_host;

#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif
#ifdef RTE_CMSIS_RTOS2                  // when RTE component CMSIS RTOS2 is used
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif

#define APP_MAIN_STK_SZ (1024U)
uint64_t app_main_stk[APP_MAIN_STK_SZ / 8];
const osThreadAttr_t app_main_attr = {
  .stack_mem  = &app_main_stk[0],
  .stack_size = sizeof(app_main_stk)
};

// CAN Driver Controller selector
#define  CAN_CONTROLLER         1       // CAN Controller number
#define CAN_QUEUE_SIZE 					16
 
#define _CAN_Driver_(n)         Driver_CAN##n
#define  CAN_Driver_(n)        _CAN_Driver_(n)
extern   ARM_DRIVER_CAN         CAN_Driver_(CAN_CONTROLLER);
#define  ptrCAN               (&CAN_Driver_(CAN_CONTROLLER))
uint32_t                        rx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         rx_data[8];
ARM_CAN_MSG_INFO                rx_msg_info;
uint32_t                        tx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         tx_data[8];
ARM_CAN_MSG_INFO                tx_msg_info;

static bool writeNewData = 0;

static void Error_Handler(){}

//Temp Functions to do stuff
static void signalUnitEvent(uint32_t event)
{
	//Do Nothing with event for now
}
void CAN_SignalObjectEvent (uint32_t obj_idx, uint32_t event) {
 
  if (obj_idx == rx_obj_idx) {                  // If receive object event
    if (event == ARM_CAN_EVENT_RECEIVE) {       // If message was received successfully
      if (ptrCAN->MessageRead(rx_obj_idx, &rx_msg_info, rx_data, 8U) > 0U) {
				if(!writeNewData)
				{
					if((rx_msg_info.id & 0X7FFFFFFF) == 0x12345678)
					{
						tx_data[0] = rx_data[0];
						tx_data[1] = rx_data[1];
						tx_data[2] = rx_data[2];
						tx_data[3] = rx_data[3];
						writeNewData = 1;
					}
				}
				
				// Read received message
        // process received message ...
      }
    }
  }
  if (obj_idx == tx_obj_idx) {                  // If transmit object event
    if (event == ARM_CAN_EVENT_SEND_COMPLETE) { // If message was sent successfully
      // acknowledge sent message ...
    }
  }
}

__NO_RETURN void loop(void *arg)
{
  (void)arg;
	
  USBD_Initialize(0U);                  /* USB Device 0 Initialization        */
  USBD_Connect   (0U);                  /* USB Device 0 Connect               */
	
	q_from_host  = queue_create(CAN_QUEUE_SIZE);
	q_frame_pool = queue_create(CAN_QUEUE_SIZE);
	q_to_host    = queue_create(CAN_QUEUE_SIZE);
	
	ARM_CAN_CAPABILITIES     can_cap;
  ARM_CAN_OBJ_CAPABILITIES can_obj_cap;
  int32_t                  status;
  uint32_t                 i, num_objects;
 
  can_cap = ptrCAN->GetCapabilities (); // Get CAN driver capabilities
  num_objects = can_cap.num_objects;    // Number of receive/transmit objects
 
  status = ptrCAN->Initialize    (signalUnitEvent, CAN_SignalObjectEvent);  // Initialize CAN driver
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  status = ptrCAN->PowerControl  (ARM_POWER_FULL);                              // Power-up CAN controller
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  status = ptrCAN->SetMode       (ARM_CAN_MODE_INITIALIZATION);                 // Activate initialization mode
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  status = ptrCAN->SetBitrate    (ARM_CAN_BITRATE_NOMINAL,              // Set nominal bitrate
                                  1000000U,                              // Set bitrate to 1000 kbit/s
                                  ARM_CAN_BIT_PROP_SEG(7U)   |          // Set propagation segment to 5 time quanta
                                  ARM_CAN_BIT_PHASE_SEG1(8U) |          // Set phase segment 1 to 11 time quantum 
                                  ARM_CAN_BIT_PHASE_SEG2(5U) |          // Set phase segment 2 to 3 time quantum 
                                  ARM_CAN_BIT_SJW(4U));                 // Resynchronization jump width is 4
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  for (i = 0U; i < num_objects; i++) {                                          // Find first available object for receive and transmit
    can_obj_cap = ptrCAN->ObjectGetCapabilities (i);                            // Get object capabilities
    if      ((rx_obj_idx == 0xFFFFFFFFU) && (can_obj_cap.rx == 1U)) { rx_obj_idx = i; }
    else if ((tx_obj_idx == 0xFFFFFFFFU) && (can_obj_cap.tx == 1U)) { tx_obj_idx = i; break; }
  }
  if ((rx_obj_idx == 0xFFFFFFFFU) || (tx_obj_idx == 0xFFFFFFFFU)) { Error_Handler(); }
 
  // Set filter to receive messages with extended ID 0x12345678 to receive object
  status = ptrCAN->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_EXTENDED_ID(0X12345678U), 0U);
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  status = ptrCAN->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX);                 // Configure transmit object
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
 
  status = ptrCAN->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX);                 // Configure receive object
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
  
  status = ptrCAN->SetMode (ARM_CAN_MODE_NORMAL);                               // Activate normal operation mode
  if (status != ARM_DRIVER_OK ) { Error_Handler(); }
	
  tx_msg_info.id = ARM_CAN_EXTENDED_ID(0x330033U);                            // Set extended ID for transmit message
  memset(tx_data, 0, 8);                                                       // Initialize transmit data

	
	
  while(1) {                            /* Loop forever                       */
		
		//If new frame from USB, send it on CAN
		struct gs_host_frame *frame = queue_pop_front(q_from_host);
		if (frame != 0) { // send can message from host
			tx_msg_info.id = frame->can_id;
			tx_msg_info.dlc = frame->can_dlc;
			
			if (ptrCAN->MessageSend(tx_obj_idx, &tx_msg_info, frame->data, 8) > 0) {
				
				// Echo sent frame back to host
				frame->timestamp_us = HAL_GetTick();
				//send_to_host_or_enqueue(frame); //TODO: Add Echos to Host
				
				//led_indicate_trx(&hLED, led_2); //TODO: Add LED Indicators to HERO
				
				free(frame);
			} else {
				//Didn't send any bytes of data, try again later
				queue_push_front(q_from_host, frame);
			}
		}
		
		osDelay(10);
		//ptrCAN->MessageSend(tx_obj_idx, &tx_msg_info, frame->data, 8);
		
  }
}

