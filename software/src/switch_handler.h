#ifndef SWITCH_HANDLER_H
#define SWITCH_HANDLER_H

#include <Arduino.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

// declarations
#define GPIO_STACK_SIZE 4096 // Stack size for the task
#define TASK_PRIORITY_SWITCH 3    // Task priority
#define BYPASS_PIN 25 // GPIO pin for the button
#define HI_LO_PIN 27 // GPIO pin for the high/low signal output pin

// public

/* * @brief Setup the bypass task
 * This function sets up the bypass task and initializes the GPIO pins.
 * @return true if the task was created successfully, false otherwise
 */
bool setup_bypass_task(void);

// private 

/* * @brief Bypass event loop task
 * This task handles the bypass button press events and toggles the hi_lo pin state.
 * On button press, a logic level change is sent to the hi_lo pin, switching the MUX.
 * @param pvParameters Pointer to task parameters
 */ 
void bypass_event_loop(void *pvParameters);

#endif // SWITCH_HANDLER_H