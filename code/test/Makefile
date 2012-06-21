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

all: halt matmult sort testfiles project2_part1_testsuite project2_part2_testsuite addrspace_verification \
HelloWorld Smiley NumberCounter ExecAddingThings MaxLockCVMaker MakeSomeLocksAndCVs queueTest supermarket startsupermarket


start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt
	
project2_part1_testsuite.o: project2_part1_testsuite.c
	$(CC) $(CFLAGS) -c project2_part1_testsuite.c
project2_part1_testsuite: project2_part1_testsuite.o start.o
	$(LD) $(LDFLAGS) start.o project2_part1_testsuite.o -o project2_part1_testsuite.coff
	../bin/coff2noff project2_part1_testsuite.coff project2_part1_testsuite
	
project2_part2_testsuite.o: project2_part2_testsuite.c
	$(CC) $(CFLAGS) -c project2_part2_testsuite.c
project2_part2_testsuite: project2_part2_testsuite.o start.o
	$(LD) $(LDFLAGS) start.o project2_part2_testsuite.o -o project2_part2_testsuite.coff
	../bin/coff2noff project2_part2_testsuite.coff project2_part2_testsuite

supermarket.o: supermarket.c
	$(CC) $(CFLAGS) -c supermarket.c
supermarket: supermarket.o start.o
	$(LD) $(LDFLAGS) start.o supermarket.o -o supermarket.coff
	../bin/coff2noff supermarket.coff supermarket
	
startsupermarket.o: startsupermarket.c
	$(CC) $(CFLAGS) -c startsupermarket.c
startsupermarket: startsupermarket.o start.o
	$(LD) $(LDFLAGS) start.o startsupermarket.o -o startsupermarket.coff
	../bin/coff2noff startsupermarket.coff startsupermarket

addrspace_verification.o: addrspace_verification.c
	$(CC) $(CFLAGS) -c addrspace_verification.c
addrspace_verification: addrspace_verification.o start.o
	$(LD) $(LDFLAGS) start.o addrspace_verification.o -o addrspace_verification.coff
	../bin/coff2noff addrspace_verification.coff addrspace_verification
	
HelloWorld.o: HelloWorld.c
	$(CC) $(CFLAGS) -c HelloWorld.c
HelloWorld: HelloWorld.o start.o
	$(LD) $(LDFLAGS) start.o HelloWorld.o -o HelloWorld.coff
	../bin/coff2noff HelloWorld.coff HelloWorld

queueTest.o: queueTest.c
	$(CC) $(CFLAGS) -c queueTest.c
queueTest: queueTest.o start.o
	$(LD) $(LDFLAGS) start.o queueTest.o -o queueTest.coff
	../bin/coff2noff queueTest.coff queueTest

Smiley.o: Smiley.c
	$(CC) $(CFLAGS) -c Smiley.c
Smiley: Smiley.o start.o
	$(LD) $(LDFLAGS) start.o Smiley.o -o Smiley.coff
	../bin/coff2noff Smiley.coff Smiley

MaxLockCVMaker.o: MaxLockCVMaker.c
	$(CC) $(CFLAGS) -c MaxLockCVMaker.c
MaxLockCVMaker: MaxLockCVMaker.o start.o
	$(LD) $(LDFLAGS) start.o MaxLockCVMaker.o -o MaxLockCVMaker.coff
	../bin/coff2noff MaxLockCVMaker.coff MaxLockCVMaker

MakeSomeLocksAndCVs.o: MakeSomeLocksAndCVs.c
	$(CC) $(CFLAGS) -c MakeSomeLocksAndCVs.c
MakeSomeLocksAndCVs: MakeSomeLocksAndCVs.o start.o
	$(LD) $(LDFLAGS) start.o MakeSomeLocksAndCVs.o -o MakeSomeLocksAndCVs.coff
	../bin/coff2noff MakeSomeLocksAndCVs.coff MakeSomeLocksAndCVs

NumberCounter.o: NumberCounter.c
	$(CC) $(CFLAGS) -c NumberCounter.c
NumberCounter: NumberCounter.o start.o
	$(LD) $(LDFLAGS) start.o NumberCounter.o -o NumberCounter.coff
	../bin/coff2noff NumberCounter.coff NumberCounter

ExecAddingThings.o: ExecAddingThings.c
	$(CC) $(CFLAGS) -c ExecAddingThings.c
ExecAddingThings: ExecAddingThings.o start.o
	$(LD) $(LDFLAGS) start.o ExecAddingThings.o -o ExecAddingThings.coff
	../bin/coff2noff ExecAddingThings.coff ExecAddingThings

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
