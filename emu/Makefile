OBJS = Mzmain.o Mzscrn.o MZhw.o Defkey.o mzbeep.o
Z80OBJS = ./z80/z80.o ./z80/z80debug.o
CC = gcc
CFLAGS = -Wall -O2 -std=gnu99 -pthread -D_DEBUG -c
INCLUDES = -I./inc -I./z80
LIBS = -L/usr/lib -L/usr/local/lib
LDFLAGS = -lrt -lpthread -pthread
PROGRAM = mz80rpi

vpath %.h ./inc:./z80
vpath %.c ./src

all:$(PROGRAM)

$(PROGRAM):$(OBJS)
	$(CC) $(OBJS) $(Z80OBJS) $(LDFLAGS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< $(INCLUDES)

Mzmain.o: mzscrn.h mzmain.h z80.h Z80Codes.h MZhw.h defkey.h mzbeep.h
Mzscrn.o: mzscrn.h mzmain.h MZhw.h
MZhw.o: mz80rpi.h z80.h Z80Codes.h MZhw.h mzscrn.h mzmain.h mzbeep.h
Defkey.o: mz80rpi.h z80.h defkey.h
mzbeep.o: mzbeep.h
