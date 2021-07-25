#include "MPU9250.h"
#include <esp_log.h>
#include <esp_timer.h>

static const char TAG[] = "app_main";

#define I2C_MASTER_SCL_IO 22               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 21               /*!< gpio number for I2C master data  */
#define I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */

MPU9250 mpu;

void print_roll_pitch_yaw();

void setup() {
    i2c_config_t conf = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_MASTER_SDA_IO,         // select GPIO specific to your project
            .scl_io_num = I2C_MASTER_SCL_IO,         // select GPIO specific to your project
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master {
                    .clk_speed = I2C_MASTER_FREQ_HZ,  // select frequency specific to your project
            },
            .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (!mpu.setup(0x68)) {  // change to your own address
        ESP_LOGE(TAG, "MPU connection failed. Please check your connection with `connection_check` example.");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    mpu.calibrateAccelGyro();
    mpu.calibrateMag();
}

void loop() {
    if (mpu.update()) {
        static int64_t prev_us = esp_timer_get_time();
        if (esp_timer_get_time() > prev_us + 25000) {
            print_roll_pitch_yaw();
            prev_us = esp_timer_get_time();
        }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
}

void print_roll_pitch_yaw() {

    ESP_LOGI(TAG,
             "accel: [%+6.2f %+6.2f %+6.2f ] (G)\tgyro: [%+7.2f %+7.2f %+7.2f ] (º/s)\tmag: [%+7.2f %+7.2f %+7.2f ]\n",
             mpu.getLinearAccX(), mpu.getLinearAccY(), mpu.getLinearAccZ(),
             mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ(),
             mpu.getMagX(), mpu.getMagY(), mpu.getMagZ());
}

extern "C" _Noreturn void app_main() {
    setup();

    while (true) {
        loop();
        taskYIELD();
    }
}
