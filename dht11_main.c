#include "gd32vf103.h"
#include "systick.h"
#include "SSD1306.h"
#include "DHT11.h"
#include <stdio.h>

 
uint8_t values[5];
uint8_t text[20];
 
void main(void)
{
    uint8_t state;
    float temperature = 0.0;
    float humidity = 0.0;

    SystemInit();
    SystemCoreClockUpdate();

    OLED_init();
    OLED_clear_screen();   
    DHT11_init();

    while (1)
    {
        state = DHT11_get_data(); 

        OLED_clear_buffer();

        if (state == 0)   
        {
            temperature = values[2] + values[3] * 0.1;
            humidity = values[0] + values[1] * 0.1;

            sprintf((char*)text, "Temp: %.2f C", temperature);
            OLED_print_string(0, 2, text);

            sprintf((char*)text, "Humid: %.2f %%", humidity);
            OLED_print_string(0, 4, text);
        }
        else if (state == 1)
        {
            OLED_print_string(0, 2, "                      "); //No Sensor Response
        }
        else if (state == 2)
        {
            OLED_print_string(0, 2, "Checksum error");
        }
        
        delay_1ms(1000);
    }
}
