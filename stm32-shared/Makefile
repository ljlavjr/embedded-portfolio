# ============================================
# STM32F407 Project Makefile Template
# Copy this into a new project and edit the
# EDIT lines below for your project's needs
# ============================================

# STM32F407VG on Discovery board
MCU = cortex-m4
MFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy

# Flags
CFLAGS = -Wall -Os $(MFLAGS) -nostartfiles --specs=nano.specs \
         -I include -I include/hal \
         -I freertos/include -I freertos/portable/GCC/ARM_CM4F \
         -I src
LDFLAGS = -T linker.ld -nostartfiles --specs=nano.specs

# ============================================
# EDIT: Add/remove drivers your project needs
# ============================================
DRIVERS = gpio.c uart.c adc.c
# Available: gpio.c uart.c adc.c spi.c iwdg.c timing.c

# ============================================
# EDIT: Add/remove FreeRTOS sources as needed
# Remove this section entirely for bare-metal
# ============================================
FREERTOS_CORE = tasks.c list.c
# Available: tasks.c list.c queue.c timers.c event_groups.c
FREERTOS_PORT = freertos/portable/GCC/ARM_CM4F/port.c
FREERTOS_HEAP = freertos/portable/MemMang/heap_4.c

# ============================================
# Source to object mapping (don't edit below)
# ============================================
DRIVER_OBJS = $(DRIVERS:.c=.o)
FREERTOS_CORE_OBJS = $(FREERTOS_CORE:.c=.o)
FREERTOS_PORT_OBJ = port.o
FREERTOS_HEAP_OBJ = heap_4.o

ALL_OBJS = startup.o main.o \
           $(DRIVER_OBJS) \
           $(FREERTOS_CORE_OBJS) \
           $(FREERTOS_PORT_OBJ) \
           $(FREERTOS_HEAP_OBJ)

# ============================================
# Build rules
# ============================================
all: main.bin

startup.o: startup.s
	$(AS) $(MFLAGS) startup.s -o startup.o

main.o: src/main.c
	$(CC) $(CFLAGS) -c src/main.c -o main.o

%.o: drivers/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: freertos/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(FREERTOS_PORT_OBJ): $(FREERTOS_PORT)
	$(CC) $(CFLAGS) -c $< -o $@

$(FREERTOS_HEAP_OBJ): $(FREERTOS_HEAP)
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: $(ALL_OBJS)
	$(CC) $(MFLAGS) $(LDFLAGS) $(ALL_OBJS) -lc -lnosys -lgcc -o main.elf

main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o main.elf main.bin