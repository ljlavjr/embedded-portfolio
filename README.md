# Embedded Systems Portfolio

## About Me
My name is Lawrence (Larry) Lavigne. I am a Computer Engineering graduate from the University of Tennessee, Knoxville and I currently work as a Technology Intern in an IT department. I am an aspiring embedded engineer and I am working towards building up the fundamentals I have learned while I was attending UT as well as expanding my knowledge of industry related embedded projects. This portfolio demonstrates my progress toward an entry-level embedded engineering role. 

## What This Is
This is a collection of projects that build from the "Hello World" of embedded programming, blinking an LED, all the way to embedded Linux and Network Security. As each new project is added, it builds on what the previous projects have taught me.

## Projects

### Phase 1: AVR Bare-Metal
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 01 | Bare-Metal Blink | ATmega2560 | Registers, Makefiles, Toolchain | Complete |
| 02 | UART Driver | ATmega2560 | Polling, Serial, Driver Architecture | Complete |
| 03 | Interrupt UART | ATmega2560 | ISR, Ring Buffer, Volatile | Complete |
| 04 | Multi-Sensor Polling | ATmega2560 | ADC, Super-loop Pain | Complete |

### Phase 2: STM32 Bare-Metal
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 05 | Bare-Metal Startup | STM32F407 | ARM Startup, Linker Scripts, Vector Table | Planned |
| 06 | Custom HAL | STM32F407 | GPIO, UART, SPI Drivers, Abstraction | Planned |
| 07 | DMA Data Acquisition | STM32F407 | DMA, ADC, Double Buffering | Planned |

### Phase 3: RTOS
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 08 | FreeRTOS Basics | STM32F407 | Tasks, Priorities, Scheduler | Planned |
| 09 | Inter-Task Communication | STM32F407 | Queues, Producer-Consumer | Planned |
| 10 | Resource Sharing | STM32F407 | Mutexes, Semaphores, Priority Inversion | Planned |
| 11 | Real-Time Sensor System | STM32F407 | Multi-task Application, Timing Analysis | Planned |

### Phase 4: Automotive Protocols
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 12 | CAN Bus Communication | STM32F407 | CAN Protocol, Message Filtering, Multi-node | Planned |
| 13 | Modbus RTU | STM32F407 | Industrial Protocol, Master/Slave, CRC | Planned |
| 14 | OBD-II Reader | STM32F407 | Automotive Diagnostics, CAN, PID Parsing | Planned |

### Phase 5: IoT & Wireless
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 15 | MQTT Sensor Node | ESP32 | WiFi, MQTT, JSON, Sleep Modes | Planned |
| 16 | Motion Detection Camera | ESP32-CAM | Camera Interface, Image Processing, Alerts | Planned |
| 17 | BLE + WiFi Gateway | Arduino Uno R4 WiFi | Multi-protocol, Bridging, FreeRTOS | Planned |
| 18 | OTA Update System | Arduino Uno R4 WiFi | Bootloaders, Flash Management, Verification | Planned |

### Phase 6: Embedded Linux
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 19 | PRU Real-Time Control | BeagleBone Black | PRU Assembly, Shared Memory, Hard Real-time | Planned |
| 20 | Linux Kernel Driver | BeagleBone Black | Kernel Modules, Character Devices, sysfs | Planned |
| 21 | Device Tree Overlays | BeagleBone Black | Device Tree, Hardware Description | Planned |
| 22 | Custom Yocto Image | Raspberry Pi Zero 2W | Build Systems, Cross-compilation | Planned |

### Phase 7: Networking & Security
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| 23 | Secure Bootloader | STM32F407 | Cryptographic Verification, Secure Boot | Planned |
| 24 | Encrypted MQTT | ESP32 | TLS, Certificates, Secure Communication | Planned |
| 25 | Network Packet Analyzer | BeagleBone Black | Raw Sockets, Protocol Parsing, Filtering | Planned |
| 26 | Embedded Firewall | Raspberry Pi Zero 2W | iptables, Traffic Rules, Logging | Planned |

### Infrastructure
| # | Project | Platform | Skills | Status |
|---|---------|----------|--------|--------|
| -- | CI/CD Pipeline | Raspberry Pi 4 | GitHub Actions, Self-hosted Runner | Complete |
| -- | IoT Backend | Raspberry Pi 4 | MQTT, InfluxDB, Grafana, Telegraf | Complete |

## Learning Path
I'm taking a bottom-up approach. Starting with bare-metal AVR and ARM programming to build hardware fundamentals (registers, GPIOs, data transfer), then moving to FreeRTOS since RTOS experience appears in nearly every embedded job posting. From there I branch into automotive protocols (CAN, Modbus) since I have some experience in that field, then IoT, embedded Linux, and eventually networking and security. I also set up a CI/CD pipeline from GitHub Actions to my Raspberry Pi server for build visibility after each push.

## Hardware
- Arduino Mega 2560 (Elegoo)
- STM32F407G-DISC1
- Raspberry Pi 4 (CI/CD server, IoT backend)
- others added as we progress through the list

## Infrastructure
- Self-hosted GitHub Actions runner on Pi 4
- MQTT broker + InfluxDB + Grafana for IoT projects

## Resources
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0090-stm32f405415-stm32f407417-stm32f427437-and-stm32f429439-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F407G-DISC1 User Manual](https://www.st.com/resource/en/user_manual/um1472-discovery-kit-with-stm32f407vg-mcu-stmicroelectronics.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)