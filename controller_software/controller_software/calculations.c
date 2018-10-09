/*
 * calculations.c
 *
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This script file contains definitions for functions used to calculate parameter values.
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "calculations.h"
#include "Macro_Definitions.h"

/*** Function Definitions ***/
double calculate_voltage(double raw_ADC_output_PC0){
	static double real_voltage;
	real_voltage = raw_ADC_output_PC0 * (1 + VOLTAGE_SHUNT_A/VOLTAGE_SHUNT_B);	 
	return real_voltage;
}

double calculate_current(double raw_ADC_output_PC5){
	static double real_current;
	real_current =  raw_ADC_output_PC5 / CURRENT_RESISTOR_C;
	return real_current;
}

double calculate_power(double coil_voltage, double coil_current, uint8_t PWM_live_time, uint8_t cycle_period){
	
	static double total_power;
	static double average_power;
	// convert binary coil voltage into a number
	// Vcc = 5V, ADC Channel is 10 bits, therefore ADC Step Size = 5/2^10 = 4.88mV
	// therefore if raw ADC output is dec 512 (i.e. 0x200 hex), then analog value is 2.5v
	total_power = coil_voltage * coil_current;
	average_power = total_power * PWM_live_time / cycle_period;
	return average_power;
	
}
