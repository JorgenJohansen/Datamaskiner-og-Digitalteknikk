.thumb
.syntax unified

.include "gpio_constants.s"     // Register-adresser og konstanter for GPIO

.text
	.global Start
	
Start:
    //LED
    LDR R0, = GPIO_BASE + PORT_SIZE * LED_PORT + GPIO_PORT_DOUTSET
    LDR R7, = GPIO_BASE + PORT_SIZE * LED_PORT + GPIO_PORT_DOUTCLR
    LDR R1, =1 << LED_PIN


    //Button
    LDR R2, = GPIO_BASE + PORT_SIZE * BUTTON_PORT + GPIO_PORT_DIN
    LDR R3, =1 << BUTTON_PIN


Loop:
	LDR R6, [R2]
    AND R6, R6, R3
    CMP R6, R3
    BNE On

Off:
    STR R1, [R7]

    B Loop

On:
    //lsl r1, r1, 1
    //STR R1, [R0]
    STR R1, [R0]
	B Loop






NOP // Behold denne på bunnen av fila

