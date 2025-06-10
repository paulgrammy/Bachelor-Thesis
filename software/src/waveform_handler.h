#ifndef WAVEFORM_HANDLER_H
#define WAVEFORM_HANDLER_H

#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "gpio_task.h" // Include GPIO task header for button press handling

// declarations

#define WAVEFORM_TASK_STACK_SIZE 1000 // Larger stack size for the task
#define WAVEFORM_TASK_PRIORITY 10

// public

/* Waveform types enumeration
 * This enum defines the different waveform types that can be generated.
 * WAVEFORM_SINE - Sine wave
 * WAVEFORM_SQUARE - Square wave
 * WAVEFORM_TRIANGLE - Triangle wave
 * WAVEFORM_SAWTOOTH - Sawtooth wave
 */
typedef enum
{
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_INFECTED,
    WAVEFORM_SWEEP,
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
 @returns current waveform as waveform_type_t enum.
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

void build_waveform_data(uint8_t waveform_type); // Build waveform data

// private

#define SAMPLE_RATE 44100
#define FREQUENCY 200
#define AMPLITUDE 512
#define N (SAMPLE_RATE / FREQUENCY) // Number of samples per period
#define INFECTED_WAVEFORM_TABLE_SIZE 294

static const float sample_rate = SAMPLE_RATE;
static const float frequency = FREQUENCY;
static const float amplitude = AMPLITUDE;
static int waveform_length_bytes = 0;
static int16_t waveform_data[N * 2];
static float nr_samples;

/* This function generates and sends waveform data in chunks to avoid blocking the CPU.
 * It uses the I2S driver to send the data to the DAC.
 * The waveform type is determined by the waveform_task_parameters variable.
 * The function will run in a loop and generate the waveform data continuously.
 * Audio data is formated as interleaved left and right samples, MSB first, Philips timing.
 * Generate amplitude values (=sample points of the waveform) as 16-bit signed integers.
 * Shift left to form a 32-bit I2S word, MSB first.
 * Then interleaving left and right samples to form a stereo output for the PCM5102 DAC.
 * ESP32 takes care of clocks and timing using DMA.
 */
static void generate_wave(int start_index, int end_index);

/* Waveform task
 */
static void waveform_task(void *pvParameters);

#endif // WAVEFORM_HANDLER_H
