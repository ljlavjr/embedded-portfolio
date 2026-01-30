#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
    // Enable the ADC
    // Set the prescalar to 128 (At 16Mhz w/ prescalar of 128: ADC clock = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint16_t adc_read(uint8_t channel) {
    // Set the channel (0-7) and use AVCC as reference voltage
    ADMUX = (channel & 0x07) | (1 << REFS0);
    // Start the conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to complete (ADSC clears when done)
    while ( ADCSRA & (1 << ADSC) );
    // Return 10-bit result
    return ADC;
}