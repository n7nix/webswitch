#
# Makefile for usb8chan_relay
#  - Sainsmart 8 channel usb relay board
#
#
VERSION = 0.1

CC	= gcc
CFLAGS	= -O2 -g -gstabs -Wall
LIBS	= -lc

RELAY_SRC = usb8chan_relay.c relay_drv.c

HDRS =

CFLAGS += -I/usr/local/include -DLINUX

SYSTYPE := $(shell uname -s)

LIBS   += -L/usr/lib

RELAY_SRC += relay_drv_sainsmart.c
LIBS	+= -lftdi -lusb -lusb-1.0
CFLAGS	+= -DDRV_SAINSMART -DBUILD_LIB

RELAY_OBJ = $(RELAY_SRC:.c=.o)

all:	usb8chan_relay

help:
	@echo "  SYSTYPE = $(SYSTYPE)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  LIBS   = $(LIBS)"
	@echo ""

usb8chan_relay: $(RELAY_OBJ) $(HDRS) Makefile
		$(CC) $(CFLAGS) $(RELAY_OBJ) -o usb8chan_relay $(LIBS)

.c.o:
	@echo "[Compile $<]"
	@$(CC) -c $(CFLAGS) $< -o $@  $(CFLAGS)

# Clean up the object files for distribution
clean:
		rm -f $(RELAY_OBJ)
		rm -f core *.asc
		rm -f usb8chan_relay
