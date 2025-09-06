#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

// ------------------- CONFIG -------------------
#define SAMPLES 20 // Number of samples for averaging

// Flex Sensor ADC Channels (update to match your wiring)
#define FLEX1 ADC_CHANNEL_0  // GPIO36 (VP) - Thumb
#define FLEX2 ADC_CHANNEL_6  // GPIO34 - Index
#define FLEX3 ADC_CHANNEL_7  // GPIO35 - Middle
#define FLEX4 ADC_CHANNEL_4  // GPIO32 - Ring
#define FLEX5 ADC_CHANNEL_5  // GPIO33 - Pinky

// MPU6050
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define GYRO_XOUT_H  0x43
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_PORT I2C_NUM_0

// DFPlayer UART
#define UART_NUM UART_NUM_1
#define TXD_PIN GPIO_NUM_25   // ESP32 TX → DFPlayer RX (via voltage divider)
#define RXD_PIN GPIO_NUM_26   // ESP32 RX ← DFPlayer TX
#define CMD_PLAY_TRACK 0x03

static const char *TAG = "FLEX+GYRO_DFPLAYER";

adc_oneshot_unit_handle_t adc1_handle;

// ------------------- DFPLAYER FUNCTIONS -------------------
void dfplayer_send_command(uint8_t cmd, uint16_t param) {
    uint8_t packet[10];
    packet[0] = 0x7E;
    packet[1] = 0xFF;
    packet[2] = 0x06;
    packet[3] = cmd;
    packet[4] = 0x00; // no feedback
    packet[5] = (uint8_t)(param >> 8);
    packet[6] = (uint8_t)(param & 0xFF);
    uint16_t checksum = 0;
    for (int i = 1; i < 7; i++) checksum += packet[i];
    checksum = -checksum;
    packet[7] = (uint8_t)(checksum >> 8);
    packet[8] = (uint8_t)(checksum & 0xFF);
    packet[9] = 0xEF;

    uart_write_bytes(UART_NUM, (const char *)packet, sizeof(packet));
    vTaskDelay(pdMS_TO_TICKS(200));
}

void play_mp3_file(int file_number) {
    ESP_LOGI(TAG, "Playing file %04d.mp3", file_number);
    dfplayer_send_command(CMD_PLAY_TRACK, file_number);
}

// ------------------- FLEX SENSOR FUNCTIONS -------------------
int get_smoothed_adc_value(int channel) {
    int sum = 0;
    for (int i = 0; i < SAMPLES; i++) {
        int value;
        adc_oneshot_read(adc1_handle, channel, &value);
        sum += value;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return sum / SAMPLES;
}

bool is_in_trigger_range(int value) {
    return (value >= 1000 && value <= 3500);
}

// ------------------- MPU6050 FUNCTIONS -------------------
void i2c_write(uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

void i2c_read(uint8_t reg, uint8_t *buf, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (MPU6050_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
}

// ------------------- MAIN APP -------------------
void app_main(void) {
    int last_played = 0;

    // ADC Init
    adc_oneshot_unit_init_cfg_t init_config = { .unit_id = ADC_UNIT_1 };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = { .atten = ADC_ATTEN_DB_11, .bitwidth = ADC_BITWIDTH_12 };
    adc_oneshot_config_channel(adc1_handle, FLEX1, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX2, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX3, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX4, &config);
    adc_oneshot_config_channel(adc1_handle, FLEX5, &config);

    // I2C Init
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_PORT, &i2c_conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    // Wake MPU6050
    i2c_write(PWR_MGMT_1, 0);

    // UART Init
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, 1024, 0, 0, NULL, 0);

    // Set DFPlayer volume
    dfplayer_send_command(0x06, 25);
    ESP_LOGI(TAG, "System Ready. Monitoring sensors...");

    while (1) {
        // FLEX READINGS
        int thumb   = get_smoothed_adc_value(FLEX1);
        int index   = get_smoothed_adc_value(FLEX2);
        int middle  = get_smoothed_adc_value(FLEX3);
        int ring    = get_smoothed_adc_value(FLEX4);
        int pinky   = get_smoothed_adc_value(FLEX5);

        // GYRO READINGS
        uint8_t data[6];
        i2c_read(GYRO_XOUT_H, data, 6);
        int16_t gyro_x = (data[0] << 8) | data[1];
        int16_t gyro_y = (data[2] << 8) | data[3];
        int16_t gyro_z = (data[4] << 8) | data[5];
       
        ESP_LOGI(TAG,
         "Thumb:%d | Index:%d | Middle:%d | Ring:%d | Pinky:%d || Gyro X:%d Y:%d Z:%d",
         thumb, index, middle, ring, pinky, gyro_x, gyro_y, gyro_z);

        // FLEX TRIGGERS
        if (is_in_trigger_range(index) && last_played != 23) {
            play_mp3_file(23); last_played = 23;
        }
        else if (is_in_trigger_range(thumb) && last_played != 5) {
            play_mp3_file(5); last_played = 5;
        }
        else if (is_in_trigger_range(middle) && last_played != 7) {
            play_mp3_file(7); last_played = 7;
        }
        else if (is_in_trigger_range(ring) && last_played != 8) {
            play_mp3_file(8); last_played = 8;
        }
        else if (is_in_trigger_range(pinky) && last_played != 25) {
            play_mp3_file(25); last_played = 25;
        }
        // GYRO TRIGGERS
        else if ((gyro_x > 1000 || gyro_y > 15000 || gyro_z > 15000) && last_played != 6) {
            play_mp3_file(6); last_played = 6;
        }
        else if (!is_in_trigger_range(thumb) &&
                 !is_in_trigger_range(index) &&
                 !is_in_trigger_range(middle) &&
                 !is_in_trigger_range(ring) &&
                 !is_in_trigger_range(pinky) &&
                 (gyro_x < 600 && gyro_y < 15000 && gyro_z < 15000)) {
            last_played = 0; // reset when no triggers
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
































