#include "gd32vf103.h"
#include "systick.h"
#include "DHT11.h"
#include <stdio.h>

#define FND_SCLK_PIN GPIO_PIN_13
#define FND_RCLK_PIN GPIO_PIN_14
#define FND_DIO_PIN  GPIO_PIN_15
#define FND_GPIO_PORT GPIOB
#define HIGH 1
#define LOW  0

uint8_t LED_ARR[32];
uint8_t values[5]; //global variable

void FND_init() {
    LED_ARR[0] = 0xC0; //0
    LED_ARR[1] = 0xF9; //1
    LED_ARR[2] = 0xA4; //2
    LED_ARR[3] = 0xB0; //3
    LED_ARR[4] = 0x99; //4
    LED_ARR[5] = 0x92; //5
    LED_ARR[6] = 0x82; //6
    LED_ARR[7] = 0xF8; //7
    LED_ARR[8] = 0x80; //8
    LED_ARR[9] = 0x90; //9
    LED_ARR[28] = 0xFE; // '-'
    LED_ARR[29] = 0xC6; // 'C'
    LED_ARR[30] = 0x89; // 'H'
}

void digitalWrite(uint32_t pin, uint8_t val) {
    if (val == HIGH)
        gpio_bit_set(FND_GPIO_PORT, pin);
    else
        gpio_bit_reset(FND_GPIO_PORT, pin);
}

void send(uint8_t val) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(FND_DIO_PIN, (val & 0x80) ? HIGH : LOW);
        val <<= 1;
        digitalWrite(FND_SCLK_PIN, LOW);
        digitalWrite(FND_SCLK_PIN, HIGH);
    }
}

void send_which(uint8_t val, uint8_t which) {
    send(val);
    send(which);
    digitalWrite(FND_RCLK_PIN, HIGH);
    digitalWrite(FND_RCLK_PIN, LOW);
}


void display_float_aligned(float value, char unit, int repeat) {
    if (unit == 'H' && value > 99.9f) {
        value = 99.9f;
    }

    int int_part = (int)value;
    int frac_part = (int)(value * 10) % 10;

    uint8_t d1 = (unit == 'C') ? LED_ARR[29] : LED_ARR[30];       // unit
    uint8_t d2 = LED_ARR[frac_part];                              // decimal
    uint8_t d3 = LED_ARR[int_part % 10] & 0x7F;                   // 1's place with DP
    uint8_t d4 = LED_ARR[int_part / 10];                          // 10's place

    for (int i = 0; i < repeat; i++) {
        send_which(d4, 0b1000); delay_1ms(5);   // D4 (leftmost)
        send_which(d3, 0b0100); delay_1ms(5);   // D3 (with DP)
        send_which(d2, 0b0010); delay_1ms(5);   // D2
        send_which(d1, 0b0001); delay_1ms(5);   // D1 (rightmost)
    }
}

void display_error(int repeat) {
    for (int i = 0; i < repeat; i++) {
        send_which(LED_ARR[28], 0b0001); delay_1ms(5);
        send_which(LED_ARR[28], 0b0010); delay_1ms(5);
        send_which(LED_ARR[28], 0b0100); delay_1ms(5);
        send_which(LED_ARR[28], 0b1000); delay_1ms(5);
    }
}

void setup(void) {
    SystemInit();
    SystemCoreClockUpdate();
    

    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_bit_set(FND_GPIO_PORT, FND_SCLK_PIN | FND_RCLK_PIN | FND_DIO_PIN);
    gpio_init(FND_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
              FND_SCLK_PIN | FND_RCLK_PIN | FND_DIO_PIN);

    FND_init();
    DHT11_init();
}

int main(void) {
    setup();

    float temperature = 0.0, humidity = 0.0;
    int displayToggle = 0;

    while (1) {
        if (DHT11_get_data() == 0) {
            temperature = values[2] + values[3] * 0.1f;
            humidity = values[0] + values[1] * 0.1f;

            if (displayToggle == 0) {
                display_float_aligned(temperature, 'C', 300);
            } else {
                display_float_aligned(humidity, 'H', 300);
            }

            displayToggle = !displayToggle;
        } else {
            display_error(300);
        }
    }
}
