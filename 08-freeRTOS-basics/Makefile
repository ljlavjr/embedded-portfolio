# STM32F407VG on Discovery board
# MCU: cpu core inside the board. ARM uses the core type. AVR uses exact chip
MCU = cortex-m4
# -mcpu=$(MCU): 	Tells compiler which CPU instruction set to use
# -mthumb: 			Use Thumb instruction set (16-bit compressed instructions, standard for Cortex-M)
# -mfloat-abi=soft:	Use software floating point. The F407 has a hardware FPU, but we you software for now 
MFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=soft

# Toolchain
# arm-none-eabi:	"ARM, no operating sytstem, embedded ABI(application binary interface)"
CC = arm-none-eabi-gcc			# C compiler
AS = arm-none-eabi-as			# Assembler
LD = arm-none-eabi-ld			# Linker
OBJCOPY = arm-none-eabi-objcopy	# Converts ELF to binary

# Flags
# -Wall -Os: 	Same as AVR, all warnings, optimize for size
# -nostdlib: 	Do not link standard C library. No printf, no malloc.
# -T linker.ld:	Use our custom linker script (tells linker memory layout)
CFLAGS = -Wall -Os $(MFLAGS) -nostdlib -I include -I include/hal -I freertos/include -I freertos/portable/GCC/ARM_CM4F -I src
LDFLAGS = -T linker.ld -nostdlib

# Files
all: main.bin

# Assemble the startup code. Runs before main()
startup.o: startup.s 
	$(AS) $(MFLAGS) startup.s -o startup.o

gpio.o: src/hal/gpio.c include/hal/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c src/hal/gpio.c -o gpio.o

uart.o: src/hal/uart.c include/hal/uart.h
	$(CC) $(CFLAGS) -c src/hal/uart.c -o uart.o

main.o: src/main.c include/hal/gpio.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

tasks.o: freertos/tasks.c
	$(CC) $(CFLAGS) -c freertos/tasks.c -o tasks.o

queue.o: freertos/queue.c
	$(CC) $(CFLAGS) -c freertos/queue.c -o queue.o

list.o: freertos/list.c
	$(CC) $(CFLAGS) -c freertos/list.c -o list.o

port.o: freertos/portable/GCC/ARM_CM4F/port.c
	$(CC) $(CFLAGS) -c freertos/portable/GCC/ARM_CM4F/port.c -o port.o

heap_4.o: freertos/portable/MemMang/heap_4.c
	$(CC) $(CFLAGS) -c freertos/portable/MemMang/heap_4.c -o heap_4.o

# Link everything together using our linker script
main.elf: startup.o main.o gpio.o uart.o tasks.o queue.o list.o port.o head_4.o
	$(CC) $(MFLAGS) $(LDFLAGS) startup.o main.o gpio.o uart.o tasks.o queue.o list.o port.o head_4.o -o main.elf 

# Convert ELF to raw binary. STM32 flash tools want .bin, not .hex like AVR
main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o main.elf main.bin