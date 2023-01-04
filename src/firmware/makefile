.PHONY: all clean

# Select target controller:
TARGET_CONTROLLER = BBSHD
#TARGET_CONTROLLER = BBS02
#TARGET_CONTROLLER = TSDZ2

# Compiler
CC = sdcc

# Target name
TARGET = bbs-fw

MAINSRC = main.c
SUBDIRS = 
		
CFLAGS = -Ddouble=float --std-c99 -D$(TARGET_CONTROLLER)
	
# Target Specific
ifeq ($(TARGET_CONTROLLER), BBSHD)
	CFLAGS += -mmcs51 --model-large --xram-size 3840	
	SUBDIRS += bbsx	
endif

ifeq ($(TARGET_CONTROLLER), BBS02)
	CFLAGS += -mmcs51 --model-large --xram-size 1792
	SUBDIRS += bbsx
endif

ifeq ($(TARGET_CONTROLLER), TSDZ2)
	CFLAGS += -mstm8
	SUBDIRS += tsdz2
endif
	
	
INCS = $(wildcard *.h $(foreach fd, $(SUBDIRS), $(fd)/*.h))
SRCS = $(filter-out main.c, $(wildcard *.c $(foreach fd, $(SUBDIRS), $(fd)/*.c)))
RELS := $(SRCS:.c=.rel)

INC_DIRS = -I./ $(addprefix -I, $(SUBDIRS))

all: $(TARGET) hex

$(TARGET): $(MAINSRC) $(RELS)
	$(CC) -o $(TARGET).ihx $(INC_DIRS) $(CFLAGS) $(MAINSRC) $(RELS)

%.rel: %.c $(INCS)
	$(CC) -o $@ -c $(INC_DIRS) $(CFLAGS) $<

echo:
	$(info SRCS: $(SRCS))
	$(info RELS: $(RELS))
	$(info INCS: $(INCS))

hex:
	@cmd /C tohex.bat
	
clean:
	@cmd /C clean.bat
	$(info Clean Finished)

.PHONY = all hex clean
.SUFFIXES: .c .rel
