# Which chip
MCU = atmega2560
# Clock speed in Hz
F_CPU = 16000000

# Compiler
CC = avr-gcc
# Flags:
# -Wall  enable all warnings
# -Os	optimize for size (important on a chips with small flash)
# -mmcu=$(MCU)	tells the compiler which chip so it knows available registers/features
# -DF_CPU=$(F_CPU)	defines F_CPU as a preprocessor macro (code needs this for timing)
CFLAGS = -Wall -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)

# Converts the complied ELF file to the HUX format the chip understands
OBJCOPY = avr-objcopy
# Both below for avrdude when you flash. Arduino uno uses the arduino protocol. Port is where the device is connected to.
PROGRAMMER = wiring
PORT = /dev/ttyACM0
BAUD = 115200

# BUILD TARGETS

all: main.hex

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

main.elf: main.o
	$(CC) $(CFLAGS) main.o -o main.elf

main.hex: main.elf
	$(OBJCOPY) -O ihex main.elf main.hex

# UTILITY TARGETS

flash: main.hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -D  -U flash:w:main.hex:i

clean:
	rm -f main.o main.elf main.hex
