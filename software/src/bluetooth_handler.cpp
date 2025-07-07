#include "bluetooth_handler.h"

// declarations
static const char *TAG_A2DP = "A2DP";                           // Tag for logging
const uint8_t I2S_SCK = 14;                                     // I2S BCLK
const uint8_t I2S_WS = 17;                                      // I2S LRCK 
const uint8_t I2S_SDOUT = 4;                                    // I2S data output

static TaskHandle_t a2dpTaskHandle = nullptr;
static BluetoothA2DPSink a2dp_sink;

void a2dp_task(void *pvParameters)
{
    ESP_LOGI(TAG_A2DP, "Starting Bluetooth A2DP sink");

    // Configure I2S
    i2s_pin_config_t my_pins = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_SDOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};

    a2dp_sink.set_pin_config(my_pins);
    a2dp_sink.set_stream_reader([](const uint8_t *data, uint32_t len){});
    a2dp_sink.start(get_board_id());

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(100));

        // If no audio is playing, clear the I2S buffer
        if (a2dp_sink.get_audio_state() == ESP_A2D_AUDIO_STATE_STOPPED)
        {
            ESP_LOGI(TAG_A2DP, "No audio playing, clearing I2S buffer");
            i2s_zero_dma_buffer(I2S_NUM_0);
        }
    }

    // Should not reach here, but clean up just in case
    a2dp_sink.end();
    vTaskDelete(nullptr);
}

// public

bool is_bluetooth_running()
{
    return is_running;
}

void bluetooth_mode_init()
{
    if (a2dpTaskHandle == nullptr)
    {
        xTaskCreatePinnedToCore(
            a2dp_task,
            "A2DP Task",
            8192,
            a2dpTaskHandle,
            5,
            &a2dpTaskHandle,
            0 
        );
    }
    else
    {
        ESP_LOGW(TAG_A2DP, "A2DP task already running");
    }

    is_running = true; 
}

void bluetooth_mode_deinit()
{
    if (a2dpTaskHandle != nullptr)
    {
        ESP_LOGI(TAG_A2DP, "Stopping A2DP task");
        a2dp_sink.end(); 
        
        vTaskDelete(a2dpTaskHandle);
        a2dpTaskHandle = nullptr;
    }
    else
    {
        ESP_LOGW(TAG_A2DP, "A2DP task not running");
    }

    is_running = false; 
}

// private
