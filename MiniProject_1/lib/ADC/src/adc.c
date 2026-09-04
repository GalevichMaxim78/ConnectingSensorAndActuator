#include "adc.h"
#include <esp_adc/adc_oneshot.h>

esp_err_t ADC_Init(adc_unit_t unit_id, adc_channel_t channel, adc_bitwidth_t bitwidth, adc_atten_t atten, adc_oneshot_unit_handle_t* handle,
              adc_cali_handle_t* cali_handle)
{
    
    adc_oneshot_unit_init_cfg_t init_config = 
    {
      .unit_id = unit_id,
      .ulp_mode = ADC_ULP_MODE_DISABLE
    };


    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, handle));

    adc_oneshot_chan_cfg_t config = {
      .bitwidth = bitwidth,
      .atten = atten
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(*handle, channel, &config));

    adc_cali_curve_fitting_config_t cali_config = 
    {
      .unit_id = unit_id,
      .atten = atten,
      .bitwidth = bitwidth
    };

    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, cali_handle));

    return ESP_OK;
}

int ADC_ReadDigital(adc_oneshot_unit_handle_t handle, adc_channel_t channel)
{
    int value;
    ESP_ERROR_CHECK(adc_oneshot_read(handle, channel, &value));

    return value;
}

int ADC_GetMaxDigital(adc_bitwidth_t bitwidth)
{
    switch(bitwidth)
    {
      case ADC_BITWIDTH_DEFAULT:
      case ADC_BITWIDTH_12: return (1 << 12) - 1;
      case ADC_BITWIDTH_11: return (1 << 11) - 1;
      case ADC_BITWIDTH_10: return (1 << 10) - 1;
      case ADC_BITWIDTH_9:  return (1 << 9) - 1;
      case ADC_BITWIDTH_13: return (1 << 13) - 1;
      default: return 0;
    }
}

int ADC_GetCalibratedVoltage(adc_cali_handle_t cali_handle, int value)
{
    int voltage;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, value, &voltage));

    return voltage;
}

void ADC_Reset(adc_oneshot_unit_handle_t handle, adc_cali_handle_t* cali_handle)
{
    adc_oneshot_del_unit(handle);

    if (cali_handle)
    {
      adc_cali_delete_scheme_curve_fitting(*cali_handle);
    }
}
