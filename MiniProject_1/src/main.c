#include "esp_adc/adc_oneshot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc/adc_cali_scheme.h>
#include <math.h>
#include "driver/gpio.h"
#include "adc.h" 

#define ADC_CHANNEL ADC_CHANNEL_5
#define RELEY_PIN 15
#define ADC_BITWIDTH ADC_BITWIDTH_12

#define THRESHOLD_DARK 0.4
#define THRESHOLD_LIGHT 0.6

TaskHandle_t ADCTaskHandle = NULL;

void ADCTask(void *arg)
{
   int potentiometr_read, potentiometr_output;

   adc_oneshot_unit_handle_t adc_handle;
   adc_cali_handle_t cali_handle;

   ESP_ERROR_CHECK(ADC_Init(ADC_UNIT_1, ADC_CHANNEL_5, ADC_BITWIDTH, ADC_ATTEN_DB_12, &adc_handle, &cali_handle));

   float calc_voltage;
   float error;

   while(1)
   {
       potentiometr_read = ADC_ReadDigital(adc_handle, ADC_CHANNEL);
       printf("ADC_PIN (GPIO 1) ADC input from LDR: %d \n", potentiometr_read);

       calc_voltage = (float)potentiometr_read / ADC_GetMaxDigital(ADC_BITWIDTH) * 3.3 *1000;
       printf(" Calculated voltage: %.0f \n", calc_voltage);

       if(potentiometr_read < (ADC_GetMaxDigital(ADC_BITWIDTH) * THRESHOLD_DARK))
       {
            printf(" turn on LED\n");
            gpio_set_level(RELEY_PIN, 1);
       }
       else if (potentiometr_read > (ADC_GetMaxDigital(ADC_BITWIDTH) * THRESHOLD_LIGHT))
       {
            printf(" turn off LED\n");
            gpio_set_level(RELEY_PIN, 0);
       }

       potentiometr_output = ADC_DigitalToVoltage(cali_handle, potentiometr_read);
       printf(" Milivolt output after calibration - Channel %d %d \n", ADC_CHANNEL, potentiometr_output);

       error = fabsf(calc_voltage - potentiometr_output) / potentiometr_output * 100;
       printf(" Error %.2f%% \n", error);
       printf("\n * \n\n");

       vTaskDelay(100);
   }

   ADC_Reset(adc_handle, &cali_handle);
   vTaskDelete(NULL);
}


void app_main() 
{
    gpio_reset_pin(RELEY_PIN);
    gpio_set_direction(RELEY_PIN, GPIO_MODE_OUTPUT);

    xTaskCreatePinnedToCore(ADCTask, "ADC Task", 4096, NULL, 10, &ADCTaskHandle, 0);
}