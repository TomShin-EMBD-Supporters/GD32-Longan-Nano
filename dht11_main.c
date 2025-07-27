#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h"
#include "DHT11.h"
#include <stdio.h>

uint8_t text[20];

void main(void)
{
    uint8_t state;
    float temperature = 0.0;
    float humidity = 0.0;
    uint8_t values[5];

    SystemInit();
    SystemCoreClockUpdate();

    OLED_init();
    OLED_clear_screen();   
    DHT11_init();

    while (1)
    {
        state = DHT11_get_data(values);  

        OLED_clear_buffer();

        if (state == 0)   
        {
            temperature = values[2] ;
            humidity = values[0];
            int rest_T =  values[3];
            int rest_H = values[1]; 

           sprintf((char*)text, "Temp: %d.%d C", (int)temperature, rest_T);
          OLED_print_string(0, 2, text);

          sprintf((char*)text, "Humid: %d.%d %%", (int)humidity, rest_H);
          OLED_print_string(0, 4, text);

        }
       
        delay_1ms(1000);
    }
}
