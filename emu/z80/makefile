#### Makefile for Z80Em
# Following #defines can be used:
# DEBUG			Compile with builtin debugger
# LSB_FIRST		Compile for a low-endian machine
# X86_ASM		Use inline 80x86 assembly (GCC only)

# C compiler used
CC	= gcc
# Linker used
#LD	= wcl386

CFLAGS	= -Wall -O2 -fomit-frame-pointer -DLSB_FIRST -D_DEBUG -c
#CFLAGS	= -5r -zk0 -ort -dLSB_FIRST -dINLINE=_inline -dDEBUG
INCLUDES = -I../inc

vpath %.h ../inc

all:    z80.o z80debug.o z80dasm

.c.o :
	$(CC) $(CFLAGS) $< $(INCLUDES)

#Z80Dasm.o:  Z80Dasm.c
z80dasm:    Z80Dasm.o
	$(CC) -o z80dasm Z80Dasm.o

Z80Debug.o: z80.h z80io.h Z80Dasm.h
Z80.o:      z80.h Z80Codes.h z80io.h Z80DAA.h mz80rpi.h
