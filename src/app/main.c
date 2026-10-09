#include "main.h"

#include <adc.h>
#include <stdbool.h>
#include <stdio.h>

#include "can.h"
#include "clock.h"
#include "gpio.h"
#include "error_handler.h"
#include "core_config.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "rtt.h"

#include <stm32g4xx_hal.h>

// static void GPIO_task_all(void* pvParameters)
// {
//     (void)pvParameters;
//     while (true)
//     {
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_8, true);
//         vTaskDelay(100 * portTICK_PERIOD_MS);
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_8, false);
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_9, true);
//         vTaskDelay(100 * portTICK_PERIOD_MS);
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_9, false);
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_10, true);
//         vTaskDelay(100 * portTICK_PERIOD_MS);
//         core_GPIO_digital_write(GPIOA, GPIO_PIN_10, false);
//     }
// }

static void print_adc_value(void* pvParameters)
{
    (void)pvParameters;
    int count = 0;
    while (true)
    {
        count++;
        uint16_t potentiometer_value = 0;
        core_ADC_read_channel(GPIOB, GPIO_PIN_12, &potentiometer_value);
        rprintf("Potentiometer%d: %d %d\n", count, potentiometer_value);

        //0 - 1000: turn on  1
        if (potentiometer_value <= 1000){
            core_GPIO_digital_write(GPIOA, GPIO_PIN_8, true);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_9, false);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_10, false);
        }

        // 1500 - 2000 turn on 2
        else if (potentiometer_value >= 1500 && potentiometer_value <= 2000){
            core_GPIO_digital_write(GPIOA, GPIO_PIN_8, true);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_9, true);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_10, false);
        }

        // 3000 - 4000 turn on all three
        else if (potentiometer_value >= 3000 && potentiometer_value <= 4000)
        {
            core_GPIO_digital_write(GPIOA, GPIO_PIN_8, true);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_9, true);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_10, true);
        }

        else
        {
            core_GPIO_digital_write(GPIOA, GPIO_PIN_8, false);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_9, false);
            core_GPIO_digital_write(GPIOA, GPIO_PIN_10, false);
        }

        vTaskDelay(200 * portTICK_PERIOD_MS);
    }
}


int main()
{
    HAL_Init();

    // Drivers
    // core_heartbeat_init(GPIOA, GPIO_PIN_8);
    // core_GPIO_set_heartbeat(GPIO_PIN_RESET);

    core_GPIO_init(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOA, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOA, GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    core_ADC_init(ADC1);
    core_ADC_setup_pin(GPIOB, GPIO_PIN_12, 0);

    if (!core_clock_init()) error_handler();
    if (!core_CAN_init(FDCAN1, 1000000)) error_handler();

    // int err = xTaskCreate(heartbeat_task, "heartbeat", 1000, NULL, 4, NULL);

    // int err1 = xTaskCreate(GPIO_task_all, "gpioAll", 1000, NULL, 4, NULL);
    // if (err1 != pdPASS)
    // {
    //     error_handler();
    // }

    int err2 = xTaskCreate(print_adc_value, "print_adc", 1000, NULL, 4, NULL);
    if (err2 != pdPASS)
    {
        error_handler();
    }

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // hand control over to FreeRTOS
    vTaskStartScheduler();

    // we should not get here ever
    error_handler();
    return 1;
}

// Called when stack overflows from rtos
// Not needed in header, since included in FreeRTOS-Kernel/include/task.h
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    error_handler();
}
