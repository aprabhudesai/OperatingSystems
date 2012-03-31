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

all: ticketClerk headCustomer rpcTest1 rpcTest2 rpcTest3 rpcTest4 rpcTest5 distributedNegativeTest_5 initializeSystem execMovieEntity1 execMovieEntity2 execCustomers concessionClerk distributedLocksTest groupMember ticketTaker movieTechnician movieManager distributedNegativeTest_1 distributedNegativeTest_2 distributedNegativeTest_3 distributedNegativeTest_4_1 distributedNegativeTest_4_2 specialTest_1 execSpecialTest_1 distributedLockAndCVTest_1 distributedLockAndCVTest_2 distributedLockAndCVAndMVTest_1 distributedLockAndCVAndMVTest_2 distributedLockAndCVAndMVTest_3 specialTest2_1 specialTest2_2 execSpecialTest2_1 execSpecialTest2_2 pingTest1 pingTest2 pingTest3 pingTest4 pingTest5

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

ticketClerk.o: ticketClerk.c
	$(CC) $(CFLAGS) -c ticketClerk.c
ticketClerk: ticketClerk.o start.o
	$(LD) $(LDFLAGS) start.o ticketClerk.o -o ticketClerk.coff
	../bin/coff2noff ticketClerk.coff ticketClerk

headCustomer.o: headCustomer.c
	$(CC) $(CFLAGS) -c headCustomer.c
headCustomer: headCustomer.o start.o
	$(LD) $(LDFLAGS) start.o headCustomer.o -o headCustomer.coff
	../bin/coff2noff headCustomer.coff headCustomer

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

initializeSystem.o: initializeSystem.c
	$(CC) $(CFLAGS) -c initializeSystem.c
initializeSystem: initializeSystem.o start.o
	$(LD) $(LDFLAGS) start.o initializeSystem.o -o initializeSystem.coff
	../bin/coff2noff initializeSystem.coff initializeSystem

execMovieEntity1.o: execMovieEntity1.c
	$(CC) $(CFLAGS) -c execMovieEntity1.c
execMovieEntity1: execMovieEntity1.o start.o
	$(LD) $(LDFLAGS) start.o execMovieEntity1.o -o execMovieEntity1.coff
	../bin/coff2noff execMovieEntity1.coff execMovieEntity1

execMovieEntity2.o: execMovieEntity2.c
	$(CC) $(CFLAGS) -c execMovieEntity2.c
execMovieEntity2: execMovieEntity2.o start.o
	$(LD) $(LDFLAGS) start.o execMovieEntity2.o -o execMovieEntity2.coff
	../bin/coff2noff execMovieEntity2.coff execMovieEntity2

execCustomers.o: execCustomers.c
	$(CC) $(CFLAGS) -c execCustomers.c
execCustomers: execCustomers.o start.o
	$(LD) $(LDFLAGS) start.o execCustomers.o -o execCustomers.coff
	../bin/coff2noff execCustomers.coff execCustomers

concessionClerk.o: concessionClerk.c
	$(CC) $(CFLAGS) -c concessionClerk.c
concessionClerk: concessionClerk.o start.o
	$(LD) $(LDFLAGS) start.o concessionClerk.o -o concessionClerk.coff
	../bin/coff2noff concessionClerk.coff concessionClerk
	
distributedLocksTest.o: distributedLocksTest.c
	$(CC) $(CFLAGS) -c distributedLocksTest.c
distributedLocksTest: distributedLocksTest.o start.o
	$(LD) $(LDFLAGS) start.o distributedLocksTest.o -o distributedLocksTest.coff
	../bin/coff2noff distributedLocksTest.coff distributedLocksTest

groupMember.o: groupMember.c
	$(CC) $(CFLAGS) -c groupMember.c
groupMember: groupMember.o start.o
	$(LD) $(LDFLAGS) start.o groupMember.o -o groupMember.coff
	../bin/coff2noff groupMember.coff groupMember

ticketTaker.o: ticketTaker.c
	$(CC) $(CFLAGS) -c ticketTaker.c
ticketTaker: ticketTaker.o start.o
	$(LD) $(LDFLAGS) start.o ticketTaker.o -o ticketTaker.coff
	../bin/coff2noff ticketTaker.coff ticketTaker

movieTechnician.o: movieTechnician.c
	$(CC) $(CFLAGS) -c movieTechnician.c
movieTechnician: movieTechnician.o start.o
	$(LD) $(LDFLAGS) start.o movieTechnician.o -o movieTechnician.coff
	../bin/coff2noff movieTechnician.coff movieTechnician
	
movieManager.o: movieManager.c
	$(CC) $(CFLAGS) -c movieManager.c
movieManager: movieManager.o start.o
	$(LD) $(LDFLAGS) start.o movieManager.o -o movieManager.coff
	../bin/coff2noff movieManager.coff movieManager

distributedNegativeTest_1.o: distributedNegativeTest_1.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_1.c
distributedNegativeTest_1: distributedNegativeTest_1.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_1.o -o distributedNegativeTest_1.coff
	../bin/coff2noff distributedNegativeTest_1.coff distributedNegativeTest_1

distributedNegativeTest_2.o: distributedNegativeTest_2.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_2.c
distributedNegativeTest_2: distributedNegativeTest_2.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_2.o -o distributedNegativeTest_2.coff
	../bin/coff2noff distributedNegativeTest_2.coff distributedNegativeTest_2

distributedNegativeTest_3.o: distributedNegativeTest_3.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_3.c
distributedNegativeTest_3: distributedNegativeTest_3.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_3.o -o distributedNegativeTest_3.coff
	../bin/coff2noff distributedNegativeTest_3.coff distributedNegativeTest_3

distributedNegativeTest_4_1.o: distributedNegativeTest_4_1.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_4_1.c
distributedNegativeTest_4_1: distributedNegativeTest_4_1.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_4_1.o -o distributedNegativeTest_4_1.coff
	../bin/coff2noff distributedNegativeTest_4_1.coff distributedNegativeTest_4_1

distributedNegativeTest_4_2.o: distributedNegativeTest_4_2.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_4_2.c
distributedNegativeTest_4_2: distributedNegativeTest_4_2.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_4_2.o -o distributedNegativeTest_4_2.coff
	../bin/coff2noff distributedNegativeTest_4_2.coff distributedNegativeTest_4_2

distributedNegativeTest_5.o: distributedNegativeTest_5.c
	$(CC) $(CFLAGS) -c distributedNegativeTest_5.c
distributedNegativeTest_5: distributedNegativeTest_5.o start.o
	$(LD) $(LDFLAGS) start.o distributedNegativeTest_5.o -o distributedNegativeTest_5.coff
	../bin/coff2noff distributedNegativeTest_5.coff distributedNegativeTest_5

specialTest_1.o: specialTest_1.c
	$(CC) $(CFLAGS) -c specialTest_1.c
specialTest_1: specialTest_1.o start.o
	$(LD) $(LDFLAGS) start.o specialTest_1.o -o specialTest_1.coff
	../bin/coff2noff specialTest_1.coff specialTest_1

execSpecialTest_1.o: execSpecialTest_1.c
	$(CC) $(CFLAGS) -c execSpecialTest_1.c
execSpecialTest_1: execSpecialTest_1.o start.o
	$(LD) $(LDFLAGS) start.o execSpecialTest_1.o -o execSpecialTest_1.coff
	../bin/coff2noff execSpecialTest_1.coff execSpecialTest_1

distributedLockAndCVTest_1.o: distributedLockAndCVTest_1.c
	$(CC) $(CFLAGS) -c distributedLockAndCVTest_1.c
distributedLockAndCVTest_1: distributedLockAndCVTest_1.o start.o
	$(LD) $(LDFLAGS) start.o distributedLockAndCVTest_1.o -o distributedLockAndCVTest_1.coff
	../bin/coff2noff distributedLockAndCVTest_1.coff distributedLockAndCVTest_1

distributedLockAndCVTest_2.o: distributedLockAndCVTest_2.c
	$(CC) $(CFLAGS) -c distributedLockAndCVTest_2.c
distributedLockAndCVTest_2: distributedLockAndCVTest_2.o start.o
	$(LD) $(LDFLAGS) start.o distributedLockAndCVTest_2.o -o distributedLockAndCVTest_2.coff
	../bin/coff2noff distributedLockAndCVTest_2.coff distributedLockAndCVTest_2

distributedLockAndCVAndMVTest_1.o: distributedLockAndCVAndMVTest_1.c
	$(CC) $(CFLAGS) -c distributedLockAndCVAndMVTest_1.c
distributedLockAndCVAndMVTest_1: distributedLockAndCVAndMVTest_1.o start.o
	$(LD) $(LDFLAGS) start.o distributedLockAndCVAndMVTest_1.o -o distributedLockAndCVAndMVTest_1.coff
	../bin/coff2noff distributedLockAndCVAndMVTest_1.coff distributedLockAndCVAndMVTest_1

distributedLockAndCVAndMVTest_2.o: distributedLockAndCVAndMVTest_2.c
	$(CC) $(CFLAGS) -c distributedLockAndCVAndMVTest_2.c
distributedLockAndCVAndMVTest_2: distributedLockAndCVAndMVTest_2.o start.o
	$(LD) $(LDFLAGS) start.o distributedLockAndCVAndMVTest_2.o -o distributedLockAndCVAndMVTest_2.coff
	../bin/coff2noff distributedLockAndCVAndMVTest_2.coff distributedLockAndCVAndMVTest_2

distributedLockAndCVAndMVTest_3.o: distributedLockAndCVAndMVTest_3.c
	$(CC) $(CFLAGS) -c distributedLockAndCVAndMVTest_3.c
distributedLockAndCVAndMVTest_3: distributedLockAndCVAndMVTest_3.o start.o
	$(LD) $(LDFLAGS) start.o distributedLockAndCVAndMVTest_3.o -o distributedLockAndCVAndMVTest_3.coff
	../bin/coff2noff distributedLockAndCVAndMVTest_3.coff distributedLockAndCVAndMVTest_3

specialTest2_1.o: specialTest2_1.c
	$(CC) $(CFLAGS) -c specialTest2_1.c
specialTest2_1: specialTest2_1.o start.o
	$(LD) $(LDFLAGS) start.o specialTest2_1.o -o specialTest2_1.coff
	../bin/coff2noff specialTest2_1.coff specialTest2_1

specialTest2_2.o: specialTest2_2.c
	$(CC) $(CFLAGS) -c specialTest2_2.c
specialTest2_2: specialTest2_2.o start.o
	$(LD) $(LDFLAGS) start.o specialTest2_2.o -o specialTest2_2.coff
	../bin/coff2noff specialTest2_2.coff specialTest2_2

execSpecialTest2_1.o: execSpecialTest2_1.c
	$(CC) $(CFLAGS) -c execSpecialTest2_1.c
execSpecialTest2_1: execSpecialTest2_1.o start.o
	$(LD) $(LDFLAGS) start.o execSpecialTest2_1.o -o execSpecialTest2_1.coff
	../bin/coff2noff execSpecialTest2_1.coff execSpecialTest2_1

execSpecialTest2_2.o: execSpecialTest2_2.c
	$(CC) $(CFLAGS) -c execSpecialTest2_2.c
execSpecialTest2_2: execSpecialTest2_2.o start.o
	$(LD) $(LDFLAGS) start.o execSpecialTest2_2.o -o execSpecialTest2_2.coff
	../bin/coff2noff execSpecialTest2_2.coff execSpecialTest2_2

pingTest1.o: pingTest1.c
	$(CC) $(CFLAGS) -c pingTest1.c
pingTest1: pingTest1.o start.o
	$(LD) $(LDFLAGS) start.o pingTest1.o -o pingTest1.coff
	../bin/coff2noff pingTest1.coff pingTest1

pingTest2.o: pingTest2.c
	$(CC) $(CFLAGS) -c pingTest2.c
pingTest2: pingTest2.o start.o
	$(LD) $(LDFLAGS) start.o pingTest2.o -o pingTest2.coff
	../bin/coff2noff pingTest2.coff pingTest2	
	
pingTest3.o: pingTest3.c
	$(CC) $(CFLAGS) -c pingTest3.c
pingTest3: pingTest3.o start.o
	$(LD) $(LDFLAGS) start.o pingTest3.o -o pingTest3.coff
	../bin/coff2noff pingTest3.coff pingTest3

pingTest4.o: pingTest4.c
	$(CC) $(CFLAGS) -c pingTest4.c
pingTest4: pingTest4.o start.o
	$(LD) $(LDFLAGS) start.o pingTest4.o -o pingTest4.coff
	../bin/coff2noff pingTest4.coff pingTest4

pingTest5.o: pingTest5.c
	$(CC) $(CFLAGS) -c pingTest5.c
pingTest5: pingTest5.o start.o
	$(LD) $(LDFLAGS) start.o pingTest5.o -o pingTest5.coff
	../bin/coff2noff pingTest5.coff pingTest5

clean:
	rm -f *.o *.coff
