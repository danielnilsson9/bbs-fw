.PHONY: all clean

# Compiler
CC = sdcc

#Platform
PLATFORM = mcs51

# Target name
PNAME = main

# External libraries
IDIR = 
SDIR = 

# In case you ever want a different name for the main source file
MAINSRC = main.c

# These are the sources that must be compiled to .rel files:
EXTRASRCS = \
	motor.c
	
HEADERS = \
	motor.h

# The list of .rel files can be derived from the list of their source files
RELS = $(EXTRASRCS:.c=.rel)

INCLUDES = -I$(IDIR) -I. -I../
CFLAGS   = -m$(PLATFORM) -Ddouble=float --std-c99 --nolospre 
ELF_FLAGS = 
LIBS     = 

# This just provides the conventional target name "all"; it is optional
# Note: I assume you set PNAME via some means not exhibited in your original file
all: $(PNAME) hex

# How to build the overall program
$(PNAME): $(MAINSRC) $(RELS)
	$(CC) $(INCLUDES) $(CFLAGS) $(ELF_FLAGS) $(LIBS) $(MAINSRC) $(RELS)

# How to build any .rel file from its corresponding .c file
# GNU would have you use a pattern rule for this, but that's GNU-specific
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
