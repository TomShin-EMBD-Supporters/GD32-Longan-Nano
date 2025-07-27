#include <stdio.h>
#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h"

void ADC_init(void);
uint16_t ADC_read(void);
void OLED_clear_line(uint8_t line);

uint8_t text[20];

int main(void) {
    SystemInit();
    SystemCoreClockUpdate();

    OLED_init();
    OLED_clear_screen();   
    ADC_init();

    uint16_t adc_value = 0;
    float percent = 0.0f;

    while(1) {
        adc_value = ADC_read();

         
        if(adc_value >= 4090) {
            adc_value = 4095;
        } else if(adc_value <= 5) {
            adc_value = 0;
        }

        
        percent = (adc_value * 100.0f) / 4095.0f;

       
        sprintf((char*)text, "ADC: %4d", adc_value);
        OLED_clear_line(2);    
        OLED_print_string(0, 2, text);

        sprintf((char*)text, "PERCENT: %5.1f %%", percent); 
        OLED_clear_line(4);    
        OLED_print_string(0, 4, text);

        delay_1ms(200);
    }
}

void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                ");  
}

void ADC_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_ADC1);   
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);

    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_1);  

    adc_deinit(ADC1);
    adc_mode_config(ADC_MODE_FREE);
    adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
    adc_special_function_config(ADC1, ADC_CONTINUOUS_MODE, ENABLE);  

    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 1);
    adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);  

    adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_NONE);
    adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);

    adc_enable(ADC1);
    delay_1ms(1);
    adc_calibration_enable(ADC1);

    adc_software_trigger_enable(ADC1, ADC_REGULAR_CHANNEL);  
}

uint16_t ADC_read(void) {
    while(!adc_flag_get(ADC1, ADC_FLAG_EOC));   
    uint16_t value = adc_regular_data_read(ADC1);
    adc_flag_clear(ADC1, ADC_FLAG_EOC);
    return value;
}
