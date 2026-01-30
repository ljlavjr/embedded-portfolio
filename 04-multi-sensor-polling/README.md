# Multi Sensor Polling
**Description:** This project explores the downside of multi sensor polling in embedded systems. An operation in the code could be skipped over if another operation is ongoing or hanging for some specific value.

## Hardware
- **Chip**: ATmega2560
- **Clock Speed**: 16MHz
- **Pins**: 
  - TX0 (PE1) - Transmit
  - RX0 (PE0) - Receive
  - PB7 - Onboard LED
  - PD2 - External Button connected
  - ADC0 - Our potentiometer
  - ADC1 - Our light sensor

### USART0 Registers Used
- **UDR0** - Data Register (read/write data)
- **UCSR0A** - Status Register (RXC0, TXC0, UDRE0 flags)
- **UCSR0B** - Control Register (RXEN0, TXEN0, RXCIE0 enables)
- **UBRR0H/UBRR0L** - Baud Rate Registers

### ADC Registers Used
- **ADCSRA** - ADC Control and Status Register A (ADEN, ADSC, ADPS bits)
- **ADMUX** - ADC Multiplexer Selection Register (channel select, voltage reference)
- **ADC** - ADC Data Register (10-bit result)

## Build
How to build this project:
### Prerequisites
This is what should be installed (using *sudo apt install ...*):
- **gcc-avr**
- **avr-libc**
- **avrdude**
- **make**
- **screen**
### Compile
```bash
make
```
### Flash
```bash
make flash
```

### Wiring
**Button:**
- One leg to GND
- Other leg to PD2 (uses internal pull-up)

**Potentiometer:**
- Left leg to GND
- Middle leg (wiper) to ADC0 (Analog pin A0)
- Right leg to 5V

**Light Sensor (LDR):**
- One leg to 5V
- Other leg to ADC1 (Analog pin A1) AND to 10K resistor to GND

## Usage
```bash
screen [port location] [baud],cs8,-parenb,-cstopb
```
Here is where you can see the values of the sensors when you adjust them in real time. Note when you click the button nothing will be outputted due to the other two adc sensors potentially blocking the hardware for a moment to calculate their values. 

## What I Learned
### The Super-Loop Problem
In a "super-loop", tasks are executed sequentially. Here is an example:
```c
while (1) {
    task1();
    task2();
    task3();
    delay();
}
```
Lets say that `task1()` is to read if a button has been pressed. If the code is executing the delay function as the button is pressed, then the read will not pick it up. 

### ADC Configuration
To set up the ADC in our hardware you must do the following:
- Enable ADC with the ADEN bit
- Set prescaler for 50-250kHz ADC clock (16Mhz / 128 = 125kHz)
- Select channel and voltage reference in ADMUX
- Start conversion with ADSC, wait for it to clear

### Why RTOS Exists
The blocking issue in this project shows why real-time operating systems exist. With RTOS:
- Button press could be set as high pro, taking precedence over the sensor reads
- Each task runs independently
- No inputs are missed

## Resources
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)