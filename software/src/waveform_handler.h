#ifndef WAVEFORM_HANDLER_H
#define WAVEFORM_HANDLER_H

#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// declarations

#define WAVEFORM_TASK_STACK_SIZE 8192
#define WAVEFORM_TASK_PRIORITY   6

// public

typedef enum {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
} waveform_type_t;

/* Start waveform mode
 * This function will initialize the waveform mode.
 * It will create a task that will run in the background and generate the waveform.
 */
void waveform_mode_init();

/* Stop waveform mode
 * This function will de-initialize the waveform mode.
 * It will stop the task that is generating the waveform.
 */
void waveform_mode_deinit();

/* Get the current waveform type
 * This function will return the current waveform type.
 @returns a waveform_type_t enum.
 */
uint8_t get_waveform_type(); // Get the current waveform type

/* Set the current waveform type
 * This function will set the current waveform type.
 * @param waveform_type - the waveform type to set
 */
void set_waveform_type(uint8_t waveform_type); // Set the current waveform type

/* Check if waveform task is running
 * This function will return true if the task is running, false otherwise.
 * It is used to check if the task is running before starting it again.
 * @return bool - true if the task is running, false otherwise
 */
bool is_waveform_running();

// private

/* Waveform task
*/
static void waveform_task(void *pvParameters);

#endif // WAVEFORM_HANDLER_H