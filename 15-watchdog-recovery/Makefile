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
CFLAGS = -Wall -Os $(MFLAGS) -nostartfiles --specs=nano.specs -I include -I include/hal -I freertos/include -I freertos/portable/GCC/ARM_CM4F -I src
LDFLAGS = -T linker.ld -nostartfiles --specs=nano.specs

# Files
all: main.bin

# Assemble the startup code. Runs before main()
startup.o: startup.s 
	$(AS) $(MFLAGS) startup.s -o startup.o

gpio.o: drivers/gpio.c include/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/gpio.c -o gpio.o

uart.o: drivers/uart.c include/uart.h
	$(CC) $(CFLAGS) -c drivers/uart.c -o uart.o

adc.o: drivers/adc.c include/adc.h
	$(CC) $(CFLAGS) -c drivers/adc.c -o adc.o

iwdg.o: drivers/iwdg.c include/iwdg.h
	$(CC) $(CFLAGS) -c drivers/iwdg.c -o iwdg.o

main.o: src/main.c include/gpio.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

tasks.o: freertos/tasks.c
	$(CC) $(CFLAGS) -c freertos/tasks.c -o tasks.o

list.o: freertos/list.c 
	$(CC) $(CFLAGS) -c freertos/list.c -o list.o

event_groups.o: freertos/event_groups.c 
	$(CC) $(CFLAGS) -c freertos/event_groups.c -o event_groups.o

port.o: freertos/portable/GCC/ARM_CM4F/port.c
	$(CC) $(CFLAGS) -c freertos/portable/GCC/ARM_CM4F/port.c -o port.o

heap_4.o: freertos/portable/MemMang/heap_4.c
	$(CC) $(CFLAGS) -c freertos/portable/MemMang/heap_4.c -o heap_4.o

timers.o: freertos/timers.c
	$(CC) $(CFLAGS) -c freertos/timers.c -o timers.o

# Link everything together using our linker script
# -lnosys:	provides stubs for OS calls you do not have
main.elf: startup.o main.o gpio.o uart.o tasks.o port.o heap_4.o adc.o iwdg.o list.o event_groups.o timers.o
	$(CC) $(MFLAGS) $(LDFLAGS) startup.o main.o gpio.o uart.o adc.o iwdg.o list.o tasks.o port.o heap_4.o event_groups.o timers.o -lc -lnosys -lgcc -o main.elf 

# Convert ELF to raw binary. STM32 flash tools want .bin, not .hex like AVR
main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o main.elf main.bin