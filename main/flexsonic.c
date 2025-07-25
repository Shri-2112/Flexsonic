//including all the necessary libraries

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_system.h"
#include "esp_adc/adc_oneshot.h"

#define UART_PORT_NUM UART_NUM_2

#define UART_TX_PIN 17
#define UART_RX_PIN 16

#define FLEX_THUMB ADC_CHANNEL_4 // GPIO32
#define FLEX_INDEX ADC_CHANNEL_5 // GPIO33
#define FLEX_MIDDLE ADC_CHANNEL_6 // GPIO34
#define FLEX_RING ADC_CHANNEL_7 // GPIO35
#define FLEX_PINKY ADC_CHANNEL_0 // GPIO36


#define TAG "FLEXSONIC"

#define THUMB_THRESHOLD ... //values to be added
#define INDEX_THRESHOLD ...
#define MIDDLE_THRESHOLD ...
#define RING_THRESHOLD ...
#define PINKY_THRESHOLD ...

// Send command to DFPlayer
void sendDFPlayerCommand(uint8_t cmd, uint8_t param) {
    uint8_t packet[10] = {
        0x7E, 0xFF, 0x06, cmd, 0x00, 0x00, 0x00, 0x00, 0xEF
    };
    packet[5] = 0x00;
    packet[6] = param;
    uint16_t checksum = 0 - (packet[1] + packet[2] + packet[3] +
                             packet[4] + packet[5] + packet[6]);
    packet[7] = (checksum >> 8) & 0xFF;
    packet[8] = checksum & 0xFF;
    uart_write_bytes(UART_PORT_NUM, (const char *)packet, 10);
}

void app_main(void) {
    // ADC config
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t adc_unit_cfg = {
        .unit_id = ADC_UNIT_1
    };
    adc_oneshot_new_unit(&adc_unit_cfg, &adc1_handle);
    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11
    };

    // UART config
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(UART_PORT_NUM, 256, 0, 0, NULL, 0);
    uart_param_config(UART_PORT_NUM, &uart_config);
    uart_set_pin(UART_PORT_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "Waiting for DFPlayer to boot...");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2s for DFPlayer to initialize

    // Set volume to max (48)
    sendDFPlayerCommand(0x06, 0x30); // Set volume to 48
    vTaskDelay(pdMS_TO_TICKS(500));

    adc_oneshot_config_channel(adc1_handle, FLEX_THUMB, &chan_cfg);
    adc_oneshot_config_channel(adc1_handle, FLEX_INDEX, &chan_cfg);
    adc_oneshot_config_channel(adc1_handle, FLEX_MIDDLE, &chan_cfg);
    adc_oneshot_config_channel(adc1_handle, FLEX_RING, &chan_cfg);
    adc_oneshot_config_channel(adc1_handle, FLEX_PINKY, &chan_cfg);

    bool playing[5] = {false, false, false, false, false};

    while (1) {
        int values[5];

        adc_oneshot_read(adc1_handle, FLEX_THUMB, &values[0]);
        adc_oneshot_read(adc1_handle, FLEX_INDEX, &values[1]);
        adc_oneshot_read(adc1_handle, FLEX_MIDDLE, &values[2]);
        adc_oneshot_read(adc1_handle, FLEX_RING, &values[3]);
        adc_oneshot_read(adc1_handle, FLEX_PINKY, &values[4]);

        ESP_LOGI(TAG, "Thumb: %d | Index: %d | Middle: %d | Ring: %d | Pinky: %d",
                 values[0], values[1], values[2], values[3], values[4]);

        // Thumb
        if (values[0] > THUMB_THRESHOLD && !playing[0]) {
            sendDFPlayerCommand(0x0F, 0x01); // 0001.mp3
            playing[0] = true;
        } 
        else if (values[0] <= THUMB_THRESHOLD) playing[0] = false;

        // Index
        if (values[1] > RING_THRESHOLD && !playing[1]) {
            sendDFPlayerCommand(0x0F, 0x02); // 0002.mp3
            playing[1] = true;
        } 
        else if (values[1] <= RING_THRESHOLD) playing[1] = false;

        // Middle
        if (values[2] > MIDDLE_THRESHOLD && !playing[2]) {
            sendDFPlayerCommand(0x0F, 0x03); // 0003.mp3
            playing[2] = true;
        } 
        else if (values[2] <= MIDDLE_THRESHOLD) playing[2] = false;

        // Ring
        if (values[3] > INDEX_THRESHOLD && !playing[3]) {
            sendDFPlayerCommand(0x0F, 0x04); // 0004.mp3
            playing[3] = true;
        } 
        else if (values[3] <= INDEX_THRESHOLD) playing[3] = false;

        // Pinky
        if (values[4] > PINKY_THRESHOLD && !playing[4]) {
            sendDFPlayerCommand(0x0F, 0x05); // 0005.mp3
            playing[4] = true;
        } 
        else if (values[4] <= PINKY_THRESHOLD) playing[4] = false;

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
