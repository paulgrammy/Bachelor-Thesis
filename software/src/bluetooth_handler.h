#ifndef BLUETOOTH_HANDLER_H
#define BLUETOOTH_HANDLER_H

#include <Arduino.h>                                            
#include <ESP_I2S.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <BluetoothA2DPSink.h>

#include "adc_handler.h"

// public

/* Starts Bluetooth and A2DP sink
 * This function will initalize Bluetooth.
 * The function will start the A2DP sink with the board ID as the name.
 */
void bluetooth_mode_init();    

/* Stops Bluetooth and A2DP sink
 * This function will disable Bluetooth, in turn disconnecting from any device. 
 */
void bluetooth_mode_deinit();   

/* Check if bluetooth task is running
* This function will return true if the task is running, false otherwise.
* It is used to check if the task is running before starting it again.
@return bool - true if the task is running, false otherwise
*/
bool is_bluetooth_running();

// private

/* Flag to mark if the bluetooth task is running */
static bool is_running = false;                                 

#endif // BLUETOOTH_HANDLER_H
