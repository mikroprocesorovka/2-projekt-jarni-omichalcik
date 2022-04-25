#include "stm8s.h"
#include "milis.h"
#include "delay.h"
#include <stdio.h>
#include "spse_stm8.h"
#include "stm8_hd44780.h"
#include "stm8s_adc2.h"
#include "uart1.h"

#define PULSE_LEN 2 // délka spouštěcího (trigger) pulzu pro ultrazvuk
#define MEASURMENT_PERIOD 100 // perioda měření ultrazvukem (měla by být víc jak (maximální_dosah*2)/rychlost_zvuku)



char text[32] = "";
void print(void){
    sprintf(text,"近swag近");
    lcd_clear();
    lcd_puts(text);
    sprintf(text,"123");
    lcd_gotoxy(0,1);
    lcd_puts(text);  
}

// pošle jeden znak na UART
void uart_putchar(char data){
	// počkej až bude UART připraven převzít další data k odeslání
 while(UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET); // čekej dokud je vlajka Transmit Empty vynulovaná (UART vysílá a nemá místo na další data)
 UART1_SendData8(data); // předej data k odeslání UARTu
}

// pošle UARTem řetězec 
void uart_puts(char* retezec){ 
 while(*retezec){ //podívej se do paměti na místo kam ukazuje proměnná "retezec" a pokud je tam nenulový znak (text ještě neskončil) tak ho pošli
  uart_putchar(*retezec); //pošli znak který je na adrese "retezec"
  retezec++; //zvedni adresu o jedna abychom moli zpracovat další znak v poli
 }
}


void init_tim1(void){
GPIO_Init(GPIOC, GPIO_PIN_1, GPIO_MODE_IN_FL_NO_IT); // PC1 (TIM1_CH1) jako echo PC1
TIM1_TimeBaseInit(15,TIM1_COUNTERMODE_UP,0xffff,0); // timer necháme volně běžet (do maximálního stropu) s časovou základnou 1MHz (1us)
// Konfigurujeme parametry capture kanálu 1 - komplikované, nelze popsat v krátkém komentáři
TIM1_ICInit(TIM1_CHANNEL_1,TIM1_ICPOLARITY_RISING,TIM1_ICSELECTION_DIRECTTI,TIM1_ICPSC_DIV1,0);
// Konfigurujeme parametry capture kanálu 2 - komplikované, nelze popsat v krátkém komentáři
TIM1_ICInit(TIM1_CHANNEL_2,TIM1_ICPOLARITY_FALLING,TIM1_ICSELECTION_INDIRECTTI,TIM1_ICPSC_DIV1,0);
TIM1_SelectInputTrigger(TIM1_TS_TI1FP1); // Zdroj signálu pro Clock/Trigger controller 
TIM1_SelectSlaveMode(TIM1_SLAVEMODE_RESET); // Clock/Trigger má po příchodu signálu provést RESET timeru
TIM1_ClearFlag(TIM1_FLAG_CC2); // pro jistotu vyčistíme vlajku signalizující záchyt a změření echo pulzu
TIM1_Cmd(ENABLE); // spustíme timer ať běží na pozadí
}

void process_measurment(void){
	static uint8_t stage=0; // stavový automat
	static uint16_t time=0; // pro časování pomocí milis
	switch(stage){
	case 0:	// čekáme než uplyne  MEASURMENT_PERIOD abychom odstartovali měření
		if(milis()-time > MEASURMENT_PERIOD){
			time = milis(); 
			GPIO_WriteHigh(GPIOC,GPIO_PIN_5); // zahájíme trigger pulz TADYYYYYYYYYYYYYYYYYYYY
			stage = 1; // a bdueme čekat až uplyne čas trigger pulzu
		}
		break;
	case 1: // čekáme než uplyne PULSE_LEN (generuje trigger pulse)
		if(milis()-time > PULSE_LEN){
			GPIO_WriteLow(GPIOC,GPIO_PIN_5); // ukončíme trigger pulz
			stage = 2; // a přejdeme do fáze kdy očekáváme echo
		}
		break;
	case 2: // čekáme jestli dostaneme odezvu (čekáme na echo)
		if(TIM1_GetFlagStatus(TIM1_FLAG_CC2) != RESET){ // hlídáme zda timer hlásí změření pulzu
			capture = TIM1_GetCapture2(); // uložíme výsledek měření
			capture_flag=1; // dáme vědět zbytku programu že máme nový platný výsledek
			stage = 0; // a začneme znovu od začátku
		}else if(milis()-time > MEASURMENT_PERIOD){ // pokud timer nezachytil pulz po dlouhou dobu, tak echo nepřijde
			stage = 0; // a začneme znovu od začátku
		}		
		break;
	default: // pokud se cokoli pokazí
	stage = 0; // začneme znovu od začátku
	}	
}

int main(void)
{
    init_milis(); // milis kvůli delay_ms()
    init_tim1(); // nastavit a spustit timer
    GPIO_Init(GPIOC, GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_SLOW); // výstup - "trigger pulz pro ultrazvuk"
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1); // taktovat MCU na 16MHz
    UART1_Init(9600,UART1_WORDLENGTH_8D,UART1_STOPBITS_1,UART1_PARITY_NO,UART1_SYNCMODE_CLOCK_DISABLE,UART1_MODE_TX_ENABLE);
    UART1_Cmd(ENABLE); // Aktivace UARTu (přebírá kontrolu nad TX pinem - PA5)

    lcd_init();
    print();
    uart_puts(text);
}  
    


/*-------------------------------  Assert -----------------------------------*/
#include "__assert__.h"
