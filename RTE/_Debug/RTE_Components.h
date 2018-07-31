
/*
 * Auto generated Run-Time-Environment Component Configuration File
 *      *** Do not modify ! ***
 *
 * Project: 'MassStorage' 
 * Target:  'Debug' 
 */

#ifndef RTE_COMPONENTS_H
#define RTE_COMPONENTS_H


/*
 * Define the Device Header File: 
 */
#define CMSIS_device_header "stm32f4xx.h"

#define RTE_CMSIS_RTOS2                 /* CMSIS-RTOS2 */
        #define RTE_CMSIS_RTOS2_RTX5            /* CMSIS-RTOS2 Keil RTX5 */
#define RTE_Compiler_EventRecorder
          #define RTE_Compiler_EventRecorder_DAP
#define RTE_DEVICE_FRAMEWORK_CLASSIC
#define RTE_DEVICE_HAL_CAN
#define RTE_DEVICE_HAL_COMMON
#define RTE_DEVICE_HAL_CORTEX
#define RTE_DEVICE_HAL_DMA
#define RTE_DEVICE_HAL_GPIO
#define RTE_DEVICE_HAL_PWR
#define RTE_DEVICE_HAL_RCC
#define RTE_DEVICE_HAL_SRAM
#define RTE_DEVICE_STARTUP_STM32F4XX    /* Device Startup for STM32F4 */
#define RTE_Drivers_CAN1                /* Driver CAN1 */
        #define RTE_Drivers_CAN2                /* Driver CAN2 */
        #define RTE_Drivers_CAN3                /* Driver CAN3 */
#define RTE_Drivers_USBD1               /* Driver USBD1 */
#define RTE_USB_Core                    /* USB Core */
          #define RTE_USB_Core_Debug              /* USB Core Debug Version */
#define RTE_USB_Device_0                /* USB Device 0 */
#define RTE_USB_Device_CustomClass_0    /* USB Device Custom Class instance 0 */

#endif /* RTE_COMPONENTS_H */
