#include "gd32vf103.h"
#include "i2c.h"
#include "systick.h"
#include <string.h>
#include <stdio.h>

#define LCD_ADDRESS       0x27  
#define LCD_BACKLIGHT     0x08
#define LCD_ENABLE        0x04
#define LCD_COMMAND       0
#define LCD_DATA          1

void lcd_send(uint8_t data, uint8_t mode);
void lcd_write_4bits(uint8_t value);
void lcd_pulse_enable(uint8_t data);
void lcd_command(uint8_t cmd);
void lcd_write(uint8_t chr);
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char *str);
void lcd_print_int(int num);
void lcd_print_float(float num, uint8_t precision);

void TIMER0_init(void);
void GPIO_init(void);
uint16_t measure_distance(void);

void lcd_expander_write(uint8_t data)
{
    i2c_write(LCD_ADDRESS << 1, 0x00, &data, 1);  
}

void lcd_write_4bits(uint8_t value)
{
    lcd_expander_write(value | LCD_BACKLIGHT);
    lcd_pulse_enable(value);
}

void lcd_pulse_enable(uint8_t data)
{
    lcd_expander_write(data | LCD_ENABLE | LCD_BACKLIGHT);
    delay_1us(1);
    lcd_expander_write((data & ~LCD_ENABLE) | LCD_BACKLIGHT);
    delay_1us(50);
}

void lcd_send(uint8_t value, uint8_t mode)
{
    uint8_t high = value & 0xF0;
    uint8_t low = (value << 4) & 0xF0;

    lcd_write_4bits(high | mode);
    lcd_write_4bits(low | mode);
}

void lcd_command(uint8_t cmd)
{
    lcd_send(cmd, LCD_COMMAND);
}

void lcd_write(uint8_t chr)
{
    lcd_send(chr, LCD_DATA);
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    static uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    lcd_command(0x80 | (col + row_offsets[row]));
}

void lcd_clear(void)
{
    lcd_command(0x01);
    delay_1ms(2);
}

void lcd_print(const char *str)
{
    while (*str)
        lcd_write(*str++);
}

void lcd_print_int(int num)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", num);
    lcd_print(buffer);
}

void lcd_print_float(float num, uint8_t precision)
{
    char buffer[32];
    char format[10];
    snprintf(format, sizeof(format), "%%.%df", precision);
    snprintf(buffer, sizeof(buffer), format, num);
    lcd_print(buffer);
}

void TIMER0_init(void)
{
    rcu_periph_clock_enable(RCU_TIMER0);

    timer_parameter_struct timer_initpara;
    timer_deinit(TIMER0);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = 107;  // 108MHz / (107+1) = 1MHz -> 1us resolution
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = 0xFFFF;
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_init(TIMER0, &timer_initpara);

    timer_enable(TIMER0);
}

void GPIO_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);

    // TRIG: PB15
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
    // ECHO: PB14
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

    // Trigger pulse
    gpio_bit_reset(GPIOB, GPIO_PIN_15);
    delay_1us(2);
    gpio_bit_set(GPIOB, GPIO_PIN_15);
    delay_1us(10);
    gpio_bit_reset(GPIOB, GPIO_PIN_15);

    // Wait for echo HIGH
    timer_counter_clear();
    timeout = 0;
    while (!gpio_input_bit_get(GPIOB, GPIO_PIN_14)) {
        delay_1us(1);
        if (++timeout > 30000)
            return 0;
    }
    echo_start = TIMER_CNT(TIMER0);

    // Wait for echo LOW
    timeout = 0;
    while (gpio_input_bit_get(GPIOB, GPIO_PIN_14)) {
        delay_1us(1);
        if (++timeout > 30000)
            return 0;
    }
    echo_end = TIMER_CNT(TIMER0);

    if (echo_end >= echo_start)
        return (echo_end - echo_start);
    else
        return 0;
}

void lcd_init(void)
{
    delay_1ms(50);

 
    lcd_write_4bits(0x30);
    delay_1ms(5);
    lcd_write_4bits(0x30);
    delay_1us(150);
    lcd_write_4bits(0x30);
    lcd_write_4bits(0x20); 

    
    lcd_command(0x28); // 4-bit, 2-line, 5x8 dots
    lcd_command(0x08); // display off
    lcd_command(0x01); // clear display
    delay_1ms(2);
    lcd_command(0x06); // entry mode set
    lcd_command(0x0C); // display on, cursor off
}



int main(void)
{
    delay_1ms(1);
    gpio_config();      // I2C LCD GPIO Config
    i2c_config();       
    GPIO_init();        // HC-SR04 GPIO Config
    TIMER0_init();      

    lcd_init();
    lcd_clear();

    uint16_t echo_time_us;
    uint16_t distance_cm;

    while (1)
    {
        echo_time_us = measure_distance();

        lcd_set_cursor(0, 0);  //y0
        lcd_print("Dist: ");
        
        if (echo_time_us == 0) {
            lcd_print("---- cm     ");
        } else {
            float dist_f = (echo_time_us * 0.034f) / 2.0f;
            distance_cm = (uint16_t)(dist_f + 0.5f);
            lcd_print_int(distance_cm);
            lcd_print(" cm     ");
        }

        lcd_set_cursor(0, 1);  //y1

        if (echo_time_us == 0) {
            lcd_print("Out of range   ");
        } else {
            float dist_f = (echo_time_us * 0.034f) / 2.0f;
            distance_cm = (uint16_t)(dist_f + 0.5f);

            uint8_t max_cm = 30;          // minimum distance for full blocks
            uint8_t total_blocks = 16;    // 16x2 LCD 
            uint8_t filled_blocks;

            if (distance_cm >= max_cm)
                filled_blocks = total_blocks;
            else
                filled_blocks = (distance_cm * total_blocks) / max_cm;

            
            for (uint8_t i = 0; i < total_blocks; i++) {
                if (i < filled_blocks)
                    lcd_write(0xFF);  // Block symbol
                else
                    lcd_write(' '); // blank
            }
        }

        delay_1ms(300);
    }
}
