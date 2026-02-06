# Custom HAL for STM32F407

## Overview
This project builds a reusable Hardware Abstraction Layer (HAL) for GPIO on the STM32F407. Instead of manipulating raw register addresses throughout application code, we create clean functions like `gpio_init()` and `write_pin()` that hide the hardware details. This makes code more readable, portable, and less error-prone.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4)
- **LED Used**: PD14 (Red)

## Concepts Learned
- Hardware abstraction layers and why they matter
- Register structs for memory-mapped I/O
- API design for embedded drivers
- Clear-then-set pattern for multi-bit register fields
- Atomic register access using BSRR

## File Structure
```
06-custom-hal/
├── src/
│   ├── main.c           # Application code using HAL
│   ├── startup.s        # Startup code from Project 05
│   └── hal/
│       └── gpio.c       # GPIO driver implementation
├── include/
│   ├── stm32f407.h      # Register definitions
│   └── hal/
│       └── gpio.h       # GPIO API
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

### rcc_gpio_clock_enable()
Enables the peripheral clock for a GPIO port. Must be called before using any GPIO functions.
```c
void rcc_gpio_clock_enable(GPIO_TypeDef *port);
```

**Example:**
```c
rcc_gpio_clock_enable(GPIOD);
```

### gpio_init()
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

### gpio_set_otype()
Sets output type for a pin.
```c
void gpio_set_otype(GPIO_TypeDef *port, uint8_t pin, GPIO_OUTPUT_TYPE otype);
```

**Types:**
- `GPIO_OUTTYPE_PUSHPULL` — Default, drives high and low
- `GPIO_OUTTYPE_OPENDRAIN` — Only drives low, needs external pull-up

### gpio_set_speed()
Sets output speed (slew rate) for a pin.
```c
void gpio_set_speed(GPIO_TypeDef *port, uint8_t pin, GPIO_SPEED speed);
```

**Speeds:** `GPIO_SPEED_LOW`, `GPIO_SPEED_MEDIUM`, `GPIO_SPEED_HIGH`, `GPIO_SPEED_VHIGH`

### gpio_set_pull()
Configures internal pull-up/pull-down resistors.
```c
void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GPIO_PULL_UD pull);
```

**Options:** `GPIO_PULL_UD_NOPUPD`, `GPIO_PULL_UD_PULLU`, `GPIO_PULL_UD_PULLD`

### write_pin()
Sets a pin high or low.
```c
void write_pin(GPIO_TypeDef *port, uint8_t pin, uint32_t value);
```

**Example:**
```c
write_pin(GPIOD, 14, HIGH);
write_pin(GPIOD, 14, LOW);
```

### read_pin()
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

### toggle_pin()
Inverts the current state of an output pin.
```c
void toggle_pin(GPIO_TypeDef *port, uint8_t pin);
```

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## Design Decisions

### Explicit Clock Enable (Option B)
We require the user to call `rcc_gpio_clock_enable()` before using a port, rather than enabling clocks automatically inside `gpio_init()`. This makes the dependency explicit in the code and matches how other peripherals (UART, SPI, timers) work. The pattern "enable clock, then init" becomes muscle memory.

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

## Key Patterns

### Clear-Then-Set for Multi-Bit Fields
MODER uses 2 bits per pin. To change a value safely:
```c
port->MODER &= ~(0x3 << (pin * 2));   // Clear both bits
port->MODER |= (mode << (pin * 2));   // Set new value
```

Just OR-ing would fail if bits were already set. This pattern applies to OSPEEDR, PUPDR, AFR, and many other registers.

### Bit Position Calculation
For 2-bit fields: `pin * 2`
For 4-bit fields: `pin * 4`

### Extracting a Single Bit
Shift right to bring the bit to position 0, then mask:
```c
return (port->IDR >> pin) & 0x1;
```

## What I Learned

**Why abstraction matters:** Raw register access works, but it's error-prone and hard to read. A simple function like `gpio_init(GPIOD, 14, GPIO_MODE_OUTPUT)` is self-documenting. Six months from now, I'll understand what that line does without checking the reference manual.

**Register structs:** Mapping a struct to a peripheral's register block is elegant. The compiler handles offset math, and access looks clean: `GPIOD->MODER` instead of `*(volatile uint32_t *)0x40020C00`.

**Design tradeoffs:** There's no single "correct" API design. The choice depends on what you value—simplicity, explicitness, or safety. I chose explicit control.

**This is how real drivers work:** ST's HAL, CMSIS, and other vendor libraries use these same patterns—register structs, clean APIs, clear-then-set. Building my own taught me what those libraries do under the hood.

## Resources
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — GPIO registers in Section 8
- [STM32F407G-DISC1 User Manual](https://www.st.com/resource/en/user_manual/um1472-discovery-kit-with-stm32f407vg-mcu-stmicroelectronics.pdf)