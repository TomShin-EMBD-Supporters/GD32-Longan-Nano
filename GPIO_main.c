#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h" 
#include <stdio.h>
#include <string.h>


#define LED1_PORT GPIOB
#define LED1_PIN  GPIO_PIN_10
#define LED2_PORT GPIOB
#define LED2_PIN  GPIO_PIN_11

#define SW1_PORT GPIOB
#define SW1_PIN  GPIO_PIN_14
#define SW2_PORT GPIOA
#define SW2_PIN  GPIO_PIN_8


char sw1_status[10] = "released";
char led1_status[5] = "OFF";
char sw2_status[10] = "released";
char led2_status[5] = "OFF";

char last_sw1_status[10] = "";
char last_led1_status[5] = "";
char last_sw2_status[10] = "";
char last_led2_status[5] = "";

void setup(void);
uint8_t read_debounced( uint32_t port, uint32_t pin);
void OLED_clear_line(uint8_t line);

int main(void) {
    setup();

    while (1) {
         
        uint8_t sw1 = read_debounced(SW1_PORT, SW1_PIN);
        uint8_t sw2 = read_debounced(SW2_PORT, SW2_PIN);

       
        if (sw1 == RESET) {
            gpio_bit_set(LED1_PORT, LED1_PIN);
            strcpy(sw1_status, "pressed");
            strcpy(led1_status, "ON");
        } else {
            gpio_bit_reset(LED1_PORT, LED1_PIN);
            strcpy(sw1_status, "released");
            strcpy(led1_status, "OFF");
        }

        if (sw2 == RESET) {
            gpio_bit_set(LED2_PORT, LED2_PIN);
            strcpy(sw2_status, "pressed");
            strcpy(led2_status, "ON");
        } else {
            gpio_bit_reset(LED2_PORT, LED2_PIN);
            strcpy(sw2_status, "released");
            strcpy(led2_status, "OFF");
        }

     
        if (strcmp(sw1_status, last_sw1_status) != 0) {
            OLED_clear_line(1);
            OLED_print_string(0, 1, "SW1:");
            OLED_print_string(40, 1, sw1_status);
            strcpy(last_sw1_status, sw1_status);
        }

        if (strcmp(led1_status, last_led1_status) != 0) {
            OLED_clear_line(2);
            OLED_print_string(0, 2, "LED1:");
            OLED_print_string(40, 2, led1_status);
            strcpy(last_led1_status, led1_status);
        }

        if (strcmp(sw2_status, last_sw2_status) != 0) {
            OLED_clear_line(4);
            OLED_print_string(0, 4, "SW2:");
            OLED_print_string(40, 4, sw2_status);
            strcpy(last_sw2_status, sw2_status);
        }

        if (strcmp(led2_status, last_led2_status) != 0) {
            OLED_clear_line(5);
            OLED_print_string(0, 5, "LED2:");
            OLED_print_string(40, 5, led2_status);
            strcpy(last_led2_status, led2_status);
        }

        delay_1ms(50);   
    }
}
void OLED_clear_line(uint8_t line) {
    OLED_print_string(0, line, "                ");  
}

void setup(void) {
    SystemInit();
    SystemCoreClockUpdate();
    

    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    
    gpio_init(LED1_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LED1_PIN);
    gpio_init(LED2_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, LED2_PIN);

     
    gpio_init(SW1_PORT, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, SW1_PIN);
    gpio_init(SW2_PORT, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, SW2_PIN);

    OLED_init();
    OLED_clear_buffer();
    OLED_fill(0x00);
    OLED_print_string(10, 0, "SW + LED Status");
}

uint8_t read_debounced(uint32_t port, uint32_t pin) {
    uint8_t first = gpio_input_bit_get(port, pin);
    delay_1ms(20);
    uint8_t second = gpio_input_bit_get(port, pin);
    return (first == second) ? first : 1;  // 1 = not pressed
}
