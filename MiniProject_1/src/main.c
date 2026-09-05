#include "esp_adc/adc_oneshot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc/adc_cali_scheme.h>
#include <math.h>
#include <pthread.h>
#include "driver/gpio.h"
#include "adc.h" 

#define ADC_CHANNEL ADC_CHANNEL_5
#define RELEY_PIN 15
#define ADC_BITWIDTH ADC_BITWIDTH_12
#define INPUT_VOLTAGE 3300

#define THRESHOLD_DARK 0.43
#define THRESHOLD_LIGHT 0.57

TaskHandle_t ADCTaskHandle = NULL;
const char* text = "Supermax78";

void ADCTask(void *arg)
{
   adc_task_data* task_data = (adc_task_data*)arg;
   int potentiometr_read, potentiometr_output;

   adc_oneshot_unit_handle_t adc_handle;
   adc_cali_handle_t cali_handle;

   ESP_ERROR_CHECK(ADC_Init(task_data->unit, task_data->channel, task_data->bitwidth, task_data->atten, &adc_handle, &cali_handle));

   float calc_voltage;
   float error;
   uint8_t state = 0;

   while(1)
   {
       potentiometr_read = ADC_ReadDigital(adc_handle, task_data->channel);
       printf("ADC_PIN (GPIO 1) ADC input from LDR: %d \n", potentiometr_read);

       calc_voltage = (float)potentiometr_read / ADC_GetMaxDigital(task_data->bitwidth) * INPUT_VOLTAGE;
       printf(" Calculated voltage in milivolts: %.0f \n", calc_voltage);

       potentiometr_output = ADC_GetCalibratedVoltage(cali_handle, potentiometr_read);
       printf(" Milivolt output after calibration - Channel %d %d \n", task_data->channel, potentiometr_output);

       if(potentiometr_output < (INPUT_VOLTAGE * THRESHOLD_DARK) && !state)
       {
            state = 1;
            printf(" turn on LED\n");
            gpio_set_level(task_data->control_pin, state);
       }
       else if (potentiometr_output > (INPUT_VOLTAGE * THRESHOLD_LIGHT) && state)
       {
            state = 0;
            printf(" turn off LED\n");
            gpio_set_level(task_data->control_pin, state);
       }

       error = fabsf(calc_voltage - potentiometr_output) / potentiometr_output * 100;
       printf(" Error %.2f%% \n", error);
       printf("\n * \n\n");
       
   
       vTaskDelay(pdMS_TO_TICKS(100));
   }

   ADC_Reset(adc_handle, &cali_handle);
}

void app_main() 
{
    gpio_reset_pin(RELEY_PIN);
    gpio_set_direction(RELEY_PIN, GPIO_MODE_OUTPUT);

    adc_task_data data = 
    {
        .unit = ADC_UNIT_1,
        .channel = ADC_CHANNEL_5,
        .control_pin = RELEY_PIN,
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };

    xTaskCreatePinnedToCore(ADCTask, "ADC Task", 4096, &data, 10, &ADCTaskHandle, 0);
    vTaskDelete(NULL);
}