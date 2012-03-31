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
	
	.globl CreateCV
	.ent	CreateCV
CreateCV:
	addiu $2,$0,SC_CreateCV
	syscall
	j	$31
	.end CreateCV
	
	.globl CreateMV
	.ent	CreateMV
CreateMV:
	addiu $2,$0,SC_CreateMV
	syscall
	j	$31
	.end CreateMV
	
	.globl DeleteMV
	.ent	DeleteMV
DeleteMV:
	addiu $2,$0,SC_DeleteMV
	syscall
	j	$31
	.end DeleteMV
	
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
	
	.globl Wait
	.ent	Wait
Wait:
	addiu $2,$0,SC_Wait
	syscall
	j	$31
	.end Wait

	.globl Signal
	.ent	Signal
Signal:
	addiu $2,$0,SC_Signal
	syscall
	j	$31
	.end Signal

	.globl Broadcast
	.ent	Broadcast
Broadcast:
	addiu $2,$0,SC_Broadcast
	syscall
	j	$31
	.end Broadcast

	.globl DeleteLock
	.ent	DeleteLock
DeleteLock:
	addiu $2,$0,SC_DeleteLock
	syscall
	j	$31
	.end DeleteLock

	.globl DeleteCV
	.ent	DeleteCV
DeleteCV:
	addiu $2,$0,SC_DeleteCV
	syscall
	j	$31
	.end DeleteCV	
	
	.globl WriteToConsole
	.ent	WriteToConsole
WriteToConsole:
	addiu $2,$0,SC_WriteToConsole
	syscall
	j	$31
	.end WriteToConsole
	
	
	.globl Down
	.ent	Down
Down:
	addiu $2,$0,SC_Down
	syscall
	j	$31
	.end Down
	
	.globl Up
	.ent	Up
Up:
	addiu $2,$0,SC_Up
	syscall
	j	$31
	.end Up

	.globl CreateSem
	.ent	CreateSem
CreateSem:
	addiu $2,$0,SC_CreateSem
	syscall
	j	$31
	.end CreateSem
	
	.globl DeleteSem
	.ent	DeleteSem
DeleteSem:
	addiu $2,$0,SC_DeleteSem
	syscall
	j	$31
	.end DeleteSem
	
	.globl Random
	.ent	Random
Random:
	addiu $2,$0,SC_Random
	syscall
	j	$31
	.end Random

	.globl DisplayBitMap
	.ent	DisplayBitMap
DisplayBitMap:
	addiu $2,$0,SC_DisplayBitMap
	syscall
	j	$31
	.end DisplayBitMap
	
	.globl ReadInput
	.ent	ReadInput
ReadInput:
	addiu $2,$0,SC_ReadInput
	syscall
	j	$31
	.end ReadInput

	.globl CustomString
	.ent	CustomString
CustomString:
	addiu $2,$0,SC_CustomString
	syscall
	j	$31
	.end CustomString
	
	/* dummy function to keep gcc happy */
        .globl  __main
        .ent    __main
__main:
        j       $31
        .end    __main

