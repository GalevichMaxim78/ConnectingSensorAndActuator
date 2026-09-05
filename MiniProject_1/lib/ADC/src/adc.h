#include <esp_adc/adc_oneshot.h>

typedef struct 
{
    adc_unit_t unit;
    adc_channel_t channel;
    uint8_t control_pin;
    adc_bitwidth_t bitwidth;
    adc_atten_t atten;
}adc_task_data;

esp_err_t ADC_Init(adc_unit_t unit_id, adc_channel_t channel, adc_bitwidth_t bitwidth, adc_atten_t atten, adc_oneshot_unit_handle_t* handle,
              adc_cali_handle_t* cali_handle);

int ADC_ReadDigital(adc_oneshot_unit_handle_t handle, adc_channel_t channel);

int ADC_GetMaxDigital(adc_bitwidth_t bitwidth);

int ADC_GetCalibratedVoltage(adc_cali_handle_t cali_handle, int value);

void ADC_Reset(adc_oneshot_unit_handle_t handle, adc_cali_handle_t* cali_handle);