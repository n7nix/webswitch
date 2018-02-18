#
# Makefile for cron temperature check & webswitch
#
# Please note that this Makefile *needs* GNU make. BSD make won't do.
#


VERSION = 1.0

# Before using this variable run: gmake clean
#GET_SMALL = 1 # comment this out to use Hobby Boards 6 chan hub
#USE_SERIAL = 1

TARGET1	= cronswit
CC	= gcc
CFLAGS	= -Os -Wall # optimize for size & show all warnings
# CFLAGS += -O2  # code optimizations
# CFLAGS += -g -gstabs # debugging options note stabs is used by dbx
#LIBS	= -lm -L/usr/local/lib -lcurl
LIBS	= -lm -L/usr/lib -lcurl

#LIBS    = -lc

OBJS		=	$(TARGET1).o webswitch.o
HDRS		= 	common.h logger.h

#CFLAGS  +=  -I/usr/local/include -I../socketds9097
CFLAGS  +=  -I/usr/local/include

# -----------------------------------------------------------------------
# Sort out what operating system is being run and modify CFLAGS and LIBS
#
# If you add a new OSTYPE here please email it to me so that I can add
# it to the distribution in the next release
# -----------------------------------------------------------------------
SYSTYPE := $(shell uname -s)

ifeq ($(SYSTYPE), Linux)
  CFLAGS += -DLINUX

  # Set LOCK to yes for serial port locking support
  LOCK = no
endif
ifeq ($(SYSTYPE), FreeBSD)
  CFLAGS += -DFREEBSD
endif


all:  $(TARGET1)

help:
	@echo "  SYSTYPE = $(SYSTYPE)"
	@echo "  CFLAGS = $(CFLAGS)"
	@echo "  LIBS   = $(LIBS)"
	@echo ""
	@echo ""
	@echo "NOTE: You must use GNU make when building with this makefile"
	@echo ""


# Build the Linux executable
$(TARGET1):	$(TARGET1).c $(OBJS) $(HDRS) Makefile
		$(CC) $(OBJS) $(LIBS) -o $@


# Clean up object files
clean:
		rm -f $(TARGET1) core *.asc *.o

