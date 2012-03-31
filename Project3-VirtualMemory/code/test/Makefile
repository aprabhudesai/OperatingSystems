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

all: halt matmult sort testfiles  uprgsortFork uprgsortExec uprgmatmultFork uprgmatmultExec uprgmatsortFork uprgmatsortExec rpcLockTest rpcMVTest2_1 rpcMVTest2_2 rpcMVTest3_1 rpcMVTest3_2 negativeTest_1 negativeTest_2 negativeTest_3 negativeTest_4_1 negativeTest_4_2 negativeTest_5 negativeTest_6 rpcTest1 rpcTest2 rpcTest3 rpcTest4 rpcTest5

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

uprgsortFork.o: uprgsortFork.c
	$(CC) $(CFLAGS) -c uprgsortFork.c
uprgsortFork: uprgsortFork.o start.o
	$(LD) $(LDFLAGS) start.o uprgsortFork.o -o uprgsortFork.coff
	../bin/coff2noff uprgsortFork.coff uprgsortFork

uprgsortExec.o: uprgsortExec.c
	$(CC) $(CFLAGS) -c uprgsortExec.c
uprgsortExec: uprgsortExec.o start.o
	$(LD) $(LDFLAGS) start.o uprgsortExec.o -o uprgsortExec.coff
	../bin/coff2noff uprgsortExec.coff uprgsortExec

uprgmatmultFork.o: uprgmatmultFork.c
	$(CC) $(CFLAGS) -c uprgmatmultFork.c
uprgmatmultFork: uprgmatmultFork.o start.o
	$(LD) $(LDFLAGS) start.o uprgmatmultFork.o -o uprgmatmultFork.coff
	../bin/coff2noff uprgmatmultFork.coff uprgmatmultFork

uprgmatmultExec.o: uprgmatmultExec.c
	$(CC) $(CFLAGS) -c uprgmatmultExec.c
uprgmatmultExec: uprgmatmultExec.o start.o
	$(LD) $(LDFLAGS) start.o uprgmatmultExec.o -o uprgmatmultExec.coff
	../bin/coff2noff uprgmatmultExec.coff uprgmatmultExec	

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

uprgmatsortFork.o: uprgmatsortFork.c
	$(CC) $(CFLAGS) -c uprgmatsortFork.c
uprgmatsortFork: uprgmatsortFork.o start.o
	$(LD) $(LDFLAGS) start.o uprgmatsortFork.o -o uprgmatsortFork.coff
	../bin/coff2noff uprgmatsortFork.coff uprgmatsortFork

uprgmatsortExec.o: uprgmatsortExec.c
	$(CC) $(CFLAGS) -c uprgmatsortExec.c
uprgmatsortExec: uprgmatsortExec.o start.o
	$(LD) $(LDFLAGS) start.o uprgmatsortExec.o -o uprgmatsortExec.coff
	../bin/coff2noff uprgmatsortExec.coff uprgmatsortExec
	
testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

negativeTest_1.o: negativeTest_1.c
	$(CC) $(CFLAGS) -c negativeTest_1.c
negativeTest_1: negativeTest_1.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_1.o -o negativeTest_1.coff
	../bin/coff2noff negativeTest_1.coff negativeTest_1

negativeTest_2.o: negativeTest_2.c
	$(CC) $(CFLAGS) -c negativeTest_2.c
negativeTest_2: negativeTest_2.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_2.o -o negativeTest_2.coff
	../bin/coff2noff negativeTest_2.coff negativeTest_2

negativeTest_3.o: negativeTest_3.c
	$(CC) $(CFLAGS) -c negativeTest_3.c
negativeTest_3: negativeTest_3.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_3.o -o negativeTest_3.coff
	../bin/coff2noff negativeTest_3.coff negativeTest_3

negativeTest_4_1.o: negativeTest_4_1.c
	$(CC) $(CFLAGS) -c negativeTest_4_1.c
negativeTest_4_1: negativeTest_4_1.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_4_1.o -o negativeTest_4_1.coff
	../bin/coff2noff negativeTest_4_1.coff negativeTest_4_1

negativeTest_4_2.o: negativeTest_4_2.c
	$(CC) $(CFLAGS) -c negativeTest_4_2.c
negativeTest_4_2: negativeTest_4_2.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_4_2.o -o negativeTest_4_2.coff
	../bin/coff2noff negativeTest_4_2.coff negativeTest_4_2

negativeTest_5.o: negativeTest_5.c
	$(CC) $(CFLAGS) -c negativeTest_5.c
negativeTest_5: negativeTest_5.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_5.o -o negativeTest_5.coff
	../bin/coff2noff negativeTest_5.coff negativeTest_5

negativeTest_6.o: negativeTest_6.c
	$(CC) $(CFLAGS) -c negativeTest_6.c
negativeTest_6: negativeTest_6.o start.o
	$(LD) $(LDFLAGS) start.o negativeTest_6.o -o negativeTest_6.coff
	../bin/coff2noff negativeTest_6.coff negativeTest_6
	
rpcLockTest.o: rpcLockTest.c
	$(CC) $(CFLAGS) -c rpcLockTest.c
rpcLockTest: rpcLockTest.o start.o
	$(LD) $(LDFLAGS) start.o rpcLockTest.o -o rpcLockTest.coff
	../bin/coff2noff rpcLockTest.coff rpcLockTest

rpcMVTest2_1.o: rpcMVTest2_1.c
	$(CC) $(CFLAGS) -c rpcMVTest2_1.c
rpcMVTest2_1: rpcMVTest2_1.o start.o
	$(LD) $(LDFLAGS) start.o rpcMVTest2_1.o -o rpcMVTest2_1.coff
	../bin/coff2noff rpcMVTest2_1.coff rpcMVTest2_1

rpcMVTest2_2.o: rpcMVTest2_2.c
	$(CC) $(CFLAGS) -c rpcMVTest2_2.c
rpcMVTest2_2: rpcMVTest2_2.o start.o
	$(LD) $(LDFLAGS) start.o rpcMVTest2_2.o -o rpcMVTest2_2.coff
	../bin/coff2noff rpcMVTest2_2.coff rpcMVTest2_2

rpcMVTest3_1.o: rpcMVTest3_1.c
	$(CC) $(CFLAGS) -c rpcMVTest3_1.c
rpcMVTest3_1: rpcMVTest3_1.o start.o
	$(LD) $(LDFLAGS) start.o rpcMVTest3_1.o -o rpcMVTest3_1.coff
	../bin/coff2noff rpcMVTest3_1.coff rpcMVTest3_1

rpcMVTest3_2.o: rpcMVTest3_2.c
	$(CC) $(CFLAGS) -c rpcMVTest3_2.c
rpcMVTest3_2: rpcMVTest3_2.o start.o
	$(LD) $(LDFLAGS) start.o rpcMVTest3_2.o -o rpcMVTest3_2.coff
	../bin/coff2noff rpcMVTest3_2.coff rpcMVTest3_2

rpcTest1.o: rpcTest1.c
	$(CC) $(CFLAGS) -c rpcTest1.c
rpcTest1: rpcTest1.o start.o
	$(LD) $(LDFLAGS) start.o rpcTest1.o -o rpcTest1.coff
	../bin/coff2noff rpcTest1.coff rpcTest1

rpcTest2.o: rpcTest2.c
	$(CC) $(CFLAGS) -c rpcTest2.c
rpcTest2: rpcTest2.o start.o
	$(LD) $(LDFLAGS) start.o rpcTest2.o -o rpcTest2.coff
	../bin/coff2noff rpcTest2.coff rpcTest2

rpcTest3.o: rpcTest3.c
	$(CC) $(CFLAGS) -c rpcTest3.c
rpcTest3: rpcTest3.o start.o
	$(LD) $(LDFLAGS) start.o rpcTest3.o -o rpcTest3.coff
	../bin/coff2noff rpcTest3.coff rpcTest3

rpcTest4.o: rpcTest4.c
	$(CC) $(CFLAGS) -c rpcTest4.c
rpcTest4: rpcTest4.o start.o
	$(LD) $(LDFLAGS) start.o rpcTest4.o -o rpcTest4.coff
	../bin/coff2noff rpcTest4.coff rpcTest4

rpcTest5.o: rpcTest5.c
	$(CC) $(CFLAGS) -c rpcTest5.c
rpcTest5: rpcTest5.o start.o
	$(LD) $(LDFLAGS) start.o rpcTest5.o -o rpcTest5.coff
	../bin/coff2noff rpcTest5.coff rpcTest5	
clean:
	rm -f *.o *.coff
