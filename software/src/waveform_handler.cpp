#include "waveform_handler.h"

// declarations

static const char *TAG_WAVEFORM = "WAVEFORM";  // Tag for logging
static uint8_t waveform_task_parameters;       // Task parameters, unused but still defined
static TaskHandle_t waveformTaskHandle = NULL; // Task handle for the waveform task
static const uint8_t I2S_SCK = 14;             // Audio data bit clock
static const uint8_t I2S_WS = 17;              // Audio data L&R clock
static const uint8_t I2S_SDOUT = 4;            // ESP32 audio data output

// definitions

// public
bool is_waveform_running()
{
    return is_running; // Return the running flag
}

void waveform_mode_init()
{
    if (waveformTaskHandle == NULL)
    {
        // Check if the task is already running
        xTaskCreatePinnedToCore(
            waveform_task,
            "Waveform Task",
            WAVEFORM_TASK_STACK_SIZE,
            &waveform_task_parameters,
            WAVEFORM_TASK_PRIORITY,
            &waveformTaskHandle,
            0);
    }
    else
    {
        ESP_LOGW(TAG_WAVEFORM, "Waveform task already running");
    }

<<<<<<< HEAD
    vTaskDelay(10);

    // Kickstart generation
=======
    //Kickstart generation
>>>>>>> 396b85de37869f8365576f73af94f6a35c727b2a
    set_waveform_type(waveform_task_parameters);

    is_running = true;
}

void waveform_mode_deinit()
{
    if (waveformTaskHandle != NULL)
    {
        vTaskDelete(waveformTaskHandle); // Delete the task
        waveformTaskHandle = NULL;       // Reset the task handle
    }

    is_running = false;

    // De-initialize I2S
    esp_err_t result = i2s_driver_uninstall(I2S_NUM_0); // Uninstall the I2S driver
    if (result == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG_WAVEFORM, "Failed to uninstall I2S driver: %s", esp_err_to_name(result));
    }
    else
    {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver uninstalled successfully");
    }

    waveform_task_parameters = 0; // Reset the waveform type
}

void waveform_task(void *pvParameters)
{
    ESP_LOGI(TAG_WAVEFORM, "Starting waveform task");

    // Optionally configure I2S pins
    i2s_pin_config_t my_pins = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};

    // This is taken from A2DP library
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = (i2s_bits_per_sample_t)16,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0, // default interrupt priority
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
        .tx_desc_auto_clear =
            true, // avoiding noise in case of data unavailability
        .fixed_mclk = 0,
        .mclk_multiple = (i2s_mclk_multiple_t)0, // I2S_MCLK_MULTIPLE_DEFAULT
        .bits_per_chan = I2S_BITS_PER_CHAN_DEFAULT
#else
        .tx_desc_auto_clear =
            true // avoiding noise in case of data unavailability
#endif
    };

    esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (result == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW(TAG_WAVEFORM, "I2S already installed");
    }
    else if (result == ESP_OK)
    {
        ESP_LOGI(TAG_WAVEFORM, "I2S driver installed successfully");
    }
    else
    {
        ESP_LOGE(TAG_WAVEFORM, "Failed to install I2S driver: %s", esp_err_to_name(result));
        return;
    }

    // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &my_pins);

    while (true)
    {
        size_t bytes_written;
        esp_err_t err = i2s_write(I2S_NUM_0, waveform_data, waveform_length_bytes, &bytes_written, portMAX_DELAY);
<<<<<<< HEAD
        i2s_zero_dma_buffer(I2S_NUM_0); // Clear the DMA buffer after writing
=======
>>>>>>> 396b85de37869f8365576f73af94f6a35c727b2a
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG_WAVEFORM, "I2S write failed: %s", esp_err_to_name(err));
        }
    }
}

uint8_t get_waveform_type()
{
    return waveform_task_parameters; // Return the current waveform type
}

void set_waveform_type(uint8_t waveform_type)
{
    waveform_task_parameters = waveform_type; // Set the current waveform type

    // Generate 1 full period of waveform data
    generate_wave(0, N);

    ESP_LOGI(TAG_WAVEFORM, "Waveform buffer cleaned %d, %d", waveform_data[0], waveform_data[1]); // Log the change
}

// private

static void generate_wave(int start_index, int end_index)
{
    // Use N as the number of samples per period when generating all waveforms except for infected waveform
    if (waveform_task_parameters == WAVEFORM_INFECTED)
    {
        // For infected waveform, use a fixed size
        nr_samples = INFECTED_WAVEFORM_TABLE_SIZE;
    }
    else
    {
        nr_samples = N; // Use N for other waveforms
    }
    
    for (int i = 0; i < nr_samples; ++i)
    {
        int16_t sample = 0; // Initialize sample to zero

        // t is N/fs where N is sample index (in this case, i) and fs is sample rate
        float t = float(i) / sample_rate;

        // phase angle required for sine and square; 2*pi*f*t
        float phase = 2.0f * M_PI * frequency * t;

        /* sinf is used because it is faster than sin
         * - if sinf is used, there is no need for type conversion
         * - it will also make code faster since it uses single-precision floating point, sin uses double-precision
         * - there won't be any loss of precision since we are using 16-bit signed int
         */
        switch (waveform_task_parameters)
        {
        case WAVEFORM_SINE:
            /* Sine wave = A*sin(2*pi*f*t) = A*sin(phase)
             * sin(phase) is in the range [-1, 1], so this will generate a continuous value in the range [-A, A]
             */
            sample = (int16_t)(amplitude * sinf(phase));
            break;
        case WAVEFORM_SQUARE:
            /* Square wave = A*sign(sin(2*pi*f*t)) = A*sign(phase)
             * sinf(phase) is in the range [-1, 1], so the if statement will fix sample to either -A or A
             */
            if (sinf(phase) > 0)
            {
                sample = (int16_t)(amplitude); // Positive half of square wave
            }
            else
            {
                sample = (int16_t)(-amplitude); // Negative half of square wave
            }
            break;
        case WAVEFORM_TRIANGLE:
            /* Triangle wave = A*(2/pi)*asin(sin(2*pi*f*t)) = A*(2/pi)*asin(phase)
             * sinf(phase) is in the range [-1, 1], so this will generate a continuous value in the range [-A, A]
             * asin(sin(phase)) is in the range [-pi/2, pi/2], so this will now map the values to [-pi/2, pi/2]
             * (2/pi) is used to scale the value to the range [-A, A]
             */
            sample = (int16_t)(amplitude * (2.0f / M_PI) * asinf(sinf(phase)));
            break;
            // Infected waveform is a 2kHz square with a 60Hz sine wave on top of it
        case WAVEFORM_INFECTED:
        {
            float good_signal = 1500.0f;
            float infected_signal = 150.0f;

            // Infected waveform = sin(1.5kHz) + sin(150Hz)
            float infected_phase = 2.0f * M_PI * infected_signal * t; // 150Hz sine wave

            // 1.5kHz sine wave
            float sine_phase = 2.0f * M_PI * good_signal * t; // 1.5kHz sine wave

            // Combine the two signals
            float combined = 0.6f * sinf(infected_phase) + 0.4f * sinf(sine_phase);

            sample = (int16_t)(amplitude * combined);
            break;
        }
        // case WAVEFORM_SWEEP:
        // {
        //     /* Sweep wave = A*sin(2*pi*f*t) = A*sin(phase)
        //      * This will generate a continuous value in the range [-A, A]
        //      * The frequency will sweep from 100Hz to 5kHz over the duration of the waveform and loop back to 100Hz
        //      */
        //     float sweep_frequency = 100.0f + (4900.0f * (float)i / (float)N); // Sweep from 100Hz to 5kHz
        //     float sweep_phase = 2.0f * M_PI * sweep_frequency * t; // Calculate the phase for the sweep frequency

        //     //Loop back to 100Hz after reaching 5kHz
        //     if (sweep_frequency > 5000.0f)
        //     {
        //         sweep_frequency = 100.0f; // Reset frequency to 100Hz
        //         sweep_phase = 2.0f * M_PI * sweep_frequency * t; // Recalculate phase for the reset frequency
        //     }

        //     sample = (int16_t)(amplitude * sinf(sweep_phase)); // Generate the sample for the sweep frequency
        //     break;
        }
        default:
            break;
        }

        // Write data to the waveform data array
        waveform_data[2 * i] = sample;     // Left channel
        waveform_data[2 * i + 1] = sample; // Right channel

        // Logging will slow down the process, so it is commented out
        // // print package
        // ESP_LOGI(TAG_WAVEFORM, "Sample %d: %d", i, sample);
        // }
    }
    
    waveform_length_bytes = sizeof(waveform_data);
}
