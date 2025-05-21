// #ifndef SWITCH_HANDLER_H
// #define SWITCH_HANDLER_H

// #include <Arduino.h>
// #include <esp_log.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
// #include <driver/gpio.h>

// // declarations
// #define GPIO_STACK_SIZE 4096 // Stack size for the task
// #define TASK_PRIORITY_SWITCH 7    // Task priority
// #define BYPASS_PIN 1 // GPIO pin for the button
// #define HI_LO_PIN 26 // GPIO pin for the high/low signal output pin

// // public

// bool setup_bypass_task(void);

// // private 

// void bypass_event_loop(void *pvParameters);

// #endif // SWITCH_HANDLER_H