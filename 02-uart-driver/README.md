# UART Driver
**Description:** This project uses UART functionality on the ATmega2560 to create a mini application that can turn off and on the onboard led using commands typed out by the user using screen or minicom.

## Hardware
- **Chip**: ATmega2560
- **Clock Speed**: 16MHz
- **Pins**: 
  - TX0 (PE1) - Transmit
  - RX0 (PE0) - Receive
  - PB7 - Onboard LED

### USART0 Registers Used
- **UDR0** - Data Register (read/write data)
- **UCSR0A** - Status Register (RXC0, TXC0, UDRE0 flags)
- **UCSR0B** - Control Register (RXEN0, TXEN0 enables)
- **UBRR0H/UBRR0L** - Baud Rate Registers

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

## Usage
```bash
screen [port location] [baud],cs8,-parenb,-cstopb
```
Here is where you can send commands like "led on" or "led off" to control the onboard LED using UART.

## What I Learned
### Baud Rate Calculation
*UBRR = (F_CPU / (16 * baud)) - 1*
For 9600 baud at 16MHz, UBRR = 103. Then you need to set this value in register UBRR0. To do this you need to set both the high byte (UBRR0H) and low byte (UBRR0L) of the register.
```c
UBRR0H = (ubrr >> 8);
UBRR0L = ubrr; 
```
### Polling for Hardware Ready
Before transmitting you need to check the UDRE0 flag in UCSR0A and wait until that flag is set. If UDRE0 is set to 1 then the transmit buffer is empty and can accept data to be sent.
```c
while ( !(UCSR0A & (1<<UDRE0)) );
```
Before reading you need to check the RXC0 flag in UCSR0A and wait until that flag is set. If RXC0 is set to 1 then there is data in UDR0 to then be received and used.
```c
while ( !(UCSR0A & (1<<RXC0)) );
```

### Driver Architecture
To create a driver architecture, the code was separated into interface and implementation. The interface of the driver can be found in uart.h where the function defintions are declared. The implementation of these functions can be found in uart.c. This makes it where I am able to reuse this code with any other project I may make in the future. (Look 03-uart-interrupt)

## Resources
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)