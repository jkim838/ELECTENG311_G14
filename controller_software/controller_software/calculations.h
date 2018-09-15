/*
 * calculations.h
 *
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This header file contains definitions for functions related to Timer and PWM.
 *
 */

double calculate_length(volatile uint8_t* raw_hall_voltage_address);
double calculate_period(volatile uint8_t* raw_hall_voltage_address);
double calculate_flow_rate(uint8_t parameter1, uint8_t parameter2);
double calculate_power(uint8_t raw_coil_voltage, uint8_t raw_coil_current, double PWM_live_time, double cycle_period);