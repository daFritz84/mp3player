#
#	MP3 Player 
#
#	author: 	Stefan Seifried
#	matr.nr.: 	0925401
#

PROJNAME	=	mp3player
MCU			=	atmega16
LIBCDIR		=	/usr/lib/avr

#--------------------------------------------------------------
# objects
#--------------------------------------------------------------
OBJS	=	main.o usart.o usart_stdout.o stringtable.o os_scheduler.o \
			task_keypad.o event_userio.o mmc_driver.o mp3decoder.o \
			task_playercontrol.o task_usart.o task_adc.o asm_spi.o \
			event_decoder.o event_display.o task_mp3decoder.o \
			lcd.o task_display.o task_mmccard.o fat.o stringman.o
#--------------------------------------------------------------
# tools
#--------------------------------------------------------------
CC		= 	avr-gcc 
OBJCOPY = 	avr-objcopy
PROGR	=	at16prog

#--------------------------------------------------------------
# flags
#--------------------------------------------------------------
CFLAGS  = 	-mmcu=$(MCU) -Wall -Os -mcall-prologues
AFLAGS	=	-mmcu=$(MCU) -Wall -Os -mcall-prologues -x assembler-with-cpp
LDFLAGS	= 	-Wl,-Map=$*.map -mmcu=$(MCU)
OCFLAGS	=	-O ihex
PRFLAGS	= 	-dpart=$(MCU)
CDEFS	=	$(shell [ -f $(LIBCDIR)/include/avr/version.h ] && echo "-D__AVR_VERSION_H_EXISTS__" )

#--------------------------------------------------------------
# targets
#--------------------------------------------------------------
all: 	$(PROJNAME).hex $(PROJNAME).elf $(OBJS)

%.hex:	%.elf
	$(OBJCOPY) $(OCFLAGS) $< $@
	@chmod ugo-x $@

%.elf:	$(OBJS)
	$(CC) $(OBJS) $(LIBS) $(LDFLAGS) -o $@

%.o:	%.c
	$(CC) $(CFLAGS) $(CDEFS) -Wa,-a=$*.list -c $< -o $@

%.o:	%.s
	$(CC) $(AFLAGS) -c $< -o $@

install: $(PROJNAME).hex
	$(PROGR) $(PRFLAGS) --segment=flash --erase
	$(PROGR) $(PRFLAGS) --segment=flash --upload if=$(PROJNAME).hex
	$(PROGR) $(PRFLAGS) --segment=flash --verify if=$(PROJNAME).hex
	
clean:
	rm -f *.hex *.elf *.map *.list *.o

size: $(PROJNAME).elf
	avr-size --format=avr --mcu=$(MCU) $(PROJNAME).elf

sizeold: $(PROJNAME).elf	
	avr-size $(PROJNAME).elf

# this is the super target: clean, make all, install + memory analysis
dist: clean all install sizeold

