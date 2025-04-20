#include <Arduino.h>

#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"
#include "esp_log.h"
#include "gpio_task.h"

static const char   *TAG_MAIN = "MAIN";
const               uint8_t I2S_SCK = 14;   /* Audio data bit clock */
const               uint8_t I2S_WS = 17;    /* Audio data L&R clock */
const               uint8_t I2S_SDOUT = 4;  /* ESP32 audio data output */

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
    // Configure I2S pins
    auto cfg = i2s.defaultConfig();
    cfg.pin_ws = I2S_WS;
    cfg.pin_bck = I2S_SCK;
    cfg.pin_data = I2S_SDOUT;

    // Check for I2S initialization
    if (i2s.begin(cfg) == false) {
        ESP_LOGE(TAG_MAIN, "Failed to initialize I2S!");
        return;
    }else {
        ESP_LOGI(TAG_MAIN, "I2S initialized successfully.");
    }

    // Setup Button GPIO task
    if(setup_gpio_task()){
        ESP_LOGI(TAG_MAIN, "GPIO task created successfully.");
    } else {
        ESP_LOGE(TAG_MAIN, "Failed to create GPIO task!");
    }
    a2dp_sink.start("MyMusic");

    ESP_LOGI(TAG_MAIN, "Setup succesfully completed.");
}

void loop() {}

