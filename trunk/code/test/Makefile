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
HelloWorld Smiley NumberCounter ExecAddingThings MaxLockCVMaker MakeSomeLocksAndCVs queueTest supermarket startsupermarket \
exec_matmult test_fork_matmult exec_sort test_fork_sort testmutex testBroadcast testsignal \
testsequencing test_server_create_destroy test_server_creates test_server_acquire_release test_server_MV \
cashier customer goodsloader manager salesman p4startsupermarket testCreateRaceConditionDriver testCreateRaceCondition

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

testCreateRaceConditionDriver.o: testCreateRaceConditionDriver.c
	$(CC) $(CFLAGS) -c testCreateRaceConditionDriver.c
	
testCreateRaceConditionDriver: testCreateRaceConditionDriver.o start.o
	$(LD) $(LDFLAGS) start.o setup.o testCreateRaceConditionDriver.o -o testCreateRaceConditionDriver.coff
	../bin/coff2noff testCreateRaceConditionDriver.coff testCreateRaceConditionDriver
	
testCreateRaceCondition.o: testCreateRaceCondition.c
	$(CC) $(CFLAGS) -c testCreateRaceCondition.c
	
testCreateRaceCondition: testCreateRaceCondition.o start.o
	$(LD) $(LDFLAGS) start.o setup.o testCreateRaceCondition.o -o testCreateRaceCondition.coff
	../bin/coff2noff testCreateRaceCondition.coff testCreateRaceCondition
	
p4startsupermarket.o: p4startsupermarket.c
	$(CC) $(CFLAGS) -c p4startsupermarket.c
p4startsupermarket: p4startsupermarket.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startsupermarket.o -o p4startsupermarket.coff
	../bin/coff2noff p4startsupermarket.coff p4startsupermarket

p4startcustomer.o: p4startcustomer.c
	$(CC) $(CFLAGS) -c p4startcustomer.c
p4startcustomer: p4startcustomer.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startcustomer.o -o p4startcustomer.coff
	../bin/coff2noff p4startcustomer.coff p4startcustomer

p4startloaders.o: p4startloaders.c
	$(CC) $(CFLAGS) -c p4startloaders.c
p4startloaders: p4startloaders.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startloaders.o -o p4startloaders.coff
	../bin/coff2noff p4startloaders.coff p4startloaders

p4startcashier.o: p4startcashier.c
	$(CC) $(CFLAGS) -c p4startcashier.c
p4startcashier: p4startcashier.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startcashier.o -o p4startcashier.coff
	../bin/coff2noff p4startcashier.coff p4startcashier

p4startmanager.o: p4startmanager.c
	$(CC) $(CFLAGS) -c p4startmanager.c
p4startmanager: p4startmanager.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startmanager.o -o p4startmanager.coff
	../bin/coff2noff p4startmanager.coff p4startmanager

p4startsalesmen.o: p4startsalesmen.c
	$(CC) $(CFLAGS) -c p4startsalesmen.c
p4startsalesmen: p4startsalesmen.o start.o
	$(LD) $(LDFLAGS) start.o setup.o p4startsalesmen.o -o p4startsalesmen.coff
	../bin/coff2noff p4startsalesmen.coff p4startsalesmen

cashier.o: cashier.c
	$(CC) $(CFLAGS) -c cashier.c setup.c
cashier: cashier.o start.o
	$(LD) $(LDFLAGS) start.o setup.o cashier.o -o cashier.coff
	../bin/coff2noff cashier.coff cashier

customer.o: customer.c
	$(CC) $(CFLAGS) -c customer.c
customer: customer.o start.o
	$(LD) $(LDFLAGS) start.o setup.o customer.o -o customer.coff
	../bin/coff2noff customer.coff customer

goodsloader.o: goodsloader.c
	$(CC) $(CFLAGS) -c goodsloader.c
goodsloader: goodsloader.o start.o
	$(LD) $(LDFLAGS) start.o setup.o goodsloader.o -o goodsloader.coff
	../bin/coff2noff goodsloader.coff goodsloader

manager.o: manager.c
	$(CC) $(CFLAGS) -c manager.c
manager: manager.o start.o
	$(LD) $(LDFLAGS) start.o setup.o manager.o -o manager.coff
	../bin/coff2noff manager.coff manager

salesman.o: salesman.c
	$(CC) $(CFLAGS) -c salesman.c
salesman: salesman.o start.o
	$(LD) $(LDFLAGS) start.o setup.o salesman.o -o salesman.coff
	../bin/coff2noff salesman.coff salesman


test_server_create_destroy.o: test_server_create_destroy.c
	$(CC) $(CFLAGS) -c test_server_create_destroy.c
test_server_create_destroy: test_server_create_destroy.o start.o
	$(LD) $(LDFLAGS) start.o test_server_create_destroy.o -o test_server_create_destroy.coff
	../bin/coff2noff test_server_create_destroy.coff test_server_create_destroy

test_server_creates.o: test_server_creates.c
	$(CC) $(CFLAGS) -c test_server_creates.c
test_server_creates: test_server_creates.o start.o
	$(LD) $(LDFLAGS) start.o test_server_creates.o -o test_server_creates.coff
	../bin/coff2noff test_server_creates.coff test_server_creates

test_server_acquire_release.o: test_server_acquire_release.c
	$(CC) $(CFLAGS) -c test_server_acquire_release.c
test_server_acquire_release: test_server_acquire_release.o start.o
	$(LD) $(LDFLAGS) start.o test_server_acquire_release.o -o test_server_acquire_release.coff
	../bin/coff2noff test_server_acquire_release.coff test_server_acquire_release

test_server_MV.o: test_server_MV.c
	$(CC) $(CFLAGS) -c test_server_MV.c
test_server_MV: test_server_MV.o start.o
	$(LD) $(LDFLAGS) start.o test_server_MV.o -o test_server_MV.coff
	../bin/coff2noff test_server_MV.coff test_server_MV

testBroadcast.o: testBroadcast.c
	$(CC) $(CFLAGS) -c testBroadcast.c
testBroadcast: testBroadcast.o start.o
	$(LD) $(LDFLAGS) start.o testBroadcast.o -o testBroadcast.coff
	../bin/coff2noff testBroadcast.coff testBroadcast

testsignal.o: testsignal.c
	$(CC) $(CFLAGS) -c testsignal.c
testsignal: testsignal.o start.o
	$(LD) $(LDFLAGS) start.o testsignal.o -o testsignal.coff
	../bin/coff2noff testsignal.coff testsignal

testsequencing.o: testsequencing.c
	$(CC) $(CFLAGS) -c testsequencing.c
testsequencing: testsequencing.o start.o
	$(LD) $(LDFLAGS) start.o testsequencing.o -o testsequencing.coff
	../bin/coff2noff testsequencing.coff testsequencing

exec_matmult.o: exec_matmult.c
	$(CC) $(CFLAGS) -c exec_matmult.c
exec_matmult: exec_matmult.o start.o
	$(LD) $(LDFLAGS) start.o exec_matmult.o -o exec_matmult.coff
	../bin/coff2noff exec_matmult.coff exec_matmult

test_fork_matmult.o: test_fork_matmult.c
	$(CC) $(CFLAGS) -c test_fork_matmult.c
test_fork_matmult: test_fork_matmult.o start.o
	$(LD) $(LDFLAGS) start.o test_fork_matmult.o -o test_fork_matmult.coff
	../bin/coff2noff test_fork_matmult.coff test_fork_matmult

test_fork_sort.o: test_fork_sort.c
	$(CC) $(CFLAGS) -c test_fork_sort.c
test_fork_sort: test_fork_sort.o start.o
	$(LD) $(LDFLAGS) start.o test_fork_sort.o -o test_fork_sort.coff
	../bin/coff2noff test_fork_sort.coff test_fork_sort

exec_sort.o: exec_sort.c
	$(CC) $(CFLAGS) -c exec_sort.c
exec_sort: exec_sort.o start.o
	$(LD) $(LDFLAGS) start.o exec_sort.o -o exec_sort.coff
	../bin/coff2noff exec_sort.coff exec_sort

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

testmutex.o: testmutex.c
	$(CC) $(CFLAGS) -c testmutex.c
testmutex: testmutex.o start.o
	$(LD) $(LDFLAGS) start.o testmutex.o -o testmutex.coff
	../bin/coff2noff testmutex.coff testmutex

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
