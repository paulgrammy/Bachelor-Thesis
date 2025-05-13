#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <Arduino.h>                                             // Include Arduino library for ESP32
#include <ESP_I2S.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <BluetoothA2DPSink.h>

#include "adc_handler.h"

// public

/* Bluetooth A2DP Sink
 * This function will initalize Bluetooth.
 * The function will start the A2DP sink with the board ID as the name.
 */
void bluetooth_mode_init();      // Starts Bluetooth + A2DP task

/* Bluetooth de-initialize
 * This function will disable Bluetooth, in turn disconnecting from any device. 
 */
void bluetooth_mode_deinit();    // Stops Bluetooth + A2DP task

/* Check if bluetooth task is running
* This function will return true if the task is running, false otherwise.
* It is used to check if the task is running before starting it again.
@return bool - true if the task is running, false otherwise
*/
bool is_bluetooth_running();

// private
static bool is_running = false;                             // Flag to mark if the task is running

#endif // BLUETOOTH_HANDLER_H
