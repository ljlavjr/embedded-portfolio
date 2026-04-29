# Project 12: CAN Bus Communication

## Overview
This project implements a custom CAN driver on the STM32F407. The driver supports initialization at common bit rates, mask mode filtering, blocking transmit, and polled receive. It is verified using the peripheral's internal loopback mode, where transmitted frames are routed back to the receive FIFO without any external hardware. This proves the register layout, init sequence, filter logic, and frame packing are all correct, and sets the foundation for two node bus communication, interrupt driven receive, and FreeRTOS integration in future milestones.

## Hardware
- **Board**: STM32F407G-DISC1
- **Chip**: STM32F407VGT6 (ARM Cortex-M4F)
- **CAN Transceiver**: SN65HVD230 (not used in loopback test, ready for next milestone)
- **CAN Pins**: PB8 (CAN1_RX), PB9 (CAN1_TX), AF9
- **UART2**: Connected to Raspberry Pi 4 for debug output
- **LED**: PD12 (Discovery green LED) for visual heartbeat
- **Clock**: 16 MHz HSI (no PLL)

## Concepts Learned
- CAN protocol fundamentals: differential signaling, dominant vs recessive, non destructive arbitration
- Message based addressing: every node sees every frame, filters decide what to keep
- bxCAN architecture: 3 transmit mailboxes, 2 receive FIFOs, 28 shared filter banks
- Bit timing: time quanta, propagation segment, phase segments, sample point
- Filter configuration in 32 bit mask mode
- Loopback and silent modes for verifying driver logic without physical layer
- Building peripheral register definitions from a reference manual
- Static layout verification with `_Static_assert` and `offsetof`

## File Structure
12-can-bus-com/
├── src/
│   └── main.c
├── drivers/
│   ├── gpio.c
│   ├── uart.c
│   └── can.c
├── include/
│   ├── stm32f407.h
│   └── hal/
│       ├── gpio.h
│       ├── uart.h
│       └── can.h
├── startup.s
├── linker.ld
└── Makefile

## CAN Protocol Primer

### Differential Signaling
CAN uses two wires (CAN_H and CAN_L) with differential signaling to reject common mode noise.
Recessive (logic 1):  CAN_H ≈ 2.5V, CAN_L ≈ 2.5V    (differential ≈ 0V)
Dominant  (logic 0):  CAN_H ≈ 3.5V, CAN_L ≈ 1.5V    (differential ≈ 2V)

The bus needs 120 ohm termination resistors at both physical ends to prevent reflections.

### Non Destructive Arbitration
When two nodes try to ransmit at the same time, CAN resolves the conflict bit by bit during the ID field instead of treating it as a collision. Both nodes start transmitting simultaneously and watch the bus while they do. CAN is a wired AND bus where dominant bits (logic 0) override recessive bits (logic 1), so as soon as a node transmits a recessive bit but sees a dominant bit on the bus, it knows another node with a lower (higher priority) ID is talking and quietly stops. The winning node never knows there was a conflict and continues transmitting without retry or corruption. This is what makes CAN deterministic. The highest priority message always wins, and you can guarentee how long a critical message will take to get through regardless of bus load. 

### Message Based, Not Address Based
[Brief explanation. Each frame has an ID identifying the content, not the recipient. Every node sees every frame. Each node uses filters to decide what to process.]
Unlike Ethernet or SPI, CAN frames have no source or destination address. Each frame carries an ID that identifies what the message is, not who it is for. Every node on the bus sees every frame, and each node decides for itself which IDs it cares about its filter banks. This works like a publish subscribe system at the hardware level. In a car, an engine ECU might broadcast an RPM message with ID 0x100. The dashboard subscribes to that ID for the tachometer. The traction control subscribes to it for slip calculations. Neither subscriber had to be configured into the engine ECU. New consumers can be added later without touching the producer.

## bxCAN Architecture
                ┌─────────────────────┐
                │   3 TX Mailboxes    │
                │  (priority sorted)  │
                └──────────┬──────────┘
                           │
                           ▼
        ┌──────────────────────────────────┐
        │           CAN Bus                │
        └──────────────────────────────────┘
                           │
                           ▼
                ┌─────────────────────┐
                │   28 Filter Banks   │
                │  (mask or list)     │
                └──────────┬──────────┘
                           │
          ┌────────────────┴────────────────┐
          ▼                                 ▼
 ┌────────────────┐                ┌────────────────┐
 │   RX FIFO 0    │                │   RX FIFO 1    │
 │  (3 deep)      │                │  (3 deep)      │
 └────────────────┘                └────────────────┘

Hardware handles retransmission, ACK generation, CRC checking, and error frame production. The driver only deals with mailbox loading, filter configuration, and FIFO reading.

## Driver Design

### Frame Structure
```c
typedef struct {
    uint32_t id;        // 11 bit standard ID or 29 bit extended
    uint8_t  ide;       // 0 = standard, 1 = extended
    uint8_t  rtr;       // 0 = data frame, 1 = remote frame
    uint8_t  dlc;       // data length code, 0 to 8 bytes
    uint8_t  data[8];   // payload
} can_frame_t;
```

### Operating Modes
| Mode | Description | Use Case |
|------|-------------|----------|
| `CAN_MODE_NORMAL` | Production mode, full bus participation | Real bus communication |
| `CAN_MODE_LOOPBACK` | Internal TX to RX, also drives bus | Verify driver while seeing waveforms |
| `CAN_MODE_LOOPBACK_SILENT` | Internal TX to RX, no bus activity | First test, no hardware needed |
| `CAN_MODE_SILENT` | Receive only, no TX or ACK | Bus monitoring without disturbance |

### Supported Bit Rates
- 125 kbps
- 250 kbps
- 500 kbps (used in this test)
- 1 Mbps

### Filter Strategy
Filter bank 0 is configured in 32 bit mask mode with ID = 0 and mask = 0. A mask of zero means "do not care about any bits," so every received frame is accepted into FIFO 0. Future projects can add more selective filters in higher banks.

### Function Reference
| Function | Purpose |
|----------|---------|
| `can_init` | Enable clock, configure GPIO, set bit timing, transition to operating mode |
| `can_filter_accept_all` | Configure bank 0 to accept everything into FIFO 0 |
| `can_transmit` | Block until a mailbox is free, then send the frame |
| `can_receive_available` | Check if FIFO 0 has a pending frame |
| `can_receive` | Read oldest frame from FIFO 0 and release the slot |

## Bit Timing Configuration

CAN bit timing divides each bit into time quanta with a sync segment, propagation/phase 1, and phase 2:
| SYNC |    TSEG1    |   TSEG2   |
|  1tq |   tseg1 tq  |  tseg2 tq |
^
|
Sample point

The formula:
Bit rate     = PCLK1 / (BRP × (1 + TSEG1 + TSEG2))
Sample point = (1 + TSEG1) / (1 + TSEG1 + TSEG2)

Target sample point is around 87.5% for automotive use. The driver currently uses placeholder timing values that worked empirically at 500 kbps. Replacing these with properly calculated values is a future improvement (see Future Work).

## Register Definitions

Peripheral registers were defined manually in `stm32f407.h` rather than using the vendor HAL. The CAN structure uses nested typedefs for repeating units (TX mailboxes, RX FIFOs, filter banks) and reserved padding to match the real memory layout. The layout was verified against RM0090 chapter 32 by comparing register offsets.

Key offsets:
| Register Group | Offset |
|----------------|--------|
| Control / Status (MCR through BTR) | 0x000 |
| TX Mailboxes (3) | 0x180 |
| RX FIFOs (2) | 0x1B0 |
| Filter Control (FMR through FA1R) | 0x200 |
| Filter Banks (28) | 0x240 |

## Loopback Test

In loopback silent mode the peripheral routes transmitted frames internally to its own receive FIFO without driving the TX pin. This isolates the driver from any physical layer issues during initial development.

### Test Sequence
1. Initialize CAN1 in loopback silent mode at 500 kbps
2. Configure accept all filter
3. Loop:
   - Transmit a frame with ID 0x123 and 4 bytes of payload (DE AD BE EF)
   - Wait briefly for the loopback path
   - Read any pending frame and print it over UART

### Output
Booting CAN loopback test
CAN ready
RX id=0x0123 dlc=04 data=DE AD BE EF
RX id=0x0123 dlc=04 data=DE AD BE EF
RX id=0x0123 dlc=04 data=DE AD BE EF

The green LED on PD12 toggles each time a frame is received, providing a visual heartbeat.

This output verifies:
1. Register definitions match the silicon layout
2. Clock enable, GPIO alternate function, and bit timing are correct
3. Init mode entry and exit sequence works
4. Filter configuration accepts incoming frames
5. Transmit correctly packs ID, DLC, and data into the mailbox
6. Receive correctly unpacks the frame from the FIFO
7. FIFO release (RFOM0) is working since fresh frames keep arriving

## What I Learned

**Register definitions are the real work**: I thought writing the CAN driver would be the hard part. The struct definitions and the offset math turned out to be where I spent the most time, and where the most could go wrong. Static assertions on key offsets caught a couple of padding mistakes at compile time that would have been awful to debug at runtime.

**Loopback mode is a gift**: The peripheral can test itself with no transceiver, no bus, and no second node. If something goes wrong in loopback, the bug is in my code. That made debugging way more focused than if I had been chasing wiring or termination problems at the same time.

**Build systems eat time**: A solid chunk of this project was fixing include paths, removing FreeRTOS leftovers from the timing module, and chasing Makefile rules that pointed at old folder structures. None of that is CAN. All of it had to get fixed before anything would compile. I should have spent more time setting the project up cleanly at the start instead of copying from old projects and patching.

**Non destructive arbitration is clever**: Lower ID wins, the loser backs off, no retries needed. I had to read it three times before it clicked. Designing this with 1986 hardware constraints is impressive.

**Doing it from scratch is slower but you actually learn**: Calling a vendor HAL function would have gotten me here in an afternoon. Writing my own took a week. But now I can explain every line of the init sequence and why it is in that order. That feels like the difference between knowing the API and knowing the peripheral.

## Future Work
- Calculate proper bit timing values for all four bit rates targeting 87.5% sample point
- Add SN65HVD230 transceiver and verify normal mode signaling on the wire
- Two node bus with a second STM32 board, 120 ohm termination at both ends
- Interrupt driven receive using FIFO 0 message pending IRQ
- FreeRTOS integration: TX queue, transmit task, RX task driven by ISR posted queue
- Custom KiCad ECU board (Project 12b) with onboard transceiver and outputs

## Build & Flash
```bash
make clean && make
st-flash write main.bin 0x08000000
```

## Resources
- [STM32F407 Reference Manual (RM0090) Chapter 32 bxCAN](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [SN65HVD230 Datasheet](https://www.ti.com/product/SN65HVD230)
- [CSS Electronics CAN Bus Explained](https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial)
- ISO 11898-1 (CAN standard, paywalled but referenced widely)