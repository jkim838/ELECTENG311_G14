/*
 * adc_setup.h
 *
 * Created: 9/14/2018 18:17:59
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This header file contains prototyping for functions related to Analog to Digital Converter.
 *
 */ 

/*** Global VariablWes ***/
uint8_t ADC_next_channel;

/*** Function Prototypes ***/
void adc_init();
double adc_digitize(uint16_t debug_raw_output);
