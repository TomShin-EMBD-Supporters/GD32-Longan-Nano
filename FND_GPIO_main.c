#include "gd32vf103.h"
#include "systick.h"
#include <stdio.h>
#include <string.h>

// define parameter
#define FND_SCLK_PIN GPIO_PIN_13
#define FND_RCLK_PIN GPIO_PIN_14
#define FND_DIO_PIN  GPIO_PIN_15
#define FND_GPIO_PORT GPIOB

// define HIGH/LOW 
#define HIGH 1
#define LOW  0
#define true 1
#define false 0

// symbols
uint8_t LED_ARR[29];

// ────────────────────────────────
// FND dictionary
// ────────────────────────────────

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
    LED_ARR[10] = 0x88; //A
    LED_ARR[11] = 0x83; //b
    LED_ARR[12] = 0xC6; //C
    LED_ARR[13] = 0xA1; //d
    LED_ARR[14] = 0x86; //E
    LED_ARR[15] = 0x8E; //F
    LED_ARR[16] = 0xC2; //G
    LED_ARR[17] = 0x89; //H
    LED_ARR[18] = 0xF9; //I
    LED_ARR[19] = 0xF1; //J
    LED_ARR[20] = 0xC3; //L
    LED_ARR[21] = 0xA9; //n
    LED_ARR[22] = 0xC0; //O
    LED_ARR[23] = 0x8C; //P
    LED_ARR[24] = 0x98; //q
    LED_ARR[25] = 0x92; //S
    LED_ARR[26] = 0xC1; //U
    LED_ARR[27] = 0x91; //Y
    LED_ARR[28] = 0xFE; // -
}

void digitalWrite(uint32_t pin, uint8_t val) {
    if (val == HIGH) {
        gpio_bit_set(FND_GPIO_PORT, pin);
    } else {
        gpio_bit_reset(FND_GPIO_PORT, pin);
    }
}

void send(uint8_t VAL) {
    for (int i = 8; i >= 1; i--) {
        digitalWrite(FND_DIO_PIN, (VAL & 0x80) ? HIGH : LOW);
        VAL <<= 1;

        digitalWrite(FND_SCLK_PIN, LOW);
        digitalWrite(FND_SCLK_PIN, HIGH);
    }
}

void send_which(uint8_t VAL, uint8_t which) {
    send(VAL);
    send(which);

    digitalWrite(FND_RCLK_PIN, HIGH);
    digitalWrite(FND_RCLK_PIN, LOW);
}

void digit4_show(int n, int repeat, uint8_t yesZero) {
    int n1 = n % 10;
    int n2 = (n % 100) / 10;
    int n3 = (n % 1000) / 100;
    int n4 = (n % 10000) / 1000;

    for (int i = 0; i <= repeat; i++) {
        send_which(LED_ARR[n1], 0b0001);
        if (yesZero || n > 9)   send_which(LED_ARR[n2], 0b0010);
        if (yesZero || n > 99)  send_which(LED_ARR[n3], 0b0100);
        if (yesZero || n > 999) send_which(LED_ARR[n4], 0b1000);
    }
}

void digit4(int n) {
    digit4_show(n, 5000, false);
     
}

void digit4yesZero(int n) {
    digit4_show(n, 5000, true);
}
 

void setup(void) {
    SystemInit();
    SystemCoreClockUpdate();
    

     
    rcu_periph_clock_enable(RCU_GPIOB);

     
    gpio_bit_set(FND_GPIO_PORT, FND_SCLK_PIN | FND_RCLK_PIN | FND_DIO_PIN);
    gpio_init(FND_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
              FND_SCLK_PIN | FND_RCLK_PIN | FND_DIO_PIN);

    FND_init();
}

int main(void) {
    setup();

    while (1) {
        
        //count 1 to 4
        send_which(LED_ARR[1], 0b0001);
        delay_1ms(1000);
        send_which(LED_ARR[2], 0b0010);
        delay_1ms(1000);
        send_which(LED_ARR[3], 0b0100);
        delay_1ms(1000);
        send_which(LED_ARR[4], 0b1000);
        delay_1ms(1000);

        // shift pattern start 
        for (uint8_t t = 0; t < 4; t++) {
            send_which(0b11111110, 0b0001 << t);
            delay_1ms(500);
        }

        send_which(0b11011111, 0b1000);
        delay_1ms(500);
        send_which(0b11101111, 0b1000);
        delay_1ms(500);

        for (uint8_t t = 0; t < 4; t++) {
            send_which(0b11110111, 0b1000 >> t);
            delay_1ms(500);
        }

        send_which(0b11111011, 0b0001);
        delay_1ms(500);
        send_which(0b11111101, 0b0001);
        delay_1ms(500);
        // shift pattern end


        //4 digits related tests
        digit4(123);
        
        digit4yesZero(12);
         
   }
}
