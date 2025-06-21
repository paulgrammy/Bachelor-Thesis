#ifndef GPIO_TASK_H
#define GPIO_TASK_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

#include "mode_handler.h"

#define GPIO_STACK_SIZE 4096                                    // Stack size for the task
#define TASK_PRIORITY 5                                         // Task priority
#define BUTTON_PIN 25                                           // GPIO pin for the button
#define STATUS_LED_PIN GPIO_NUM_13                                  // GPIO pin for the status LED


// public

/* Setup GPIO task function
 * This function will set up the GPIO task for the button press.
 * It will create a task that will run in the background and check for GPIO events.
 */
bool setup_gpio_task(void);

//private

/* GPIO event loop function
 * This function is called from the setup_gpio_task() function. It will create a task that will run in the background and check for GPIO events.
 * The task will be created with a stack size of 2048 bytes and a priority of 5.
 * The task will run indefinitely until it is deleted or the ESP32 is reset.
 @param pvParameters - pointer to the task parameters
 */
static void gpio_event_loop(void *pvParameters);

#endif // GPIO_TASK_H