#include "esp_adc/adc_oneshot.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_adc/adc_cali_scheme.h>
#include <math.h>
#include "driver/gpio.h"

#define ADC_PIN ADC_CHANNEL_5
#define RELEY_PIN 15
#define RELEY_PIN_IN 17

#define THRESHOLD_DARK 0.4
#define THRESHOLD_LIGHT 0.6
#define MAX_ADC_VALUE 4095


TaskHandle_t ADCTaskHandle = NULL;

void ADCTask(void *arg)
{
   int potentiometr_read, potentiometr_output;

   adc_oneshot_unit_handle_t handle = NULL;
   adc_oneshot_unit_init_cfg_t init_config1 = 
   {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE
   };

   ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &handle));

   adc_oneshot_chan_cfg_t config = {
      .bitwidth = ADC_BITWIDTH_DEFAULT,
      .atten = ADC_ATTEN_DB_12
   };

   ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, ADC_PIN, &config));

   adc_cali_handle_t cali_handle = NULL;

   adc_cali_curve_fitting_config_t cali_config = 
   {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12
   };

   ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));

   float calc_voltage;
   float error;

   while(1)
   {
       ESP_ERROR_CHECK(adc_oneshot_read(handle, ADC_PIN, &potentiometr_read));
       printf("ADC_PIN (GPIO 1) ADC input from LDR: %d \n", potentiometr_read);

       calc_voltage = (float)potentiometr_read / MAX_ADC_VALUE * 3.3 *1000;
       printf(" Calculated voltage: %.0f \n", calc_voltage);

       if(potentiometr_read < (MAX_ADC_VALUE * THRESHOLD_DARK))
       {
            printf(" turn on LED\n");
            gpio_set_level(RELEY_PIN, 1);
       }
       else if (potentiometr_read > (MAX_ADC_VALUE * THRESHOLD_LIGHT))
       {
            printf(" turn off LED\n");
            gpio_set_level(RELEY_PIN, 0);
       }

       adc_cali_raw_to_voltage(cali_handle, potentiometr_read, &potentiometr_output);
       printf(" Milivolt output after calibration - Channel 5 %d \n", potentiometr_output);

      error = fabsf(calc_voltage - potentiometr_output) / potentiometr_output * 100;
       printf(" Error %.2f%% \n", error);
       printf("\n * \n\n");

       vTaskDelay(100);
   }

   adc_oneshot_del_unit(handle);
   adc_cali_delete_scheme_curve_fitting(cali_handle);
   vTaskDelete(NULL);
}


void app_main() 
{
    gpio_reset_pin(RELEY_PIN);
    gpio_set_direction(RELEY_PIN, GPIO_MODE_OUTPUT);

    xTaskCreatePinnedToCore(ADCTask, "ADC Task", 4096, NULL, 10, &ADCTaskHandle, 0);
}