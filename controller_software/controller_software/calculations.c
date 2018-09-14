/*
 * calculations.c
 *
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.4
 *
 * This header file contains definitions for functions used to calculate parameter values.
 *
 */

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "calculations.h"

/*** Custom Macro Definitions ***/
#define CALCULATION_DEBUG_MODE

/*** Function Definitions ***/
double calculate_length(volatile double* hall_voltage_address){
	
	double stroke_length;
	
	#ifdef CALCULATION_DEBUG_MODE
	return 0;
	
	#else
	return stroke_length;
	
	#endif
	
}

double calculate_period(volatile double* hall_voltage_address){
	
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

double calculate_power(double coil_voltage, double coil_current){

	double power;
	
	#ifdef CALCULATION_DEBUG_MODE
	return 0;
	
	#else
	return power;
	
	#endif

}