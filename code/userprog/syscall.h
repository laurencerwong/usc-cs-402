/* syscalls.h 
 * 	Nachos system call interface.  These are Nachos kernel operations
 * 	that can be invoked from user programs, by trapping to the kernel
 *	via the "syscall" instruction.
 *
 *	This file is included by user programs and by the Nachos kernel. 
 *
 * Copyright (c) 1992-1993 The Regents of the University of California.
 * All rights reserved.  See copyright.h for copyright notice and limitation 
 * of liability and disclaimer of warranty provisions.
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/* system call codes -- used by the stubs to tell the kernel which system call
 * is being asked for
 */
#define SC_Halt		0
#define SC_Exit		1
#define SC_Exec		2
#define SC_Join		3
#define SC_Create	4
#define SC_Open		5
#define SC_Read		6
#define SC_Write	7
#define SC_Close	8
#define SC_Fork		9
#define SC_Yield	10

/*#ifdef CHANGED*/

#define SC_CreateLock 11
#define SC_CreateCondition 12
#define SC_Acquire 13
#define SC_Release 14
#define SC_Signal 15
#define SC_Broadcast 16
#define SC_DestroyLock 17
#define SC_DestroyCondition 18
#define SC_Wait 19
#define SC_CreateSemaphore 20
#define SC_DestroySemaphore 21
#define SC_P 22
#define SC_V 23
#define SC_NPrint 24
#define SC_NEncode2to1 25
#define SC_CreateQueue 27
#define SC_DestroyQueue 28
#define SC_QueuePush 29
#define SC_QueueFront 30
#define SC_QueuePop 31
#define SC_QueueEmpty 32
#define SC_QueueSize 33
#define SC_NTime 34
#define SC_NRand 35
#define SC_NSrand 36
#define SC_ReadInt 37
#define SC_RandInt 38
#define SC_CreateMV 39
#define SC_DestroyMV 40
#define SC_SetMV 41
#define SC_GetMV 42

#define MAX_LOCKS_CONDITIONS 500




/*#endif*/

#define MAXFILENAME 256

#ifndef IN_ASM

/* The system call interface.  These are the operations the Nachos
 * kernel needs to support, to be able to run user programs.
 *
 * Each of these is invoked by a user program by simply calling the 
 * procedure; an assembly language stub stuffs the system call code
 * into a register, and traps to the kernel.  The kernel procedures
 * are then invoked in the Nachos kernel, after appropriate error checking, 
 * from the system call entry point in exception.cc.
 */

/* Stop Nachos, and print out performance stats */
void Halt();		
 

/* Address space control operations: Exit, Exec, and Join */

/* This user program is done (status = 0 means exited normally). */
void Exit(int status);	

/* A unique identifier for an executing user program (address space) */
typedef int SpaceId;	
 
/* Run the executable, stored in the Nachos file "name", and return the 
 * address space identifier
 */
SpaceId Exec(char *name, int length, char* threadName, int threadNameLength);
 
/* Only return once the the user program "id" has finished.  
 * Return the exit status.
 */
int Join(SpaceId id); 	
 

/* File system operations: Create, Open, Read, Write, Close
 * These functions are patterned after UNIX -- files represent
 * both files *and* hardware I/O devices.
 *
 * If this assignment is done before doing the file system assignment,
 * note that the Nachos file system has a stub implementation, which
 * will work for the purposes of testing out these routines.
 */
 
/* A unique identifier for an open Nachos file. */
typedef int OpenFileId;	

/* when an address space starts up, it has two open files, representing 
 * keyboard input and display output (in UNIX terms, stdin and stdout).
 * Read and Write can be used directly on these, without first opening
 * the console device.
 */

#define ConsoleInput	0  
#define ConsoleOutput	1  
 
/* Create a Nachos file, with "name" */
void Create(char *name, int size);

/* Open the Nachos file "name", and return an "OpenFileId" that can 
 * be used to read and write to the file.
 */
OpenFileId Open(char *name, int size);

/* Write "size" bytes from "buffer" to the open file. */
void Write(char *buffer, int size, OpenFileId id);

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int Read(char *buffer, int size, OpenFileId id);

/* Close the file, we're done reading and writing to it. */
void Close(OpenFileId id);



/* User-level thread operations: Fork and Yield.  To allow multiple
 * threads to run within a user program. 
 */

/* Fork a thread to run a procedure ("func") in the *same* address space 
 * as the current thread.
 */
void Fork(void (*func)(), char* name, int length);

/* Yield the CPU to another runnable thread, whether in this address space 
 * or not. 
 */
void Yield();		

#ifdef CHANGED

int CreateLock(char *, int);
int CreateCondition(char *, int);
void DestroyLock(int, int);
void DestroyCondition(int, int);
void Wait(int, int);
void Signal(int, int);
void Acquire(int);
void Release(int);
void Broadcast(int, int);
/*Semaphore stuff*/
int CreateSemaphore(char *, int, int);
void DestroySemaphore(int, int);
void V(int);
void P(int);

void NPrint(char*, int, int, int);
int NEncode2to1(int, int);
int ReadInt(char*, int);
int RandInt();

int CreateQueue();
void DestroyQueue(int);
void QueuePush(int, int);
int QueueFront(int);
void QueuePop(int);
int QueueEmpty(int);
int QueueSize(int);
int NRand();
time_t NTime();
void NSrand(int);

int CreateMV(char*, int, int, int);
void DestroyMV(int);
int GetMV(int, int);
void SetMV(int, int, int);


#endif /*CHAGNED */

#endif /* IN_ASM */

#endif /* SYSCALL_H */
