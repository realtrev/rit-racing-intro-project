#include "main.h"

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

#include <stm32g4xx_hal.h>

static void blink_port(const int ms, GPIO_TypeDef *gpio_loc, const uint16_t pin)
{
    while(true) {
        core_GPIO_digital_write(gpio_loc, pin, true);
        vTaskDelay(ms / 2.0 * portTICK_PERIOD_MS);
        core_GPIO_digital_write(gpio_loc, pin, false);
        vTaskDelay(ms / 2.0 * portTICK_PERIOD_MS);
    }
}

static void GPIO_task_1(void *pvParameters) {
    (void) pvParameters;
    blink_port(1000, GPIOA, GPIO_PIN_8);
}

static void GPIO_task_2(void *pvParameters) {
    (void) pvParameters;
    blink_port(3000, GPIOA, GPIO_PIN_9);
}

int main() {
    HAL_Init();

    // Drivers
    // core_heartbeat_init(GPIOA, GPIO_PIN_8);
    // core_GPIO_set_heartbeat(GPIO_PIN_RESET);

    core_GPIO_init(GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOA, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    // core_GPIO_init(GPIOA, GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    if (!core_clock_init()) error_handler();
    if (!core_CAN_init(FDCAN1, 1000000)) error_handler();

    // int err = xTaskCreate(heartbeat_task, "heartbeat", 1000, NULL, 4, NULL);

    int err1 = xTaskCreate(GPIO_task_1, "gpio1", 1000, NULL, 4, NULL);
    if (err1 != pdPASS) {
        error_handler();
    }

    int err2 = xTaskCreate(GPIO_task_2, "gpio2", 1000, NULL, 4, NULL);
    if (err2 != pdPASS) {
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
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName) {
    (void) xTask;
    (void) pcTaskName;

    error_handler();
}
