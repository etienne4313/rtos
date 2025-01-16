/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                    Copyright 1992-2021 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*
* Filename : os_cpu.h
* Version  : V2.93.01
*********************************************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
 * AVR gcc: int = 16 bit, long long = 64 bit, void * = 16 bit 
 */
typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                            */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                            */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                            */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                            */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                            */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                            */
typedef unsigned long  cpu_cycle_t;

typedef unsigned char  OS_STK;                   /* Each stack entry is 8-bit wide                      */
typedef unsigned char  OS_CPU_SR;                /* Define size of CPU status register (PSW = 8 bits)   */

#include "platform_lib.h"
#include <lib.h>

#define	OS_STK_GROWTH  1u                        /* Stack grows from HIGH to LOW memory on AVR          */

#define STACK_SIZE 128 
#define STK_HEAD(size) (size - 1u)

#define  OS_CRITICAL_METHOD   3 
                                                                                                                                                    
#if      OS_CRITICAL_METHOD == 1
#define  OS_ENTER_CRITICAL()    cli()                    /* Disable interrupts                        */
#define  OS_EXIT_CRITICAL()     sei()                    /* Enable  interrupts                        */
#endif

#if      OS_CRITICAL_METHOD == 3                                                                                                                    
/*
 * Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
 * would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
 * disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
 * disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
 * into the CPU's status register.
 */
#define  OS_ENTER_CRITICAL() do{ \
	cpu_sr = SREG;\
	cli(); \
} while(0)

#define OS_EXIT_CRITICAL() do { \
	SREG = cpu_sr; \
} while(0)
#endif

/*
 *
 * FreeRTOS Kernel V10.5.1+
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * portSAVE_CONTEXT() and portRESTORE_CONTEXT comes from FreeRTOS for AVR
 *
 * Macro to save all the general purpose registers, the save the stack pointer
 * into the TCB.
 *
 * The first thing we do is save the flags then disable interrupts. This is to
 * guard our stack against having a context switch interrupt after we have already
 * pushed the registers onto the stack - causing the 32 registers to be on the
 * stack twice.
 *
 * r1 is set to zero (__zero_reg__) as the compiler expects it to be thus, however
 * some of the math routines make use of R1.
 *
 * r0 is set to __tmp_reg__ as the compiler expects it to be thus.
 *
 * #if defined(__AVR_HAVE_RAMPZ__)
 * #define __RAMPZ__ 0x3B
 * #endif
 *
 * #if defined(__AVR_3_BYTE_PC__)
 * #define __EIND__ 0x3C
 * #endif
 *
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer.
 *
 * OSTCBStkPtr is the first element in typedef struct os_tcb so No need to index it
 */

#if defined(__AVR_3_BYTE_PC__) && defined(__AVR_HAVE_RAMPZ__)
#define AVR3BYTEa  "in     __tmp_reg__, 0x3B                       \n\t"
#define AVR3BYTEb  "push   __tmp_reg__                             \n\t"
#define AVR3BYTEc  "in     __tmp_reg__, 0x3C                       \n\t"
#define AVR3BYTEd  "push   __tmp_reg__                             \n\t"   
#else
#define AVR3BYTEa
#define AVR3BYTEb
#define AVR3BYTEc
#define AVR3BYTEd
#endif
#define portSAVE_CONTEXT()                                                              \
        __asm__ __volatile__ (  "push   __tmp_reg__                             \n\t"   \
                                "in     __tmp_reg__, __SREG__                   \n\t"   \
                                "push   __tmp_reg__                             \n\t"   \
								AVR3BYTEa \
								AVR3BYTEb \
								AVR3BYTEc \
								AVR3BYTEd \
                                "push   __zero_reg__                            \n\t"   \
                                "clr    __zero_reg__                            \n\t"   \
                                "push   r2                                      \n\t"   \
                                "push   r3                                      \n\t"   \
                                "push   r4                                      \n\t"   \
                                "push   r5                                      \n\t"   \
                                "push   r6                                      \n\t"   \
                                "push   r7                                      \n\t"   \
                                "push   r8                                      \n\t"   \
                                "push   r9                                      \n\t"   \
                                "push   r10                                     \n\t"   \
                                "push   r11                                     \n\t"   \
                                "push   r12                                     \n\t"   \
                                "push   r13                                     \n\t"   \
                                "push   r14                                     \n\t"   \
                                "push   r15                                     \n\t"   \
                                "push   r16                                     \n\t"   \
                                "push   r17                                     \n\t"   \
                                "push   r18                                     \n\t"   \
                                "push   r19                                     \n\t"   \
                                "push   r20                                     \n\t"   \
                                "push   r21                                     \n\t"   \
                                "push   r22                                     \n\t"   \
                                "push   r23                                     \n\t"   \
                                "push   r24                                     \n\t"   \
                                "push   r25                                     \n\t"   \
                                "push   r26                                     \n\t"   \
                                "push   r27                                     \n\t"   \
                                "push   r28                                     \n\t"   \
                                "push   r29                                     \n\t"   \
                                "push   r30                                     \n\t"   \
                                "push   r31                                     \n\t"   \
                                "lds    r26, OSTCBCur                           \n\t"   \
                                "lds    r27, OSTCBCur + 1                       \n\t"   \
								"in     __tmp_reg__, __SP_L__                   \n\t"   \
								"st     x+, __tmp_reg__                         \n\t"   \
								"in     __tmp_reg__, __SP_H__                   \n\t"   \
								"st     x+, __tmp_reg__                         \n\t"   \
                             );

#if defined(__AVR_3_BYTE_PC__) && defined(__AVR_HAVE_RAMPZ__)
#define PAVR3BYTEa  "out    0x3C, __tmp_reg__                       \n\t"
#define PAVR3BYTEb  "pop    __tmp_reg__                             \n\t"
#define PAVR3BYTEc  "out    0x3B, __tmp_reg__                       \n\t"
#define PAVR3BYTEd  "pop    __tmp_reg__                             \n\t"
#else
#define PAVR3BYTEa
#define PAVR3BYTEb
#define PAVR3BYTEc
#define PAVR3BYTEd
#endif

#define portRESTORE_CONTEXT()                                                           \
        __asm__ __volatile__ (  "lds    r26, OSTCBCur                           \n\t"   \
                                "lds    r27, OSTCBCur + 1                       \n\t"   \
                                "ld     r28, x+                                 \n\t"   \
                                "out    __SP_L__, r28                           \n\t"   \
                                "ld     r29, x+                                 \n\t"   \
                                "out    __SP_H__, r29                           \n\t"   \
                                "pop    r31                                     \n\t"   \
                                "pop    r30                                     \n\t"   \
                                "pop    r29                                     \n\t"   \
                                "pop    r28                                     \n\t"   \
                                "pop    r27                                     \n\t"   \
                                "pop    r26                                     \n\t"   \
                                "pop    r25                                     \n\t"   \
                                "pop    r24                                     \n\t"   \
                                "pop    r23                                     \n\t"   \
                                "pop    r22                                     \n\t"   \
                                "pop    r21                                     \n\t"   \
                                "pop    r20                                     \n\t"   \
                                "pop    r19                                     \n\t"   \
                                "pop    r18                                     \n\t"   \
                                "pop    r17                                     \n\t"   \
                                "pop    r16                                     \n\t"   \
                                "pop    r15                                     \n\t"   \
                                "pop    r14                                     \n\t"   \
                                "pop    r13                                     \n\t"   \
                                "pop    r12                                     \n\t"   \
                                "pop    r11                                     \n\t"   \
                                "pop    r10                                     \n\t"   \
                                "pop    r9                                      \n\t"   \
                                "pop    r8                                      \n\t"   \
                                "pop    r7                                      \n\t"   \
                                "pop    r6                                      \n\t"   \
                                "pop    r5                                      \n\t"   \
                                "pop    r4                                      \n\t"   \
                                "pop    r3                                      \n\t"   \
                                "pop    r2                                      \n\t"   \
                                "pop    __zero_reg__                            \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
								PAVR3BYTEa \
								PAVR3BYTEb \
								PAVR3BYTEc \
								PAVR3BYTEd \
                                "out    __SREG__, __tmp_reg__                   \n\t"   \
                                "pop    __tmp_reg__                             \n\t"   \
                             );
                                
/********************************************************************************************************
 *                                START HIGHEST PRIORITY TASK READY-TO-RUN
 *
 * Description : This function is called by OSStart() to start the highest priority task that was created
 * by your application before calling OSStart().
 */
void OSStartHighRdy(void);

/********************************************************************************************************
 *                                       TASK LEVEL CONTEXT SWITCH
 *
 * Description : This function is called when a task makes a higher priority task ready-to-run.
 *
 * Note(s)     : 1) Upon entry,
 *                  OSTCBCur     points to the OS_TCB of the task to suspend
 *                  OSTCBHighRdy points to the OS_TCB of the task to resume
 */
void OS_TASK_SW(void);

/*********************************************************************************************************
 *                                INTERRUPT LEVEL CONTEXT SWITCH
 *
 * Description : This function is called by OSIntExit() to perform a context switch to a task that has
 *               been made ready-to-run by an ISR.
 *
 * Note(s)     : 1) Upon entry,
 *                  OSTCBCur     points to the OS_TCB of the task to suspend
 *                  OSTCBHighRdy points to the OS_TCB of the task to resume
 */
void OSIntCtxSw(void);

