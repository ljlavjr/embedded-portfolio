# STM32F407VG on Discovery board
# MCU: cpu core inside the board. ARM uses the core type. AVR uses exact chip
MCU = cortex-m4
# -mcpu=$(MCU): 		Tells compiler which CPU instruction set to use
# -mthumb: 				Use Thumb instruction set (16-bit compressed instructions, standard for Cortex-M)
# -mfloat-abi=hard:		Use hardware floating point.
# -mfpu=fpv4-sp-d16:	FPv4 floating point, single precision only, 16 double-word registers
MFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Toolchain
# arm-none-eabi:	"ARM, no operating sytstem, embedded ABI(application binary interface)"
CC = arm-none-eabi-gcc			# C compiler
AS = arm-none-eabi-as			# Assembler
LD = arm-none-eabi-ld			# Linker
OBJCOPY = arm-none-eabi-objcopy	# Converts ELF to binary

# Flags
# -Wall -Os: 						Same as AVR, all warnings, optimize for size
# -nostartfiles --specs=nano.specs:	Skips default setup. Gives you a tiny embedded-friendly version of libs
# -I ...:							Include paths
# -T linker.ld:						Use our custom linker script (tells linker memory layout)
CFLAGS = -Wall -Os $(MFLAGS) -nostartfiles --specs=nano.specs -I include -I include/hal -I src
LDFLAGS = -T linker.ld -nostartfiles --specs=nano.specs

# Files
all: main.bin

# Assemble the startup code. Runs before main()
startup.o: startup.s 
	$(AS) $(MFLAGS) startup.s -o startup.o

gpio.o: drivers/gpio.c include/hal/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/gpio.c -o gpio.o

uart.o: drivers/uart.c include/hal/uart.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/uart.c -o uart.o

timing.o: drivers/timing.c include/hal/timing.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/timing.c -o timing.o

can.o: drivers/can.c include/hal/can.h include/hal/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/can.c -o can.o

main.o: src/main.c include/hal/gpio.h include/hal/uart.h include/hal/can.h include/hal/timing.h include/stm32f407.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

# Link everything together using our linker script
# -lnosys:	provides stubs for OS calls you do not have
main.elf: startup.o main.o gpio.o uart.o timing.o can.o
	$(CC) $(MFLAGS) $(LDFLAGS) startup.o main.o gpio.o uart.o timing.o can.o -lc -lnosys -lgcc -o main.elf

# Convert ELF to raw binary. STM32 flash tools want .bin, not .hex like AVR
main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o main.elf main.bin