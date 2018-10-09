/*
 * Comm_Setup.c
 *
 * Created: 9/23/2018 11:03:36
 * Author : Oliver K. jkim838 846548800
 * Revision 1.2.7
 *
 * Description:
 * This script file contains definitions for functions related to communication between master and slave system.
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
/*** Custom Header Files ***/
#include "Comm_Setup.h"
#include "Macro_Definitions.h"

void usart_init(uint16_t UBRR){
	
	// Enable USART Transmitter
	UCSR0B |= (1 << TXEN0);
	// Enable USART Receiver
	UCSR0B |= (1 << RXEN0);
	// Enable USART Receive Interrupt
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

void usart_transmit(uint8_t data){
	
	while (DATA_REG_IS_FULL);		// Waits for the UDR register to be empty.
	UDR0 = data;					// Transmits the data.
	
}

int usart_printf(char var, FILE *stream){

	if(var == '\n'){
		usart_transmit('\r');
	}
	usart_transmit(var);
	return 0;

}

void usart_TX_data(uint8_t MOTOR_ID, uint8_t Current_FL, uint16_t numerical_req, double frequency, double expected_power, double coil_current, double coil_voltage, bool req_found, bool clear_error, bool error_collision, bool error_jammed){
	
	// Slave to Master Communication
	// Transmit Report...
	printf("{");
	printf("\"%d\":", MOTOR_ID);
	printf("{");
	if(req_found){
		// If new flow rate command was specified in the master-slave input, then print the following...
		printf("\"mfc\":{\"req"":\"%d\",\"cur\":\"%d\"},\"ver:\"\"001.003.005"",", numerical_req, Current_FL);
	}
	else{
		// If no new flow rate command was specified or overflew in the master-slave input, then flow rate should be "000"...
		printf("\"mfc\":{\"req"":\"%d%d%d\",\"cur\":\"%d\"},\"ver:\"\"001.003.005"",", 0,0,0, Current_FL);
	}
	// Display operating conditions...
	printf("\"param\":{\"pwr\":\"%0.2fW\",\"freq\":\"%0.1fHz\",\"curr\":\"%0.1fA\",\"volt\":\"%0.2fV\"},", expected_power, frequency, coil_current, coil_voltage);
	// if error is not cleared, when there are either of errors present...
	if(!clear_error && (error_collision || error_jammed)){
		// display error clearance message...
		printf("\"clr\":\"ew\",");
		// if only collision happend...
		if(!error_jammed && error_collision){
			printf("\"ew\":[\"           \",\"pistonCollision\"]");
		}
		// if only jam happend...
		else if(error_jammed && !error_collision){
			printf("\"ew\":[\"cmprStalled\",\"               \"]");
		}
		// if both happend...
		else if(error_jammed && error_collision){
			printf("\"ew\":[\"cmprStalled\",\"pistonCollision\"]");
		}
	}
	// if error is cleared by the user, OR no errors are present in the system...
	else if(clear_error || (!error_jammed && !error_collision)){
		printf("           "); // eleven spaces
		printf("                                  "); //31 spaces
	}
	printf("}");
	printf("}");
	
	// just to make the terminal look nicer...
	printf("\n");
	
}