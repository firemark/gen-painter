MCU = atmega328p
F_CPU = 8000000ul

DUDE_PROGRAMMER = usbasp
DUDE_PORT = usb

TARGET=main

AVR_OBJS=epd.avr.o image.avr.o art.avr.o $(TARGET).avr.o
SDL_OBJS=sdl.sdl.o image.sdl.o art.sdl.o

OPTIMIZE = Os
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
OBJSIZE = avr-size

# Don't touch anything below that line. Ya, rly.
#-----------------------------------------------------------------------

CFLAGS  = -mmcu=$(MCU)
CFLAGS += -$(OPTIMIZE)
CFLAGS += -g
CFLAGS += -std=gnu99
CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -ffreestanding
CFLAGS += -fshort-enums
CFLAGS += -fverbose-asm
CFLAGS += -Wall
CFLAGS += -I/usr/avr/include
CFLAGS += -Wstrict-prototypes
CFLAGS += -DF_CPU=$(F_CPU)

SDL_CFLAGS = -std=gnu99 -Wall

ESC = \033[
DEBUG = $(ESC)34;40;1m
PROGRAM = $(ESC)33;40;1m
STOP = $(ESC)0m

sdl: $(SDL_OBJS)
	gcc $(SDL_CFLAGS) -o sdl $(SDL_OBJS) -lSDL2

all: $(AVR_OBJS)
	@echo -e " $(DEBUG)>>> Creating TARGET...$(STOP)"
	$(CC) $(CFLAGS) -Wl,-Map,$(TARGET).map -o $(TARGET).elf $(AVR_OBJS)
	@echo -e " $(DEBUG)>>> Creating HEX...$(STOP)"
	$(OBJCOPY) -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
	@echo -e "  $(DEBUG)>>> Size dump:$(STOP)"
	$(OBJSIZE) $(TARGET).elf

clean:
	@echo -e "$(DEBUG)>>> Removing the rubbish...$(STOP)"
	rm -f *.o *.hex *.elf *.map *.lst

%.sdl.o: %.c
	gcc $(SDL_CFLAGS) -c -o $@ $<

%.avr.o: %.c
	@echo -e " $(DEBUG)>>> Creating $@ file...$(STOP)"
	$(CC) $(CFLAGS) -c -o $@ $<

%.lst: %.elf
	@echo -e " $(DEBUG)>>> Creating $@ file...$(STOP)"
	$(OBJDUMP) -d $< > $@

program: all install
debug: all $(TARGET).lst
pprogram: clean all program
aall: clean all

install:
	@echo -e " $(PROGRAM)>!> Programming!$(STOP)"
	avrdude -p $(MCU) -P $(DUDE_PORT) -c $(DUDE_PROGRAMMER) -U flash:w:$(TARGET).hex