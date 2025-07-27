#include "gd32vf103.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INPUT_BUFFER_SIZE 10

void UART0_setup(void);
void delay_ms(uint32_t ms);

char input_buffer[INPUT_BUFFER_SIZE];
uint8_t input_index = 0;

int main(void)
{
    UART0_setup();

    uint8_t byte;
    int new_speed = 0;

    while (1) {
        if (usart_flag_get(USART0, USART_FLAG_RBNE)) {
            byte = usart_data_receive(USART0);

            if (byte == '\r' || byte == '\n') {
                input_buffer[input_index] = '\0'; // null-terminate

                new_speed = atoi(input_buffer);

                if (new_speed < 1000) new_speed = 1000;
                if (new_speed > 2000) new_speed = 2000;

                char msg[64];
                sprintf(msg, "Rotation speed : %d\r\n", new_speed);

                for (int i = 0; msg[i] != '\0'; i++) {
                    usart_data_transmit(USART0, msg[i]);
                    while (!usart_flag_get(USART0, USART_FLAG_TBE));
                }

                input_index = 0; // reset input buffer index
            } else if (input_index < INPUT_BUFFER_SIZE - 1) {
                input_buffer[input_index++] = byte;
            }
        }
    }
}

void UART0_setup(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART0);

    // PA9 = TX, PA10 = RX
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);   // TX
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10); // RX

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_enable(USART0);
}
