/*
 * GPIO.c
 *
 *  Created on: Jul 3, 2019
 *      Author: AVE-LAP-016
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "../FreeRTOS_Files/FreeRTOS.h"
#include "../FreeRTOS_Files/task.h"
#include "../FreeRTOS_Files/semphr.h"
#include "../CAN/CAN.h"

#include "GPIO.h"

extern SemaphoreHandle_t xSemaphore; /*Handler of counting semphore*/
extern TaskHandle_t handler_init_GPIO; /*Handler of GPIO_init task */

/* toggle_LED_one task for toggol LED & increment delay depend on boutton pressing
 * input ; void
 * output ; void */
void toggle_LED_one(void *x)
{
    configASSERT(((uint32_t)x)==1);
    while (1)
    {
        if (uxSemaphoreGetCount( xSemaphore ) == 2)
        {

            /* Turn on the LED.*/

            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);
            vTaskDelay(1000); /* task delay */

            /* Turn off the LED.*/

            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);

            xSemaphoreTake(xSemaphore, 0);
        }
        vTaskDelay(50); /* task delay */
    }
}

/* GPIO init task for initialize system clock for GPIO
 * input ; void
 * output ; void */
void init_GPIO(void *x)
{
    configASSERT(((uint32_t)x)==1);
    /* Enable the GPIO port that is used for the on-board LED.*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    /* Check if the peripheral access is enabled.*/
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    /* Enable the GPIO pin for the LED (PF3).  Set the direction as output, and*/
    /* enable the GPIO pin for digital function.*/
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    vTaskSuspend(handler_init_GPIO);/*suspend task*/

}

