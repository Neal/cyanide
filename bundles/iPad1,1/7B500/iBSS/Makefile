CROSS ?= arm-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
SRC = ../../../..
BIN2C = $(SRC)/tools/bin2c
LOADADDR = 0x42000000
TYPE = iBSS
MODEL = k48ap
OBJCOPY = $(CROSS)objcopy
OBJECTS = entry.o main.o common.o commands.o task.o lock.o aes.o bdev.o image.o nvram.o filesystem.o kernel.o memory.o patch.o functions.o framebuffer.o breakpoint.o uart.o radio.o
CFLAGS = -I. -I./.. -I./../.. -I./$(SRC)/include -nostdlib -mlittle-endian
LDFLAGS = -Ttext=$(LOADADDR) -nostdlib -lc -lm -lgcc

%.o: $(SRC)/%.S
	$(CC) -c $(<) -o $(@) $(INCLUDES) $(CFLAGS) 

%.o: $(SRC)/%.c
	$(CC) -c $(<) -o $(@) $(INCLUDES) $(CFLAGS) 

all: payload

payload: payload.elf
	$(BIN2C) $(SRC)/payloads/$(MODEL).$(TYPE).payload $(SRC)/payloads/$(TYPE).$(MODEL).h $(TYPE)_$(MODEL)

payload.elf: $(OBJECTS)
	$(CC) -o $(@) $(OBJECTS) $(LDFLAGS)
	$(OBJCOPY) -O binary $(@) $(SRC)/payloads/$(MODEL).$(TYPE).payload
	rm -rf *.o *.elf

clean:
	rm -rf *.o *.elf

