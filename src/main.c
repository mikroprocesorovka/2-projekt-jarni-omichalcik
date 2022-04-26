#include "stm8s.h"
#include "milis.h"
#include "spse_stm8.h"
/*#include "delay.h"*/

#include <stdio.h>
#include "stm8s_adc2.h"
#include "uart1.h"

#define _ISOC99_SOURCE
#define _GNU_SOURCE

#define LED_PORT GPIOC
#define LED_PIN  GPIO_PIN_5
#define LED_REVERSE GPIO_WriteReverse(LED_PORT, LED_PIN)

void setup(void)
{
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);      // taktovani MCU na 16MHz
    GPIO_Init(LED_PORT, LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
    ADC2_SchmittTriggerConfig(ADC2_SCHMITTTRIG_CHANNEL4, DISABLE); //PB4
    // nastavíme clock pro ADC (16MHz / 4 = 4MHz)
    ADC2_PrescalerConfig(ADC2_PRESSEL_FCPU_D4);
    // volíme zarovnání výsledku (typicky vpravo, jen vyjmečně je výhodné vlevo)
    ADC2_AlignConfig(ADC2_ALIGN_RIGHT);
    // nasatvíme multiplexer na některý ze vstupních kanálů
    ADC2_Select_Channel(ADC2_CHANNEL_4);
    // rozběhneme AD převodník
    ADC2_Cmd(ENABLE);
    // počkáme než se AD převodník rozběhne (~7us)
    ADC2_Startup_Wait();

    init_milis();
    init_uart1();
}


int main(void)
{
    uint32_t time = 0;
    uint16_t ADCx;
    uint16_t napeti;
    uint16_t teplota;

    setup();

    while (1) {

        if (milis() - time > 333) {
            LED_REVERSE; 
            time = milis();
            ADCx = ADC_get(ADC2_CHANNEL_4);
            napeti = (uint32_t)3300 * ADCx / 1024;
            teplota = ((uint32_t)33000 * ADCx - 4096000)/ 19968;
            printf("U = %dmV\r\nTeplota = %d.%d°C\r\n", napeti, teplota/10, teplota%10);

        }

    }
}

/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
