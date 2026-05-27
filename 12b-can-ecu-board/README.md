# Project 12b: Custom CAN ECU Board

## Purpose
A custom STM32 based ECU board that connects to a CAN bus and performs configurable actions (LED patterns, fan control, button input reporting) based on received messages. Designed to be a second node on the bus alongside my STM32F407 Discovery board for two node CAN demonstrations.

## Functional Requirements
1. Receive and transmit CAN messages at 125, 250, 500, and 1000 kbps
2. Drive 4 status LEDs with patterns controlled by CAN messages
3. Drive 1 PWM output at 5V via MOSFET, controlled by CAN messages
4. Read 2 button inputs and report state changes over CAN
5. Read 1 analog input (potentiometer) and report value periodically over CAN
6. Provide power and CAN activity indicator LEDs
7. Allow programming and debugging via 5 pin SMT SWD header
8. Allow UART debug via 4 pin SMT UART header
9. Operate from USB-C 5V power only

## Non Functional Requirements
1. 2 layer PCB
2. 100% SMT components, full assembly via JLCPCB
3. Maximum dimensions: 80mm x 60mm
4. Cost target: under $20 per board (5 board run)
5. All parts available in JLCPCB parts library where possible

## Out of Scope for v1
1. 12V automotive style input (USB-C only for now)
2. Automotive grade transient protection (TVS arrays, common mode chokes)
3. CAN FD support
4. Galvanic isolation
5. Full size USB to UART bridge (use external FTDI cable)

## Bill of Materials

| # | Part | Manufacturer | JLC Part | Library | Approx Cost | Package | Notes |
|---|------|--------------|----------|---------|-------------|---------|-------|
| 1 | STM32F407VGT6 | ST | C18584 | Extended | $9.00 | LQFP-100 | Same as Discovery |
| 2 | AMS1117-3.3 | EVVO | C6186 | Basic | $0.10 | SOT-223-3L | 3.3V LDO |
| 3 | SN65HVD230D | TI | C12084 | Extended | $1.50 | SOIC-8 | CAN transceiver |
| 4 | USB4135-GF-A | GCT | C2765186 | Extended | $0.78 | SMT horizontal | USB-C power only |
| 5 | LMT86DCKR | TI | (extended) | Extended | $1.00 | SC70-5 | Analog temp sensor |
| 6 | 8MHz 20pF crystal | YXC | C115962 | Extended | $0.20 | SMD-3225 | HSE for STM32 |
| 7 | 20pF 0603 C0G | UNIOHM | C1804 | Basic | $0.02 | 0603 | 2x for crystal load caps |
| 8 | SI2302 | various | C10487 | Basic | $0.04 | SOT-23 | N-channel logic level MOSFET |
| 9 | 1N5819 Schottky | various | C8598 | Basic | $0.02 | SOD-123 | Fan flyback diode |
| 10 | TS-1187A SMT button | various | C318884 | Basic | $0.10 | SMT 6mm | 3x: 2 user + 1 reset |
| 11 | LED Red 0603 | KENTO | C2286 | Basic | $0.05 | 0603 | 2x: power, CAN active |
| 12 | LED Green 0603 | KENTO | C72043 | Basic | $0.05 | 0603 | 2x output |
| 13 | LED Blue 0603 | KENTO | C72041 | Basic | $0.05 | 0603 | 1x output |
| 14 | LED Yellow 0603 | KENTO | C72038 | Basic | $0.05 | 0603 | 1x output |
| 15 | 470Ω 0603 1% | UNIOHM | C23179 | Basic | $0.01 | 0603 | 6x for LEDs |
| 16 | 5.1kΩ 0603 1% | UNIOHM | C23186 | Basic | $0.01 | 0603 | 2x USB-C CC pins |
| 17 | 10kΩ 0603 1% | UNIOHM | C25804 | Basic | $0.01 | 0603 | MOSFET pull down + boot pin + reset |
| 18 | 100Ω 0603 1% | UNIOHM | C22775 | Basic | $0.01 | 0603 | MOSFET gate series |
| 19 | 0.1µF 0603 X7R | various | C14663 | Basic | $0.01 | 0603 | ~12x decoupling |
| 20 | 10µF 0805 X5R | various | C15850 | Basic | $0.03 | 0805 | 2x bulk caps |
| 21 | 0.1 inch male header | various | C124378 | Extended | $0.20 | THT | 4x: SWD, UART, CAN, fan |

## Note
The board will be assembled by JLCPCB since I do not own a soldering station :(