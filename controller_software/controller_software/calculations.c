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

/*** Custom Macro Definitions ***/
#define CALCULATION_DEBUG_MODE
#define ADC_REFERENCE_VOLTAGE 5
#define ADC_RESOLUTION 1024

#ifdef CALCULATION_DEBUG_MODE
	#define VOLTAGE_SHUNT_A 1.6
	#define VOLTAGE_SHUNT_B 1
	#define CURRENT_RESISTOR_C 0.150
#else
	#define VOLTAGE_SHUNT_A	// Insert value for voltage shunt resistor A 
	#define VOLTAGE_SHUNT_B // Insert value for voltage shunt resistor B
	#define CURRENT_RESISTOR_C // Insert value for current resistor C
#endif

/*** Function Definitions ***/
double calculate_length(volatile uint8_t* raw_hall_voltage_address){
	
	double stroke_length;
	
	#ifdef CALCULATION_DEBUG_MODE
	return 0;
	
	#else
	return stroke_length;
	
	#endif
	
}

double calculate_period(volatile uint8_t* raw_hall_voltage_address){
	
	double period;
	
	#ifdef CALCULATION_DEBUG_MODE
	return 0;
	
	#else
	return period;
	
	#endif
	
}

double calculate_flow_rate(uint8_t parameter1, uint8_t parameter2){

	double flow_rate;
	
	#ifdef CALCULATION_DEBUG_MODE
	return 0;
	
	#else
	return flow_rate;
	
	#endif

}

double calculate_power(double coil_voltage, double coil_current, double PWM_live_time, double cycle_period){
	
	static double analog_coil_voltage;
	static double analog_coil_current;
	static double total_power;
	static double average_power;
	// convert binary coil voltage into a number
	// Vcc = 5V, ADC Channel is 10 bits, therefore ADC Step Size = 5/2^10 = 4.88mV
	// therefore if raw ADC output is dec 512 (i.e. 0x200 hex), then analog value is 2.5v
	
	// convert binary coil voltage (i.e. ADC output) to an analog value...
	analog_coil_voltage = coil_voltage * (1 + VOLTAGE_SHUNT_A/VOLTAGE_SHUNT_B);	 
	// convert binary coil current (i.e. ADC output) to an analog value...
	analog_coil_current = coil_current / CURRENT_RESISTOR_C;
	total_power = analog_coil_voltage * analog_coil_current;
	average_power = total_power * (PWM_live_time / cycle_period);
	return average_power;
	
}