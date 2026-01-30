#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "adc.h"

// Button on PD2
#define BUTTON_PIN PD2
#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD
#define BUTTON_INPUT PIND

void print_number(uint16_t num) {
    char buf[6];
    int i = 0;
    
    if (num == 0) {
        uart_transmit('0');
        return;
    }
    
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    while (i > 0) {
        uart_transmit(buf[--i]);
    }
}

int main(void) {
    // Button as input with pull-up
    BUTTON_DDR &= ~(1 << BUTTON_PIN);
    BUTTON_PORT |= (1 << BUTTON_PIN);
    
    // LED for feedback
    DDRB |= (1 << PB7);
    
    uart_init(9600);
    adc_init();
    
    uart_print("Multi-sensor polling started\r\n");
    
    while (1) {
        // Read button (active low because of pull-up)
        if (!(BUTTON_INPUT & (1 << BUTTON_PIN))) {
            uart_print("BUTTON PRESSED!\r\n");
            PORTB ^= (1 << PB7);
        }
        
        // Read potentiometer (ADC0)
        uint16_t pot = adc_read(0);
        uart_print("Pot: ");
        print_number(pot);
        uart_print("\r\n");
        
        // Read light sensor (ADC1)
        uint16_t light = adc_read(1);
        uart_print("Light: ");
        print_number(light);
        uart_print("\r\n");
        
        // Simulate slow processing
        _delay_ms(500);
        
        uart_print("---\r\n");
    }
    
    return 0;
}