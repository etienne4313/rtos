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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/kthread.h>
#include <asm/msr.h>

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                            */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                            */
typedef unsigned int   INT16U;                   /* Unsigned 16 bit quantity                            */
typedef signed   int   INT16S;                   /* Signed   16 bit quantity                            */
typedef unsigned long  INT32U;                   /* Unsigned 32 bit quantity                            */
typedef signed   long  INT32S;                   /* Signed   32 bit quantity                            */
typedef unsigned long long u64;
typedef unsigned long long cpu_cycle_t;

typedef unsigned long  OS_STK;                   /* Each stack entry is 32-bit wide                     */
typedef unsigned long OS_CPU_SR;

#include "platform_lib.h"
#include <lib.h>

#define	OS_STK_GROWTH  1u                        /* Stack grows from HIGH to LOW                        */

#define STACK_SIZE 1024
#define STK_HEAD(size) (size - sizeof(OS_STK))

#define  OS_CRITICAL_METHOD    3 /* Save IRQ flag in cpu_sr */

#define	OS_ENTER_CRITICAL() { cpu_sr = 0; }
#define	OS_EXIT_CRITICAL() do {\
	cpu_sr = cpu_sr ; \
	exit_critical(); \
} while(0)

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

