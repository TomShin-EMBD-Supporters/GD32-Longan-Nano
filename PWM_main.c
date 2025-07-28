#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h"
#include <stdio.h>
void PWM_init(void);
void OLED_clear_line(uint8_t line);

uint8_t text[20];

int main(void) {
    SystemInit();
    SystemCoreClockUpdate();


    OLED_init();
    OLED_clear_screen();   

    PWM_init();
    
    uint32_t duty = 0;
    int direction = 1;

    while (1) {
      
       timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, duty);
        
      
        if (direction == 1) {
            duty += 100;
            if (duty >= 9999) {
                duty = 9999;
                direction = -1;
            }
        } else {
            if (duty <= 100) {
                duty = 0;
                direction = 1;
            } else {
                duty -= 100;
            }
        }
        
        float duty_percent = (duty * 100.0f) / 9999;


        sprintf((char*)text, "     CNT: %4d", duty);
        OLED_clear_line(2);    
        OLED_print_string(0, 2, text);

        sprintf((char*)text, "     PWM: %4.1f%%", duty_percent); 
        OLED_clear_line(4);    
        OLED_print_string(0, 4, text);

        
        delay_1ms(50);  
    }
}
void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                ");  
}

void PWM_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_TIMER0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);  // PA9 = TIMER0_CH1

    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocinitpara;

    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler         = 107;         // 108MHz / (107+1) = 1MHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 9999;        // 1MHz / 10000 = 100Hz
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER0, &timer_initpara);

    timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate     = TIMER_CCX_ENABLE;
    timer_ocinitpara.outputnstate    = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocpolarity      = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate     = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER0, TIMER_CH_1, &timer_ocinitpara);

    
    timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);

    
    timer_primary_output_config(TIMER0, ENABLE);

    timer_enable(TIMER0);
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, 0);
}
