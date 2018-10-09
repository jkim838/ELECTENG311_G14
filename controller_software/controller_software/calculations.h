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

double calculate_power(double coil_voltage, double coil_current, uint8_t PWM_live_time, uint8_t cycle_period);
double calculate_voltage(double ADC_output_PC0);
double calculate_current(double ADC_output_PC5);