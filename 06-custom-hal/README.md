# Project 06: Custom HAL for STM32F407

## Overview
This project builds a reusable Hardware Abstraction Layer (HAL) for the STM32F407. Instead of manipulating raw register addresses throughout application code, we create clean functions like `gpio_init()`, `uart_write_string()`, and `spi_transfer()` that hide the hardware details. This makes code more readable, portable, and less error-prone.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4)
- **LEDs Used**: PD12 (Green), PD14 (Red)
- **UART**: USART2 (PA2 TX, PA3 RX) via USB-to-TTL adapter
- **SPI**: SPI1 (PA5 SCK, PA6 MISO, PA7 MOSI)

## Concepts Learned
- Hardware abstraction layers and why they matter
- Register structs for memory-mapped I/O
- API design for embedded drivers
- Clear-then-set pattern for multi-bit register fields
- Atomic register access using BSRR
- Alternate function configuration for peripherals
- SPI master mode and software slave management
- Baud rate calculation for UART

## File Structure
```
06-custom-hal/
├── src/
│   ├── main.c
│   ├── startup.s
│   └── hal/
│       ├── gpio.c
│       ├── uart.c
│       └── spi.c
├── include/
│   ├── stm32f407.h
│   └── hal/
│       ├── gpio.h
│       ├── uart.h
│       └── spi.h
├── linker.ld
└── Makefile
```

## Register Struct Approach

Instead of defining individual register addresses:
```c
#define GPIOD_MODER  (*(volatile uint32_t *)0x40020C00)
#define GPIOD_OTYPER (*(volatile uint32_t *)0x40020C04)
#define GPIOD_ODR    (*(volatile uint32_t *)0x40020C14)
```

We define a struct that mirrors the register layout:
```c
typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

#define GPIOD ((GPIO_TypeDef *) 0x40020C00)
```

Now we access registers as `GPIOD->MODER`, `GPIOD->ODR`, etc. This is cleaner and matches how vendor libraries like CMSIS work.

## API Reference

### GPIO Functions

#### rcc_gpio_clock_enable()
Enables the peripheral clock for a GPIO port. Must be called before using any GPIO functions.
```c
void rcc_gpio_clock_enable(GPIO_TypeDef *port);
```

**Example:**
```c
rcc_gpio_clock_enable(GPIOD);
```

#### gpio_init()
Configures a pin's mode (input, output, alternate function, or analog).
```c
void gpio_init(GPIO_TypeDef *port, uint8_t pin, GPIO_MODE mode);
```

**Modes:**
- `GPIO_MODE_INPUT` — Digital input
- `GPIO_MODE_OUTPUT` — Digital output
- `GPIO_MODE_ALT` — Alternate function (UART, SPI, etc.)
- `GPIO_MODE_AN` — Analog (ADC)

**Example:**
```c
gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT);
```

#### gpio_set_otype()
Sets output type for a pin.
```c
void gpio_set_otype(GPIO_TypeDef *port, uint8_t pin, GPIO_OUTPUT_TYPE otype);
```

**Types:**
- `GPIO_OUTTYPE_PUSHPULL` — Default, drives high and low
- `GPIO_OUTTYPE_OPENDRAIN` — Only drives low, needs external pull-up

#### gpio_set_speed()
Sets output speed (slew rate) for a pin.
```c
void gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_SPEED speed);
```

**Speeds:** `GPIO_SPEED_LOW`, `GPIO_SPEED_MEDIUM`, `GPIO_SPEED_HIGH`, `GPIO_SPEED_VHIGH`

#### gpio_set_pull()
Configures internal pull-up/pull-down resistors.
```c
void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GPIO_PULL_UD pull);
```

**Options:** `GPIO_PULL_UD_NOPUPD`, `GPIO_PULL_UD_PULLU`, `GPIO_PULL_UD_PULLD`

#### gpio_set_alt_func()
Sets the alternate function for a pin. Required for UART, SPI, I2C, etc.
```c
void gpio_set_alt_func(GPIO_TypeDef *port, uint8_t pin, uint8_t af);
```

**Example:**
```c
gpio_set_alt_func(GPIOA, 2, 0x7);  // AF7 for USART2
```

#### write_pin()
Sets a pin high or low.
```c
void write_pin(GPIO_TypeDef *port, uint8_t pin, uint32_t value);
```

**Example:**
```c
write_pin(GPIOD, 14, HIGH);
write_pin(GPIOD, 14, LOW);
```

#### read_pin()
Reads the current state of a pin. Returns 0 or 1.
```c
uint8_t read_pin(GPIO_TypeDef *port, uint8_t pin);
```

**Example:**
```c
if (read_pin(GPIOA, 0)) {
    // Button pressed
}
```

#### toggle_pin()
Inverts the current state of an output pin.
```c
void toggle_pin(GPIO_TypeDef *port, uint8_t pin);
```

---

### UART Functions

#### uart_init()
Initializes USART2 with the specified baud rate. Configures PA2 (TX) and PA3 (RX) automatically.
```c
void uart_init(uint32_t baud_rate);
```

**Example:**
```c
uart_init(9600);
```

#### uart_write_char()
Sends a single character over UART. Blocks until transmit buffer is empty.
```c
void uart_write_char(char c);
```

#### uart_write_string()
Sends a null-terminated string over UART.
```c
void uart_write_string(const char *str);
```

**Example:**
```c
uart_write_string("Hello from STM32!\r\n");
```

#### uart_write_hex()
Sends a byte as two hex characters (e.g., 0xA5 sends "A5").
```c
void uart_write_hex(uint8_t val);
```

#### uart_read_char()
Waits for and returns a single character from UART. Blocks until data is received.
```c
char uart_read_char(void);
```

---

### SPI Functions

#### spi_init()
Initializes SPI1 in master mode. Configures PA5 (SCK), PA6 (MISO), PA7 (MOSI) automatically. Uses software slave management.
```c
void spi_init(void);
```

**Example:**
```c
spi_init();
```

#### spi_transfer()
Sends one byte and simultaneously receives one byte. This is how SPI works—data is shifted out on MOSI while data is shifted in on MISO.
```c
uint8_t spi_transfer(uint8_t data);
```

**Example:**
```c
uint8_t received = spi_transfer(0xA5);
```

**Loopback test (connect MOSI to MISO):**
```c
uint8_t sent = 0x42;
uint8_t received = spi_transfer(sent);
// sent == received if loopback wired correctly
```

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## UART Output
Connect a USB-to-TTL adapter:
- Adapter RX → PA2 (STM32 TX)
- Adapter TX → PA3 (STM32 RX)
- Adapter GND → STM32 GND

View output:
```bash
screen /dev/ttyUSB0 9600
```

## Design Decisions

### Explicit Clock Enable
We require the user to call `rcc_gpio_clock_enable()` before using a port, rather than enabling clocks automatically inside `gpio_init()`. This makes the dependency explicit in the code and matches how other peripherals (UART, SPI, timers) work. The pattern "enable clock, then init" becomes muscle memory.

For UART and SPI, clock enable is handled internally since those drivers configure their own GPIO pins—bundling it makes sense there.

### Separate Configuration Functions
Instead of one large init function with many parameters:
```c
gpio_init(port, pin, mode, otype, speed, pull);  // Too many params
```

We use the mode in init and provide separate functions for other settings:
```c
gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT);  // Common case is simple
gpio_set_speed(GPIOD, 14, GPIO_SPEED_HIGH);  // Only when needed
```

Most pins only need mode set. Output type, speed, and pull-up/pull-down have sensible defaults (push-pull, low speed, no pull).

### BSRR vs ODR for Writes
We use the BSRR register instead of ODR for `write_pin()`. BSRR is write-only and atomic—writing a 1 to bits 0-15 sets the pin, writing a 1 to bits 16-31 clears it. No read-modify-write cycle needed, so no risk of race conditions with interrupts.

### Software Slave Management for SPI
SPI1 is configured with SSM and SSI bits set, meaning the NSS pin is managed in software. This prevents the SPI from accidentally entering slave mode and allows manual chip select control via GPIO.

## Key Patterns

### Clear-Then-Set for Multi-Bit Fields
MODER uses 2 bits per pin. To change a value safely:
```c
port->MODER &= ~(0x3 << (pin * 2));   // Clear both bits
port->MODER |= (mode << (pin * 2));   // Set new value
```

Just OR-ing would fail if bits were already set. This pattern applies to OSPEEDR, PUPDR, AFR, and many other registers.

### AFR Uses 4 Bits Per Pin
The alternate function register is split into AFR[0] (pins 0-7) and AFR[1] (pins 8-15), with 4 bits per pin:
```c
uint8_t index = pin / 8;
uint8_t shift = (pin % 8) * 4;
port->AFR[index] &= ~(0xF << shift);
port->AFR[index] |= (af << shift);
```

### Bit Position Calculation
- 1-bit fields: `pin`
- 2-bit fields: `pin * 2`
- 4-bit fields: `pin * 4` (or `(pin % 8) * 4` for AFR)

### Extracting a Single Bit
Shift right to bring the bit to position 0, then mask:
```c
return (port->IDR >> pin) & 0x1;
```

### Waiting for Status Bits
Polling loops check status register bits before read/write operations:
```c
while (!(USART2->SR & (1 << TXE)));  // Wait for transmit empty
while (!(SPI1->SR & (1 << RXNE)));   // Wait for receive not empty
```

## What I Learned

**Why abstraction matters:** Raw register access works, but it's error-prone and hard to read. A simple function like `gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT)` is self-documenting. Six months from now, I'll understand what that line does without checking the reference manual.

**Register structs:** Mapping a struct to a peripheral's register block is elegant. The compiler handles offset math, and access looks clean: `GPIOD->MODER` instead of `*(volatile uint32_t *)0x40020C00`.

**Design tradeoffs:** There's no single "correct" API design. The choice depends on what you value—simplicity, explicitness, or safety. I chose explicit control for GPIO clocks and implicit setup for UART/SPI since those drivers manage their own pins.

**Peripheral clocks:** STM32 keeps peripheral clocks off by default to save power. GPIO is on AHB1, USART2 is on APB1, SPI1 is on APB2. Forgetting to enable clocks is a common debugging headache.

**SPI quirks:** Software slave management (SSM/SSI bits) is required when not using a dedicated NSS pin. Without it, the SPI peripheral won't transmit in master mode.

**This is how real drivers work:** ST's HAL, CMSIS, and other vendor libraries use these same patterns—register structs, clean APIs, clear-then-set. Building my own taught me what those libraries do under the hood.

## Resources
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — GPIO (Section 8), USART (Section 30), SPI (Section 28)
- [STM32F407G-DISC1 User Manual](https://www.st.com/resource/en/user_manual/um1472-discovery-kit-with-stm32f407vg-mcu-stmicroelectronics.pdf)