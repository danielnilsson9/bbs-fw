.PHONY: all clean

# Compiler
CC = sdcc

#Platform
PLATFORM = mcs51

# Target name
PNAME = main


# When addding support for more controllers, define
# new entries here if auto detection cannot be supported.
TARGET_CONTROLLER = BBSHD


# External libraries
IDIR = 
SDIR = 

MAINSRC = main.c

EXTRASRCS = \
	adc.c \
	app.c \
	cfgstore.c \
	eeprom.c \
	eventlog.c \
	extcom.c \
	lights.c \
	motor.c \
	uart.c \
	system.c \
	throttle.c \
	watchdog.c \
	sensors.c
	
HEADERS = \
	adc.h \
	app.h \
	cfgstore.h \
	eventlog.h \
	eeprom.h \
	extcom.h \
	lights.h \
	motor.h \
	uart.h \
	system.h \
	throttle.h \
	util.h \
	version.h \
	watchdog.h \
	sensors.h \
	pins.h

# The list of .rel files can be derived from the list of their source files
RELS = $(EXTRASRCS:.c=.rel)

INCLUDES = -I$(IDIR) -I. -I../
CFLAGS   = -m$(PLATFORM) --model-large -Ddouble=float --std-c99 -D$(TARGET_CONTROLLER)
ELF_FLAGS = 
LIBS     = 

all: $(PNAME) hex

$(PNAME): $(MAINSRC) $(RELS)
	$(CC) $(INCLUDES) $(CFLAGS) $(ELF_FLAGS) $(LIBS) $(MAINSRC) $(RELS)

# Build .rel file from .c file
%.rel: %.c $(HEADERS)
	$(CC) -c $(INCLUDES) $(CFLAGS) $(ELF_FLAGS) $(LIBS) $<

# Suffixes appearing in suffix rules we care about.
# Necessary because .rel is not one of the standard suffixes.
.SUFFIXES: .c .rel

hex:
	@cmd /C tohex.bat
	
clean:
	@cmd /C clean.bat
	@echo "Clean Finished"
