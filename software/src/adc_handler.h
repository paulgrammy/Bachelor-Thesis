#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

#include <Arduino.h>                                            
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"                                         
#include "esp_adc_cal.h"                                      

// public

/* Setup the ADC for Voltage sensing
 * This function will trigger the ADC to read the voltage from the GPIO pin.
 */
bool setup_adc(void);                                       

/* Get board ID
 * This function will return the board ID based on the voltage read from the ADC.
 * The board ID is determined by the voltage level.
 */
const char* get_board_id(void);                                   

// private

/* Read ADC value
 * This function will read the ADC value from the GPIO pin.
 * The ADC value is averaged over a number of samples to improve accuracy.
 * The function returns the average ADC value in millivolts (mV).
*/
static uint16_t read_adc(void);                                 

/*  Identify the board based on voltage
 *  This function will identify the board based on the voltage read from the ADC.
 *  The voltage is read in millivolts (mV) and compared to predefined thresholds.
 *  The function returns a string representing the board ID.
 */
const char* identify_board(uint16_t voltage_mV);              

/* Set board ID
 * This function will set the board ID based on the voltage read from the ADC.
 * The board ID is determined by the voltage level.
 */
void identify_and_set_board_id(void);

#endif // ADC_HANDLER_H