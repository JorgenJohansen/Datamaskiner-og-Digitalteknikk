#include "o3.h"
#include "gpio.h"
#include "systick.h"

#define LED_PORT GPIO_PORT_E
#define LED_PIN 2


#define BTN0_PORT GPIO_PORT_B
#define BTN0_PIN 9
#define BTN1_PORT GPIO_PORT_B
#define BTN1_PIN 10

/**************************************************************************//**
 * @brief Konverterer nummer til string 
 * Konverterer et nummer mellom 0 og 99 til string
 *****************************************************************************/
void int_to_string(char *timestamp, unsigned int offset, int i) {
    if (i > 99) {
        timestamp[offset]   = '9';
        timestamp[offset+1] = '9';
        return;
    }

    while (i > 0) {
	    if (i >= 10) {
		    i -= 10;
		    timestamp[offset]++;
		
	    } else {
		    timestamp[offset+1] = '0' + i;
		    i=0;
	    }
    }
}

/**************************************************************************//**
 * @brief Konverterer 3 tall til en timestamp-string
 * timestamp-argumentet må være et array med plass til (minst) 7 elementer.
 * Det kan deklareres i funksjonen som kaller som "char timestamp[7];"
 * Kallet blir dermed:
 * char timestamp[7];
 * time_to_string(timestamp, h, m, s);
 *****************************************************************************/
void time_to_string(char *timestamp, int h, int m, int s) {
    timestamp[0] = '0';
    timestamp[1] = '0';
    timestamp[2] = '0';
    timestamp[3] = '0';
    timestamp[4] = '0';
    timestamp[5] = '0';
    timestamp[6] = '\0';

    int_to_string(timestamp, 0, h);
    int_to_string(timestamp, 2, m);
    int_to_string(timestamp, 4, s);
}

typedef struct {
    volatile word CTRL;
    volatile word MODEL;
    volatile word MODEH;
    volatile word DOUT;
    volatile word DOUTSET;
    volatile word DOUTCLR;
    volatile word DOUTTGL;
    volatile word DIN;
    volatile word PINLOCKN;
} gpio_port_map_t;

typedef struct {
    volatile gpio_port_map_t ports[6];
    volatile word unused_space[10];
    volatile word EXTIPSELL;
    volatile word EXTIPSELH;
    volatile word EXTIRISE;
    volatile word EXTIFALL;
    volatile word IEN;
    volatile word IF;
    volatile word IFS;
    volatile word IFC;
    volatile word ROUTE;
    volatile word INSENSE;
    volatile word LOCK;
    volatile word CTRL;
    volatile word CMD;
    volatile word EM4WUEN;
    volatile word EM4WUPOL;
    volatile word EM4WUCAUSE;
} gpio_map_t;

typedef struct {
    volatile word CTRL;
    volatile word LOAD;
    volatile word VAL;
    volatile word CALIB;
} systick_map_t;

struct Time {
	int seconds;
	int minutes;
	int hours;
};
struct Time time = {
	.seconds = 0,
	.minutes = 0,
	.hours = 0
};

/*
 * State chart:
 * 0: Adjusting seconds
 * 1: Adjusting minutes
 * 2: Adjusting hours
 * 3: Counting down
 * 4: Alarm is on
 */
int state = 0;

void init_GPIO(void) {
	// GPIO-adresse
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;

	// Set input/output
	GPIO->ports[LED_PORT].MODEL  = (~(0b1111 << (4*LED_PIN))      & GPIO->ports[LED_PORT].MODEL)  | (GPIO_MODE_OUTPUT << (4*LED_PIN));
	GPIO->ports[BTN0_PORT].MODEH = (~(0b1111 << (4*(BTN0_PIN-8))) & GPIO->ports[BTN0_PORT].MODEH) | (GPIO_MODE_INPUT << (4*(BTN0_PIN-8)));
	GPIO->ports[BTN1_PORT].MODEH = (~(0b1111 << (4*(BTN1_PIN-8))) & GPIO->ports[BTN1_PORT].MODEH) | (GPIO_MODE_INPUT << (4*(BTN1_PIN-8)));
	

	//Interrupt port select
	GPIO->EXTIPSELH = (~(0b1111 << (4*(BTN0_PIN-8))) & GPIO->EXTIPSELH) | (GPIO_MODE_INPUT << (4*(BTN0_PIN-8)));
	GPIO->EXTIPSELH = (~(0b1111 << (4*(BTN1_PIN-8))) & GPIO->EXTIPSELH) | (GPIO_MODE_INPUT << (4*(BTN1_PIN-8)));

	//Trigger
	GPIO->EXTIFALL = GPIO->EXTIFALL | 1<<BTN0_PIN;
	GPIO->EXTIFALL = GPIO->EXTIFALL | 1<<BTN1_PIN;

	//Interrupt enable
	GPIO->IEN = GPIO->IEN | 1<<BTN0_PIN;
	GPIO->IEN = GPIO->IEN | 1<<BTN1_PIN;
}

void init_systick(void) {
	systick_map_t *SYSTICK = (systick_map_t*)SYSTICK_BASE;

	SYSTICK->LOAD = FREQUENCY;
	SYSTICK->VAL = FREQUENCY;
	SYSTICK->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
}
void clock_on(void) {
	systick_map_t *SYSTICK = (systick_map_t*)SYSTICK_BASE;
	SYSTICK->CTRL = SYSTICK->CTRL | SysTick_CTRL_ENABLE_Msk;
}
void clock_off(void) {
	systick_map_t *SYSTICK = (systick_map_t*)SYSTICK_BASE;
	SYSTICK->CTRL = SYSTICK->CTRL & ~SysTick_CTRL_ENABLE_Msk;
}
void light_on(void) {
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;
	GPIO->ports[LED_PORT].DOUTSET = 1 << LED_PIN;
}
void light_off(void) {
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;
	GPIO->ports[LED_PORT].DOUTCLR = 1 << LED_PIN;
}
void update_lcd(void) {
	char times[7];
	time_to_string(times, time.hours, time.minutes, time.seconds);
	lcd_write(times);
}
void GPIO_ODD_IRQHandler(void) {// Din interrupt-kode her
	// GPIO-adresse
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;
	if(state==0) {
		time.seconds++;
	}
	else if( state == 1 ) {
		time.minutes++;
	}
	else if(state==2) {
		time.hours++;
	}

	update_lcd();
	GPIO->IFC = 1 << BTN0_PIN;

}
void GPIO_EVEN_IRQHandler(void) {
	// GPIO-adresse
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;

	if(state !=3) { //If we're not counting down

		state = ++state % 5;

		if(state==0){
			light_off();
		}
		if(state == 3) {
			clock_on();
		}

	}


	GPIO->IFC = 1 << BTN1_PIN;
}
void SysTick_Handler(void) {
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;
	// GPIO->ports[LED_PORT].DOUTTGL = 1 << LED_PIN;

	if(time.seconds == 0) {
		if(time.minutes == 0) {
			if(time.hours == 0) {
				clock_off();
				light_on();
				state = 4;
				return;
			}
			--time.hours;
			time.minutes = 60;
		}
		--time.minutes;
		time.seconds = 60;
	}
	--time.seconds;
	update_lcd();
}

int main(void) {
	init();
	init_GPIO();
	init_systick();
	// clock_on();

	// GPIO-adresse
	gpio_map_t *GPIO = (gpio_map_t*)GPIO_BASE;
	// systick_map_t *SYSTICK = (systick_map_t*)SYSTICK_BASE;
	// GPIO->ports[LED_PORT].DOUTTGL = 1 << LED_PIN;
	time.seconds = 0;
	time.minutes = 0;
	time.hours = 0;
	update_lcd();
	while(true);
	return 0;
}