/*
 * debug_usart.c
 *
 * Created: 9/15/2018 17:30:46
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This script file contains definitions for functions used for debugging ADC
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "debug_usart.h"

/*** Custom Macro Definitions ***/
#define F_CPU 16000000UL							// Define CPU Operational Clock
#define DATA_REG_IS_FULL !((1 << UDRE0) & UCSR0A)
#define ASCII_CONVERT 48							// Convert to ASCII Characters

/*** Delay Header ***/
#include <util/delay.h>

/*** Function Definitions ***/
void debug_usart_init(uint16_t UBRR){
	
	//Enable USART Transmitter
	UCSR0B |= (1 << TXEN0);
	//UCSR0B |= (1 << RXEN0);
	UCSR0B |= (1 << RXCIE0);
	
	// Sets the mode of the USART to be asynchronous.
	UCSR0C &= ~(1 << UMSEL01);
	UCSR0C &= ~(1 << UMSEL00);
	
	// Disables the parity bit.
	UCSR0C &= ~(1 << UPM01);
	UCSR0C &= ~(1 << UPM00);
	
	// Sets only one stop bit.
	UCSR0C &= ~(1 << USBS0);
	
	// Sets the data width to be 8 bits (UCSZ0[2..0] = 011).
	UCSR0B &= ~(1 << UCSZ02);
	UCSR0C |= ((1 << UCSZ01) | (1 << UCSZ00));
	// Sets the UBRR to be 103.
	UBRR0H = (UBRR >> 8);
	UBRR0L = UBRR;
	
}

void debug_usart_transmit(uint8_t data){
	
	while (DATA_REG_IS_FULL);		// Waits for the UDR register to be empty.
	UDR0 = data;					// Transmits the data.
	_delay_ms(20);

}

void debug_usart_decompose_transmit(uint8_t *output, double input){
	
	input = input * 1000;
	output[3] = (uint16_t)(input / 1000);
	output[2] = (uint16_t)(input / 100) % 100;
	output[1] = (uint16_t)(input / 10) % 10;
	output[0] = (uint16_t)input % 10;
	
	debug_usart_transmit(output[3] + ASCII_CONVERT);
	debug_usart_transmit('.');
	debug_usart_transmit(output[2] + ASCII_CONVERT);
	debug_usart_transmit(output[1] + ASCII_CONVERT);
	debug_usart_transmit(output[0] + ASCII_CONVERT);
	debug_usart_transmit('V');
	debug_usart_transmit(' ');
	
}
