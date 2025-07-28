#include "gd32vf103.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "systick.h"
#include "SSD1306.h"

#define INPUT_BUFFER_SIZE 10

void PWM_init(void);
void GPIO_init(void);
void UART0_setup(void);
void OLED_clear_line(uint8_t line);

char input_buffer[INPUT_BUFFER_SIZE];
uint8_t input_index = 0;
uint8_t text[32];  

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();

    UART0_setup();
    PWM_init();
    GPIO_init();
    
    OLED_init();
    OLED_clear_screen();   

    int new_speed = 0;
    int prev_speed = 0;
    uint8_t byte;
    char* speed_state;

    while (1) {
        if (usart_flag_get(USART0, USART_FLAG_RBNE)) {
            byte = usart_data_receive(USART0);

            if (byte == '\r' || byte == '\n') {
                input_buffer[input_index] = '\0';
                new_speed = atoi(input_buffer);

                if (new_speed > 2000) new_speed = 2000;
                if (new_speed < -2000) new_speed = -2000;

                 
                char msg[64];
                sprintf(msg, "Rotation speed : %d\r\n", new_speed);
                for (int i = 0; msg[i] != '\0'; i++) {
                    usart_data_transmit(USART0, msg[i]);
                    while (!usart_flag_get(USART0, USART_FLAG_TBE));
                }

                 
                if (new_speed == 0) {
                    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 0);
                    gpio_bit_reset(GPIOB, GPIO_PIN_10);
                    gpio_bit_reset(GPIOB, GPIO_PIN_11);
                }
                 
                else if ((new_speed > 0 && prev_speed < 0) || (new_speed < 0 && prev_speed > 0)) {
                    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 0);
                    gpio_bit_reset(GPIOB, GPIO_PIN_10);
                    gpio_bit_reset(GPIOB, GPIO_PIN_11);
                    delay_1ms(100);

                    if (new_speed > 0) {
                        gpio_bit_set(GPIOB, GPIO_PIN_11);
                        gpio_bit_reset(GPIOB, GPIO_PIN_10);
                    } else {
                        gpio_bit_set(GPIOB, GPIO_PIN_10);
                        gpio_bit_reset(GPIOB, GPIO_PIN_11);
                    }
                    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, abs(new_speed));
                }
                 
                else {
                    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, abs(new_speed));

                    if (new_speed > 0) {
                        gpio_bit_set(GPIOB, GPIO_PIN_11);
                        gpio_bit_reset(GPIOB, GPIO_PIN_10);
                    } else if (new_speed < 0) {
                        gpio_bit_set(GPIOB, GPIO_PIN_10);
                        gpio_bit_reset(GPIOB, GPIO_PIN_11);
                    }
                }

                 
                int duty = abs(new_speed);
                float duty_percent = ((float)(duty - 1000) * 99.9f) / (float)(2000 - 1000);
                if (duty_percent < 0.0f) duty_percent = 0.0f;
                if (duty_percent > 99.9f) duty_percent = 99.9f;

                if (duty_percent <= 33.3f) {
                    speed_state = "SLOW ";
                } else if (duty_percent <= 66.6f) {
                    speed_state = "NORMAL";
                } else {
                    speed_state = "FAST ";
                }

                 
                sprintf((char*)text, "INPUT: %5d", new_speed);
                OLED_clear_line(2);
                OLED_print_string(0, 2, text);

                 
                const char* direction;
                if (new_speed == 0) {
                    direction = "STOP";
                } else if (new_speed > 0) {
                    direction = "FORWARD";
                } else {
                    direction = "REVERSE";
                }
                sprintf((char*)text, "Direction: %s", direction);
                OLED_clear_line(4);
                OLED_print_string(0, 4, text);

                 
                sprintf((char*)text, "Speed: %4.1f%% %s", duty_percent, speed_state);
                OLED_clear_line(6);
                OLED_print_string(0, 6, text);

                 
                prev_speed = new_speed;
                input_index = 0;
            }
            else if (input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = byte;
            }
        }
    }
}

void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                      ");
}

void UART0_setup(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);   // TX
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // RX

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}

void GPIO_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10 | GPIO_PIN_11);

    gpio_bit_set(GPIOB, GPIO_PIN_11); // IN1
    gpio_bit_reset(GPIOB, GPIO_PIN_10); // IN2
}

void PWM_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_TIMER0);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);  // PA8 = TIMER0_CH0

    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocinitpara;

    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler         = 107;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 1999;
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
