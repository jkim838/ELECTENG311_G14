/*
 * adc_setup.c
 *
 * Created: 9/14/2018 18:17:46
 * Author: Oliver K jkim838 846548800
 *
 * Revision 1.1.5
 *
 * This script file contains definitions for functions related to Analog to Digital Converter.
 */ 

/*** Header Files ***/
#include <avr/io.h>
#include <stdint.h>

/*** Custom Header Files ***/
#include "adc_setup.h"
#include "Macro_Definitions.h"

/*** Function Definitions ***/
void adc_init(){
	
	/** ADCMUX: ADC Multiplexer Selection Register **/
	/* Voltage Reference Selection */
	ADMUX &= ~(1 << REFS1);			// Use Vcc as reference voltage
	ADMUX |=  (1 << REFS0);
	/* ADC Left Adjust Result */
	ADMUX &= ~(1 << ADLAR);			// Right adjust ADC reading to ADCH
	/* Analog Channel Selection Bits */
	// Change input pin as specified by the data sheet...
	ADMUX &= ~(1 << MUX3);							// Channel 0 (Coil Voltage Sensor): Input pin is PC0
	ADMUX &= ~(1 << MUX2);
	ADMUX &= ~(1 << MUX1);
	ADMUX &= ~(1 << MUX0);
	ADC_next_channel = ADC_COIL_VOLTAGE_CHANNEL;	// Note: Although it is named ADC_next_channel, in this function specifically this variable signifies initial state of the ADC Channel.
	
	/** ADCSRA: ADC Control and Status Register A **/
	/* ADC Enable */
	ADCSRA |= (1 << ADEN);			// ADC Power 'On'
	/* ADC Auto Trigger */
	#ifdef ADC_ENABLE_AUTO_TRIGGER
		ADCSRA |= (1 << ADATE);			// Auto Trigger 'On'
	#endif
	/* ADC Interrupt Setting */
	//	ADCSRA &=~(1 << ADIF);
	ADCSRA |= (1 << ADIE);			// Interrupt 'On'
	/* Pre-scaler Setup */
	ADCSRA |= (1 << ADPS2);			// Pre-scaler Division factor to 64
	ADCSRA |= (1 << ADPS1);			// ADC Frequency = 125 KHz @Xplained Mini (F_CPU 16MHz)
	ADCSRA |= (1 << ADPS0);			// ADC Frequency = 62.5 KHz @PCB (F_CPU 8MHz)
	
	/** ADCSRB: ADC Control and Status Register B **/
	/* ADC Auto Trigger Source */
	// NOTE: If ADATE is cleared, ADTS will have no effect
	#ifdef ADC_ENABLE_AUTO_TRIGGER
		ADCSRB &=~(1 << ADTS0);			// Timer/Counter 1 Overflow
		ADCSRB |= (1 << ADTS1);
		ADCSRB |= (1 << ADTS2);
	#endif

	/** ADC Begin **/
	ADCSRA |= (1 << ADSC);			// Start new conversion
	
}

double debug_adc_digitize(uint16_t debug_raw_output){
	
	double digitized_ADC_output = (double) debug_raw_output * ADC_REFERENCE_VOLTAGE / ADC_RESOLUTION;
	return digitized_ADC_output;
	
}