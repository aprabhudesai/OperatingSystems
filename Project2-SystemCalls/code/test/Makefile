# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles userprog1 userprog2 userprogInteract1 userprog3

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
userprog1.o: userprog1.c
	$(CC) $(CFLAGS) -c userprog1.c
userprog1: userprog1.o start.o
	$(LD) $(LDFLAGS) start.o userprog1.o -o userprog1.coff
	../bin/coff2noff userprog1.coff userprog1

userprog2.o: userprog2.c
	$(CC) $(CFLAGS) -c userprog2.c
userprog2: userprog2.o start.o
	$(LD) $(LDFLAGS) start.o userprog2.o -o userprog2.coff
	../bin/coff2noff userprog2.coff userprog2

userprogInteract1.o: userprogInteract1.c
	$(CC) $(CFLAGS) -c userprogInteract1.c
userprogInteract1: userprogInteract1.o start.o
	$(LD) $(LDFLAGS) start.o userprogInteract1.o -o userprogInteract1.coff
	../bin/coff2noff userprogInteract1.coff userprogInteract1

userprog3.o: userprog3.c
	$(CC) $(CFLAGS) -c userprog3.c
userprog3: userprog3.o start.o
	$(LD) $(LDFLAGS) start.o userprog3.o -o userprog3.coff
	../bin/coff2noff userprog3.coff userprog3
	
sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

clean:
	rm -f *.o *.coff
