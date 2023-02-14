#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


#include "microphone_connection.h"

#define microphone_GPIO_PORT 5

void microphone_sensor_setup(){
    esp_rom_gpio_pad_select_gpio(microphone_GPIO_PORT);
    gpio_set_direction(microphone_GPIO_PORT, GPIO_MODE_INPUT);
    // gpio_pulldown_en(microphone_GPIO_PORT);
    // gpio_pulldown_dis(microphone_GPIO_PORT);
    // gpio_pullup_dis(microphone_GPIO_PORT);
    // gpio_pullup_en(microphone_GPIO_PORT);

    // adc1_config_width(ADC_WIDTH_BIT_10);
    // adc1_config_channel_atten(ADC1_CHANNEL_3, 3);


}

int get_microphone_state(){
    return  gpio_get_level(microphone_GPIO_PORT);
    // return adc1_get_raw(microphone_GPIO_PORT);
}