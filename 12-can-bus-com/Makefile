# STM32F407VG on Discovery board
MCU = cortex-m4
MFLAGS = -mcpu=$(MCU) -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -Wall -Os $(MFLAGS) -nostartfiles --specs=nano.specs -I include -I include/hal -I src
LDFLAGS = -T linker.ld -nostartfiles --specs=nano.specs

all: main.bin

startup.o: startup.s
	$(AS) $(MFLAGS) startup.s -o startup.o

gpio.o: drivers/gpio.c include/hal/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/gpio.c -o gpio.o

uart.o: drivers/uart.c include/hal/uart.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/uart.c -o uart.o

can.o: drivers/can.c include/hal/can.h include/hal/gpio.h include/stm32f407.h
	$(CC) $(CFLAGS) -c drivers/can.c -o can.o

main.o: src/main.c include/hal/gpio.h include/hal/uart.h include/hal/can.h include/stm32f407.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

main.elf: startup.o main.o gpio.o uart.o can.o
	$(CC) $(MFLAGS) $(LDFLAGS) startup.o main.o gpio.o uart.o can.o -lc -lnosys -lgcc -o main.elf

main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

clean:
	rm -f *.o main.elf main.bin