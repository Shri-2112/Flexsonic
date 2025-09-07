// For only mpu mapped with audios


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_log.h"

#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define GYRO_XOUT_H  0x43

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_PORT I2C_NUM_0

#define UART_NUM UART_NUM_1
#define TXD_PIN 25
#define RXD_PIN 26
#define CMD_PLAY_TRACK 0x03

static const char *TAG = "MPU_DFPLAYER";

// Write to DFPlayer
void df_send(uint8_t cmd, uint16_t param) {
    uint8_t packet[10];
    packet[0] = 0x7E; packet[1] = 0xFF; packet[2] = 0x06; packet[3] = cmd;
    packet[4] = 0x00; // No feedback
    packet[5] = param >> 8; packet[6] = param & 0xFF;
    uint16_t sum = 0; for (int i = 1; i < 7; i++) sum += packet[i];
    sum = 0 - sum;
    packet[7] = sum >> 8; packet[8] = sum & 0xFF; packet[9] = 0xEF;
    uart_write_bytes(UART_NUM, (const char *)packet, 10);
}

// I2C write
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

// I2C read multiple bytes
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

void app_main(void) {
    // I2C init
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_PORT, &conf);
    i2c_driver_install(I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);

    // Wake MPU6050
    i2c_write(PWR_MGMT_1, 0);

    // UART init
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
    df_send(0x06, 25);

    while (1) {
        uint8_t data[6];
        i2c_read(GYRO_XOUT_H, data, 6);

        int16_t gyro_x = (data[0] << 8) | data[1];
        int16_t gyro_y = (data[2] << 8) | data[3];
        int16_t gyro_z = (data[4] << 8) | data[5];

        ESP_LOGI(TAG, "Gyro X:%d Y:%d Z:%d", gyro_x, gyro_y, gyro_z);

        if (gyro_x > 1000|| gyro_y > 15000 || gyro_z > 15000) {
            df_send(CMD_PLAY_TRACK, 1); // Play track 1
            vTaskDelay(pdMS_TO_TICKS(1000)); // Prevent spam
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


