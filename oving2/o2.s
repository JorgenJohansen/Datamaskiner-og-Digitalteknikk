.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO
.include "sys-tick_constants.s" // Register-adresser og konstanter for SysTick

.text
	.global Start
	
Start:

    // Skriv din kode her...
    //For EXTIPSELH
    LDR R1, = #0b1111
    LSL R2, R1, 4
    MVN R3, R2
    LDR R0, = GPIO_BASE + GPIO_EXTIPSELH
    LDR R4, [R0]
    AND R5, R3, R4
    LDR R6, = PORT_B
    LSL R7, R6, 4
    ORR R8, R5, R7
    STR R8, [R0]

    //For IEN
    LDR R1, = #0b00000001
    LDR R0, = GPIO_BASE + GPIO_IEN
    LDR R2, [R0]
    LSL R1, R1, #BUTTON_PIN
    ORR R3, R1, R2
    STR R3, [R0]

    //For EXTIFALL
    LDR R1, = #0b00000001
    LDR R0, = GPIO_BASE + GPIO_EXTIFALL
    LDR R2, [R0]
    LSL R1, R1, #BUTTON_PIN
    ORR R3, R1, R2
    STR R3, [R0]
    //R0 is tenths
    //R1 is seconds
    //R2 is minutes
    LDR R0, = tenths
    LDR R1, = seconds
    LDR R2, = minutes
    //store 1 into tenths
    //LDR R3, = 1
    //STR R3, [R0]

    //inititalisere interrupt vektor
    //R4 er load registeret vårt
    LDR R4, = SYSTICK_BASE + SYSTICK_LOAD
    //Sett R5 til et tiendels sekund som er satt til som verdi til adressen R4
    LDR R5, = FREQUENCY / 10
    STR R5, [R4]
    //R6 er VAL registeret vårt
    LDR R6, = SYSTICK_BASE + SYSTICK_VAL
    STR R5, [R6]
    //R7 er vårt CTRL register
    LDR R7, = SYSTICK_BASE + SYSTICK_CTRL
    LDR R4, = 0b110
    STR R4, [R7]



    //For lyset
    LDR R8, = GPIO_BASE + PORT_SIZE * LED_PORT + GPIO_PORT_DOUTTGL
    LDR R9, =1 << LED_PIN

    //For knappen
    LDR R10, = GPIO_BASE + PORT_SIZE * BUTTON_PORT + GPIO_PORT_DIN
    LDR R11, =1 << BUTTON_PIN



Loop:
	wfi
	b Loop

.global GPIO_ODD_IRQHandler
.thumb_func
GPIO_ODD_IRQHandler:
	//Insert kode for knapp here!!!!
    LDR R7, = SYSTICK_BASE + SYSTICK_CTRL
	LDR R11, [R7]
	EOR R11, #SysTick_CTRL_ENABLE_Msk
	STR R11, [R7]

	//STR R9, [R8]
	//clear interrupt flag register for port 9
	LDR R11, = GPIO_BASE + GPIO_IFC
	LDR R12, = 1<<9
	STR R12, [R11]
	BX LR // Returner fra interrupt

.global SysTick_Handler
.thumb_func

SysTick_Handler:
	//Teller opp tiendels sekunder
	LDR R3, [R0]
    ADD R3, #1
    STR R3,[R0]
    //Branching
    CMP R3, #10
    BEQ Seconds

	BX LR // Returner fra interrupt

Seconds:
	//Insert lys kode here!!!!
	STR R9, [R8]
	//Setter tiendels sekunder til null når vi har et sekund
	LDR R5, = 0
	STR R5, [R0]
	//Teller opp sekunder
	LDR R4, [R1]
    ADD R4, #1
    STR R4,[R1]
    CMP R4, #60
    //
    BEQ Minutes
    BX LR // Returner fra interrupt

Minutes:

	STR R5, [R1]
	LDR R5, [R2]
    ADD R5, #1
    STR R5,[R2]
    BX LR // Returner fra interrupt







NOP // Behold denne pÃ¥ bunnen av fila

