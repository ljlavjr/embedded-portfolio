# Project 07: DMA Data Acquisition

## Overview
This project implements Direct Memory Access (DMA) to transfer ADC samples to memory without CPU involvement. Instead of the CPU polling the ADC and manually copying each value, the DMA controller handles transfers automatically. This frees the CPU for other tasks and enables high-speed data acquisition.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4)
- **ADC Input**: PA0 (ADC1 Channel 0)
- **Debug Output**: USART2 via USB-to-TTL adapter

## Concepts Learned
- Direct Memory Access (DMA) and why it matters
- Peripheral-to-memory transfers
- Double buffering for continuous data acquisition
- ADC continuous conversion mode
- Connecting peripherals to DMA channels/streams
- DMA configuration: circular mode, memory increment, data sizes

## File Structure
```
07-dma-data-acquisition/
├── src/
│   ├── main.c
│   ├── startup.s
│   └── hal/
│       ├── gpio.c
│       ├── uart.c
│       ├── adc.c
│       └── dma.c
├── include/
│   ├── stm32f407.h
│   └── hal/
│       ├── gpio.h
│       ├── uart.h
│       ├── adc.h
│       └── dma.h
├── linker.ld
└── Makefile
```

## Why DMA?

### The Polling Problem
With polling, the CPU waits for each ADC conversion:
```c
while (1) {
    start_conversion();
    while (!conversion_done());  // CPU stuck here
    value = read_adc();
    buffer[i++] = value;
}
```

At high sample rates, the CPU spends most of its time waiting.

### The DMA Solution
DMA transfers data without CPU involvement:
```
ADC completes conversion
       ↓
DMA detects data ready
       ↓
DMA copies ADC->DR to buffer[i]
       ↓
DMA increments pointer
       ↓
Repeat (CPU is free the entire time)
```

The CPU configures DMA once, then does other work. DMA handles the transfers automatically.

## Double Buffering

DMA is configured with two buffers:
- While DMA fills Buffer A, CPU processes Buffer B
- When Buffer A is full, DMA switches to Buffer B
- CPU then processes Buffer A
- No data is lost, no gaps in acquisition
```
Time →
DMA:  [Fill A][Fill B][Fill A][Fill B]...
CPU:  [      ][Proc A][Proc B][Proc A]...
```

## Register Structures

### ADC Registers
```c
typedef struct {
    volatile uint32_t SR;       // Status
    volatile uint32_t CR1;      // Control 1
    volatile uint32_t CR2;      // Control 2
    volatile uint32_t SMPR1;    // Sample time (ch 10-18)
    volatile uint32_t SMPR2;    // Sample time (ch 0-9)
    // ... other registers
    volatile uint32_t DR;       // Data register
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *) 0x40012000)
```

### DMA Registers
```c
typedef struct {
    volatile uint32_t CR;       // Control
    volatile uint32_t NDTR;     // Number of data to transfer
    volatile uint32_t PAR;      // Peripheral address
    volatile uint32_t M0AR;     // Memory 0 address
    volatile uint32_t M1AR;     // Memory 1 address
    volatile uint32_t FCR;      // FIFO control
} DMA_Stream_TypeDef;

#define DMA2_Stream0 ((DMA_Stream_TypeDef *) 0x40026410)
```

## API Reference

### ADC Functions

#### adc_init()
Initializes ADC1 for polling mode.
```c
void adc_init(void);
```

#### adc_read()
Performs a single ADC conversion and returns the result. Blocks until conversion completes.
```c
uint16_t adc_read(uint8_t channel);
```

**Example:**
```c
uint16_t value = adc_read(0);  // Read channel 0 (PA0)
```

#### adc_init_dma()
Initializes ADC1 for continuous DMA mode.
```c
void adc_init_dma(uint8_t channel);
```

**Configures:**
- Continuous conversion mode
- DMA request generation
- DDS bit for continuous DMA requests

#### adc_start()
Starts ADC conversions. Call after DMA is configured and started.
```c
void adc_start(void);
```

---

### DMA Functions

#### dma_init()
Configures DMA2 Stream 0 for ADC1 transfers with double buffering.
```c
void dma_init(uint16_t *buffer0, uint16_t *buffer1, uint16_t size);
```

**Parameters:**
- `buffer0` — First buffer address
- `buffer1` — Second buffer address  
- `size` — Number of samples per buffer

**Configures:**
- Channel 0 (ADC1)
- 16-bit transfers
- Memory increment
- Circular mode
- Double buffer mode
- Peripheral-to-memory direction

#### dma_start()
Enables the DMA stream. Call after dma_init().
```c
void dma_start(void);
```

## Usage Example
```c
#define BUFFER_SIZE 16

uint16_t buffer0[BUFFER_SIZE];
uint16_t buffer1[BUFFER_SIZE];

int main(void) {
    // Configure PA0 as analog input
    rcc_gpio_clock_enable(GPIOA);
    gpio_init(GPIOA, 0, GPIO_MODE_AN);

    // Initialize DMA first
    dma_init(buffer0, buffer1, BUFFER_SIZE);

    // Initialize ADC for DMA mode
    adc_init_dma(0);

    // Start DMA, then ADC
    dma_start();
    adc_start();

    // Buffers now fill automatically
    while (1) {
        // Process buffer data
    }
}
```

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## DMA Configuration Details

### Stream Selection
ADC1 can use DMA2 Stream 0 or Stream 4, both on Channel 0. We use Stream 0.

### CR Register Bits
| Bit(s) | Field | Value | Meaning |
|--------|-------|-------|---------|
| 27:25 | CHSEL | 000 | Channel 0 (ADC1) |
| 18 | DBM | 1 | Double buffer mode |
| 14:13 | MSIZE | 01 | 16-bit memory |
| 12:11 | PSIZE | 01 | 16-bit peripheral |
| 10 | MINC | 1 | Memory increment |
| 8 | CIRC | 1 | Circular mode |
| 7:6 | DIR | 00 | Peripheral to memory |
| 0 | EN | 1 | Enable stream |

### ADC CR2 Bits for DMA
| Bit | Field | Purpose |
|-----|-------|---------|
| 1 | CONT | Continuous conversion mode |
| 8 | DMA | Enable DMA requests |
| 9 | DDS | Continue DMA requests after each conversion |
| 30 | SWSTART | Start conversion |

## Key Patterns

### Initialize DMA Before Peripheral
Configure DMA first, then the peripheral that triggers it:
```c
dma_init(...);
adc_init_dma(0);
dma_start();
adc_start();
```

### Disable Stream Before Configuring
```c
DMA2_Stream0->CR &= ~(1 << 0);           // Disable
while (DMA2_Stream0->CR & (1 << 0));     // Wait until disabled
// Now configure...
```

### Cast Addresses for DMA Registers
```c
DMA2_Stream0->PAR = (uint32_t)&ADC1->DR;
DMA2_Stream0->M0AR = (uint32_t)buffer0;
```

## What I Learned

**DMA frees the CPU:** With polling, the CPU is stuck waiting for each conversion. With DMA, the CPU configures the transfer once and is then free to do other work. This is essential for high-speed data acquisition.

**Configuration order matters:** DMA must be configured and enabled before starting the ADC. Otherwise, DMA requests are missed.

**Double buffering prevents data loss:** While DMA fills one buffer, the CPU can process the other. No gaps, no overwriting data being processed.

**STM32 DMA is channel-based:** Each peripheral maps to specific DMA controllers, streams, and channels. ADC1 uses DMA2 Stream 0 Channel 0—this is fixed in hardware.

**Circular mode for continuous acquisition:** Without circular mode, DMA stops after filling the buffer once. Circular mode wraps back to the start automatically.

## Future Improvements

- Add DMA transfer complete interrupt to signal when buffers are ready
- Implement proper buffer switching with flags
- Add averaging or filtering of acquired samples
- Support multiple ADC channels with scan mode

## Resources
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — ADC (Section 13), DMA (Section 10)
- [STM32F407G-DISC1 User Manual](https://www.st.com/resource/en/user_manual/um1472-discovery-kit-with-stm32f407vg-mcu-stmicroelectronics.pdf)