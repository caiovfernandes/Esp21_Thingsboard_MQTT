//standard includes
#include <stdio.h>
#include <string.h>

//esp includes
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"

//freertos includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//driver includes
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

//local includes
#include "wifi.h"
#include "http_client.h"
#include "mqtt.h"
#include "dht.h"
#include "microphone_connection.h"
#include "led.h"
#include "driver/ledc.h"

#define LED_1 2



SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

void conectadoWifi(void * params)
{
    while(true) {
        if(xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY)) {
            // Processamento Internet
            mqtt_start();
        }
    }
}

void monitorLocalmicrophone(void * params)
{
    while(true) {
        int microphone = get_microphone_state();
        printf("microphone: %d\n", microphone);
        ESP_LOGI("DEBUG", "Microphone: %d", microphone);
        led_state(microphone);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sendMQTTData(void * params)
{
    char telemetry[200];
    char microphoneString[50];

    if(xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY)) {
        while(true) {
            int microphone = get_microphone_state();
            float dhtTemperature = DHT11_read().temperature;
            float dhtHumidity = DHT11_read().humidity;

            // assemble json
            sprintf(
                telemetry,
                "{\"temperature\": %f, \n\"humidity\": %f}",
                dhtTemperature,
                dhtHumidity
            );
            // send json
            mqtt_envia_mensagem("v1/devices/me/telemetry", telemetry);

            sprintf(
                microphoneString,
                "{\"microphone\": %d}",
                microphone
            );
            // send json string to mqtt
            mqtt_envia_mensagem("v1/devices/me/attributes", microphoneString);

            // delay 1s
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void)
{
    // Initialize DHT.
    DHT11_init(GPIO_NUM_4);

    // Initialize microphone sensor.
    microphone_sensor_setup();
    printf("microphone sensor\n");
    // Initialize LED.
    led_setup();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&conectadoWifi,  "MQTT connection", 4096, NULL, 1, NULL);
    xTaskCreate(&sendMQTTData, "Broker connection", 4096, NULL, 1, NULL);
    xTaskCreate(&monitorLocalmicrophone, "Local microphone connection", 4096, NULL, 1, NULL);

    // works only after unplugging and plugging the USB cable

}
