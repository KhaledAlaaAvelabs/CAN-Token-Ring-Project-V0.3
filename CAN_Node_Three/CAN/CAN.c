/*
 * CAN.c
 *
 *  Created on: Jul 3, 2019
 *      Author: AVE-LAP-016
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_can.h"

#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"

#include "driverlib/can.h"
#include "driverlib/interrupt.h"

#include "../FreeRTOS_Files/FreeRTOS.h"
#include "../FreeRTOS_Files/task.h"

#include "CAN.h"
#include "../FreeRTOS_Files/semphr.h"
extern SemaphoreHandle_t xSemaphore;

extern TaskHandle_t handler_init_CAN_Tx; /*Handler of system_init task */
extern TaskHandle_t handler_init_CAN_Rx; /*Handler of system_init task */
extern TaskHandle_t handler_init_CAN_send; /*Handler of system_init task */
extern TaskHandle_t handler_init_GPIO; /*Handler of GPIO_init task */
UBaseType_t number;

/*tiva1*/
/* CAN TX init task for initialize CAN0 Tx
 * input ; void
 * output ; void */
void CAN_TX_Init(void *x)
{

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    /*configure can pins*/
    GPIOPinConfigure(GPIO_PB4_CAN0RX);
    GPIOPinConfigure(GPIO_PB5_CAN0TX);

    GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_CAN0))
    {
    }
    CANInit(CAN0_BASE);
    CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);
    //  CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
    CANEnable(CAN0_BASE);
    /*creating counter semphore to sync between tasks*/
    xSemaphore = xSemaphoreCreateCounting(4, 0);
    /*After init finished this task will not run again*/
    vTaskSuspend(handler_init_CAN_Tx);

}

/* CAN TX init task for initialize CAN0 Rx
 * input ; void
 * output ; void */
void CAN_RX_Init(void *x)
{
    configASSERT(((uint32_t)x)==1);

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB4_CAN0RX);
    GPIOPinConfigure(GPIO_PB5_CAN0TX);

    GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    CANInit(CAN0_BASE);

    CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);

    CANEnable(CAN0_BASE);
    vTaskSuspend(handler_init_CAN_Rx);/*suspend task*/

}

/* CAN0 Send task for send data with CAN0
 * input ; void
 * output ; void */
void CAN0_Send(void *x)
{

    configASSERT(((uint32_t)x)==1);
    uint8_t i = 0;

    tCANMsgObject sCANMessage;
    sCANMessage.ui32MsgID = 3;
    //   sCANMessage.ui32MsgIDMask = 3;
    sCANMessage.ui32Flags = 0; // MSG_OBJ_TX_INT_ENABLE;
    sCANMessage.ui32MsgLen = 8; // sizeof(pui8MsgData);
    sCANMessage.pui8MsgData = '5';

    while (1)
    {

        if ((uxSemaphoreGetCount( xSemaphore ) == 1))
        {

            /*data need to send more time to ensure data received*/
            for (i = 0; i < 1; i++)
            {

                CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_TX);

            }
            xSemaphoreTake(xSemaphore, 0);

        }

        vTaskDelay(50); /* task delay */
    }

}

/* CAN0 Receive task for receive data with CAN0
 * input ; void
 * output ; void */
void CAN0_Receive(void *x)
{
    configASSERT(((uint32_t)x)==1);

    uint32_t ui32Status;
    tCANMsgObject sCANMessage;
    uint8_t pui8MsgData[8];
    /*configure message object 2 for receiving*/
    sCANMessage.ui32MsgID = 2;
    sCANMessage.ui32MsgIDMask = 2;
    sCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    sCANMessage.ui32MsgLen = 8;

    CANMessageSet(CAN0_BASE, 2, &sCANMessage, MSG_OBJ_TYPE_RX);

    for (;;)
    {
        /*check status to get message*/

        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
        while (!(ui32Status & CAN_STATUS_RXOK))
            ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
        if (uxSemaphoreGetCount( xSemaphore ) == 0)
        {

            sCANMessage.pui8MsgData = pui8MsgData;
            CANMessageGet(CAN0_BASE, 2, &sCANMessage, 0);
            if (sCANMessage.ui32MsgID == 2)
            {
                xSemaphoreGive(xSemaphore);
                xSemaphoreGive(xSemaphore);
            }
        }
        vTaskDelay(50); /* task delay */
    }

}

