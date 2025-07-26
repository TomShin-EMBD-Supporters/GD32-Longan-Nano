#include <stdio.h>
#include <systick.h>
#include <gd32vf103.h>
#include "SSD1306.h"


uint16_t measure_distance(void);
void TIMER0_init(void);
void GPIO_init(void);
void OLED_clear_line(uint8_t line);


int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    GPIO_init();
    TIMER0_init();

    OLED_init();
    OLED_clear_screen();
 
    uint16_t echo_time_us;
    uint16_t distance_cm;
    char buffer[32];

    while(1) {
        echo_time_us = measure_distance();

        if (echo_time_us == 0) {
            sprintf(buffer, "Distance: Out of range");
        } else {
             
            float distance_cm_f = (echo_time_us * 0.034f) / 2.0f;
            distance_cm = (uint16_t)(distance_cm_f + 0.5f);

            sprintf(buffer, "      Dist: %d CM", distance_cm);
        }

        OLED_clear_line(4);
        OLED_print_string(0, 4, (uint8_t*)buffer);

        OLED_print_string(0, 2, "   TomShin Project");
        OLED_print_string(0, 6, "   EMBD_Supporters");
        
        delay_1ms(100);
        

    }
}
void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                      ");
}
void TIMER0_init(void)
{
    rcu_periph_clock_enable(RCU_TIMER0);

    timer_parameter_struct timer_initpara;
    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = 107;  // 108 MHz / (107+1) = 1 MHz timer clock -> 1 us resolution
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = 0xFFFF; // 65535us (65ms)
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_init(TIMER0, &timer_initpara);

    timer_enable(TIMER0);
}
void GPIO_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);

    // TRIG (PB15)
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    // ECHO (PB14)
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);

   
}
static inline void timer_counter_clear(void)
{
    TIMER_CNT(TIMER0) = 0;
}
uint16_t measure_distance(void)
{
    uint16_t echo_start = 0, echo_end = 0;
    uint32_t timeout = 0;

     
    gpio_bit_reset(GPIOB, GPIO_PIN_15);
    delay_1us(2);

    gpio_bit_set(GPIOB, GPIO_PIN_15);
    delay_1us(10);
    gpio_bit_reset(GPIOB, GPIO_PIN_15);

     
    timer_counter_clear();

     
    timeout = 0;
    while (!gpio_input_bit_get(GPIOB, GPIO_PIN_14)) {
        delay_1us(1);
        if (++timeout > 30000) // 30ms 
            return 0; 
    }
    echo_start = timer_counter_read(TIMER0);

     
    timeout = 0;
    while (gpio_input_bit_get(GPIOB, GPIO_PIN_14)) {
        delay_1us(1);
        if (++timeout > 30000) // 30ms 
            return 0;  
    }
    echo_end = timer_counter_read(TIMER0);

    if (echo_end >= echo_start)
        return (echo_end - echo_start);
    else
        return 0; 
}
