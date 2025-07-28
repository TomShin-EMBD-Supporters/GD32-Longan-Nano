#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h"
#include <stdio.h>
void PWM_init(void);
void GPIO_init(void);
void OLED_clear_line(uint8_t line);

uint8_t text[20];

int main(void) {
    SystemInit();
    SystemCoreClockUpdate();


    OLED_init();
    OLED_clear_screen();   

    PWM_init();
    GPIO_init();
    

       uint32_t duty = 1000;
    int direction = 1;
    float duty_percent = 0.0f;
    const uint32_t min_duty = 1000;
    const uint32_t max_duty = 2000;

    char* speed_state;

    while (1) {
        timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, duty);

        
        if (direction == 1) {
            duty += 1;
            if (duty >= max_duty) {
                duty = max_duty;
                direction = -1;
            }
        } else {
            if (duty <= min_duty) {
                duty = min_duty;
                direction = 1;
            } else {
                duty -= 1;
            }
        }

       
        duty_percent = ((float)(duty - min_duty) * 99.9f) / (float)(max_duty - min_duty);

        
        if (duty_percent < 0.0f) duty_percent = 0.0f;
        if (duty_percent > 99.9f) duty_percent = 99.9f;

        
        if (duty_percent <= 33.3f) {
            speed_state = "SLOW ";
        } else if (duty_percent <= 66.6f) {
            speed_state = "NORMAL";
        } else {
            speed_state = "FAST ";
        }

        
        sprintf((char*)text, "     CNTVAL: %4d", duty);
        OLED_clear_line(2);    
        OLED_print_string(0, 2, text);

        sprintf((char*)text, "     Speed: %4.1f%%", duty_percent); 
        OLED_clear_line(4);    
        OLED_print_string(0, 4, text);

        sprintf((char*)text, "     State: %s", speed_state);
        OLED_clear_line(6);    
        OLED_print_string(0, 6, text);
 
    }

}
void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                      ");  
}
void GPIO_init(void){
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_bit_set(GPIOB, GPIO_PIN_11);
    gpio_bit_reset(GPIOB, GPIO_PIN_10);

}
void PWM_init(void) {
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_TIMER0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);  // PA8 = TIMER0_CH0

    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocinitpara;

    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler         = 107;         // 108MHz / (107+1) = 1MHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1999;        // 1MHz / 2000 = 500Hz
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_init(TIMER0, &timer_initpara);

    timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate     = TIMER_CCX_ENABLE;
    timer_ocinitpara.outputnstate    = TIMER_CCXN_DISABLE;
    timer_ocinitpara.ocpolarity      = TIMER_OC_POLARITY_HIGH;
    timer_ocinitpara.ocidlestate     = TIMER_OC_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_ocinitpara);

    
    timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    
    timer_primary_output_config(TIMER0, ENABLE);

    timer_enable(TIMER0);
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 0);
}
