# UART Interrupts
**Description:** This project uses UART functionality on the ATmega2560 to create a mini application that can turn off and on the onboard led using commands typed out by the user using screen or minicom. Unlike 02-uart-driver, however, we will be using interrupts instead of polling the register and a ring buffer for command storage.

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
- **UCSR0B** - Control Register (RXEN0, TXEN0, RXCIE0 enables)
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
### Using Hardware Interrupts
Instead of polling the registers and waiting until the flags change, we can use interrupts to trigger the gathering of data. We enable the RX Complete Interrupt bit to then create a ISR function that handles grabbing what was received over UART and pushing that to a ring buffer to be used later.
*Enabling the Interrupt*
```c
// Enable TX, RX, and RX Complete Interrupt
UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);
```
*Creating a Interrupt Service Routine function looking at the correct register (USART0_RX_vect)*
```c
ISR(USART0_RX_vect) {
    char c = UDR0;
    ring_buffer_put(&rx_buffer, c);
}
```

### Using a Ring Buffer
For interrupts to work properly, we need to store received data somewhere until main code is ready to process it. This is where a ring buffer comes in.

A ring buffer is a fixed-size array with two indices:
- **head** — where the next byte will be written
- **tail** — where the next byte will be read

When a character arrives, the ISR stores it at `head` and increments `head`. When main code reads, it takes from `tail` and increments `tail`. Both indices wrap around using modulo: `(index + 1) % BUFFER_SIZE`.
```c
bool ring_buffer_put(volatile RingBuffer_t *rb, char c) {
    uint8_t next = (rb->head + 1) % RING_BUFFER_SIZE;
    if (next == rb->tail) return false;  // buffer full
    rb->buffer[rb->head] = c;
    rb->head = next;
    return true;
}
```

The buffer is empty when `head == tail`. It's full when the next head position would equal tail.


### The `volatile` Keyword
The ring buffer is accessed by both the ISR and main code. Without `volatile`, the compiler might optimize away reads by caching values in registers instead of reading from memory. This breaks everything because main code would never see the ISR's updates.
```c
static volatile RingBuffer_t rx_buffer;
```

`volatile` tells the compiler: "This variable can change at any time. Always read from memory."

## Resources
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)