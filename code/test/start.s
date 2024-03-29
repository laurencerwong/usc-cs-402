/* Start.s 
 *	Assembly language assist for user programs running on top of Nachos.
 *
 *	Since we don't want to pull in the entire C library, we define
 *	what we need for a user program here, namely Start and the system
 *	calls.
 */

#define IN_ASM
#include "syscall.h"

        .text   
        .align  2

/* -------------------------------------------------------------
 * __start
 *	Initialize running a C program, by calling "main". 
 *
 * 	NOTE: This has to be first, so that it gets loaded at location 0.
 *	The Nachos kernel always starts a program by jumping to location 0.
 * -------------------------------------------------------------
 */

	.globl __start
	.ent	__start
__start:
	jal	main
	move	$4,$0		
	jal	Exit	 /* if we return from main, exit(0) */
	.end __start

/* -------------------------------------------------------------
 * System call stubs:
 *	Assembly language assist to make system calls to the Nachos kernel.
 *	There is one stub per system call, that places the code for the
 *	system call into register r2, and leaves the arguments to the
 *	system call alone (in other words, arg1 is in r4, arg2 is 
 *	in r5, arg3 is in r6, arg4 is in r7)
 *
 * 	The return value is in r2. This follows the standard C calling
 * 	convention on the MIPS.
 * -------------------------------------------------------------
 */

	.globl Halt
	.ent	Halt
Halt:
	addiu $2,$0,SC_Halt
	syscall
	j	$31
	.end Halt

	.globl Exit
	.ent	Exit
Exit:
	addiu $2,$0,SC_Exit
	syscall
	j	$31
	.end Exit

	.globl Exec
	.ent	Exec
Exec:
	addiu $2,$0,SC_Exec
	syscall
	j	$31
	.end Exec

	.globl Join
	.ent	Join
Join:
	addiu $2,$0,SC_Join
	syscall
	j	$31
	.end Join

	.globl Create
	.ent	Create
Create:
	addiu $2,$0,SC_Create
	syscall
	j	$31
	.end Create

	.globl Open
	.ent	Open
Open:
	addiu $2,$0,SC_Open
	syscall
	j	$31
	.end Open

	.globl Read
	.ent	Read
Read:
	addiu $2,$0,SC_Read
	syscall
	j	$31
	.end Read

	.globl Write
	.ent	Write
Write:
	addiu $2,$0,SC_Write
	syscall
	j	$31
	.end Write

	.globl Close
	.ent	Close
Close:
	addiu $2,$0,SC_Close
	syscall
	j	$31
	.end Close

	.globl Fork
	.ent	Fork
Fork:
	addiu $2,$0,SC_Fork
	syscall
	j	$31
	.end Fork

	.globl Yield
	.ent	Yield
Yield:
	addiu $2,$0,SC_Yield
	syscall
	j	$31
	.end Yield

	.globl CreateLock
	.ent	CreateLock

CreateLock:
	addiu $2,$0,SC_CreateLock
	syscall
	j	$31
	.end CreateLock

	.globl CreateCondition
	.ent	CreateCondition

CreateCondition:
	addiu $2,$0,SC_CreateCondition
	syscall
	j	$31
	.end CreateCondition

	.globl Acquire
	.ent	Acquire


Acquire:
	addiu $2,$0,SC_Acquire
	syscall
	j	$31
	.end Acquire

	.globl Release
	.ent	Release

Release:
	addiu $2,$0,SC_Release
	syscall
	j	$31
	.end Release

	.globl Broadcast
	.ent	Broadcast

Broadcast:
	addiu $2,$0,SC_Broadcast
	syscall
	j	$31
	.end Broadcast

	.globl DestroyLock
	.ent	DestroyLock

DestroyLock:
	addiu $2,$0,SC_DestroyLock
	syscall
	j	$31
	.end DestroyLock

	.globl DestroyCondition
	.ent	DestroyCondition

DestroyCondition:
	addiu $2,$0,SC_DestroyCondition
	syscall
	j	$31
	.end DestroyCondition

	.globl Signal
	.ent	Signal

Signal:
	addiu $2,$0,SC_Signal
	syscall
	j	$31
	.end Signal

	.globl CreateSemaphore
	.ent	CreateSemaphore

CreateSemaphore:
	addiu $2,$0,SC_CreateSemaphore
	syscall
	j	$31
	.end CreateSemaphore

	.globl DestroySemaphore
	.ent	DestroySemaphore

DestroySemaphore:
	addiu $2,$0,SC_DestroySemaphore
	syscall
	j	$31
	.end DestroySemaphore

	.globl V
	.ent	V

V:
	addiu $2,$0,SC_V
	syscall
	j	$31
	.end V

	.globl P
	.ent	P

P:
	addiu $2,$0,SC_P
	syscall
	j	$31
	.end P

	.globl NPrint
	.ent	NPrint

NPrint:
	addiu $2,$0,SC_NPrint
	syscall
	j	$31
	.end NPrint

	.globl NEncode2to1
	.ent	NEncode2to1

NEncode2to1:
	addiu $2,$0,SC_NEncode2to1
	syscall
	j	$31
	.end NEncode2to1

	.globl Wait
	.ent	Wait

Wait:
	addiu $2,$0,SC_Wait
	syscall
	j	$31
	.end Wait

	.globl CreateQueue
	.ent	CreateQueue

CreateQueue:
	addiu $2,$0,SC_CreateQueue
	syscall
	j	$31
	.end CreateQueue

	.globl DestroyQueue
	.ent	DestroyQueue

DestroyQueue:
	addiu $2,$0,SC_DestroyQueue
	syscall
	j	$31
	.end DestroyQueue

 	.globl QueuePush
	.ent	QueuePush

QueuePush:
	addiu $2,$0,SC_QueuePush
	syscall
	j	$31
	.end QueuePush

	.globl QueueFront
	.ent	QueueFront

QueueFront:
	addiu $2,$0,SC_QueueFront
	syscall
	j	$31
	.end QueueFront

	.globl QueuePop
	.ent	QueuePop

QueuePop:
	addiu $2,$0,SC_QueuePop
	syscall
	j	$31
	.end QueuePop

	.globl QueueEmpty
	.ent	QueueEmpty

QueueEmpty:
	addiu $2,$0,SC_QueueEmpty
	syscall
	j	$31
	.end QueueEmpty

	.globl QueueSize
	.ent	QueueSize

QueueSize:
	addiu $2,$0,SC_QueueSize
	syscall
	j	$31
	.end QueueSize

	.globl NTime
	.ent	NTime

NTime:
	addiu $2,$0,SC_NTime
	syscall
	j	$31
	.end NTime

	.globl NRand
	.ent	NRand

NRand:
	addiu $2,$0,SC_NRand
	syscall
	j	$31
	.end NRand

	.globl NSrand
	.ent	NSrand

NSrand:
	addiu $2,$0,SC_NSrand
	syscall
	j	$31
	.end NSrand
	

	.globl ReadInt
	.ent	ReadInt

ReadInt:
	addiu $2,$0,SC_ReadInt
	syscall
	j	$31
	.end ReadInt

	.globl RandInt
	.ent	RandInt

RandInt:
	addiu $2,$0,SC_RandInt
	syscall
	j	$31
	.end RandInt

	.globl CreateMV
	.ent	CreateMV

CreateMV:
	addiu $2,$0,SC_CreateMV
	syscall
	j	$31
	.end CreateMV

	.globl DestroyMV
	.ent	DestroyMV

DestroyMV:
	addiu $2,$0,SC_DestroyMV
	syscall
	j	$31
	.end DestroyMV

	.globl SetMV
	.ent	SetMV

SetMV:
	addiu $2,$0,SC_SetMV
	syscall
	j	$31
	.end SetMV

	.globl GetMV
	.ent	GetMV

GetMV:
	addiu $2,$0,SC_GetMV
	syscall
	j	$31
	.end GetMV

/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

