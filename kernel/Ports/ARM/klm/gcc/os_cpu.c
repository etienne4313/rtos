/*
 * Copyright 2024, Etienne Martineau etienne4313@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <ucos_ii.h>
#include "context.h"

void OSTaskCreateHook(OS_TCB *ptcb){}
void OSTaskDelHook (OS_TCB *ptcb){}
void OSTaskStatHook (void){}
void OSTaskReturnHook(OS_TCB *ptcb){}
void OSTCBInitHook(OS_TCB *ptcb){}
void OSTimeTickHook (void){}
void OSInitHookEnd(void){}
void OSTaskIdleHook(void){}
void OSInitHookBegin(void){}

int rtos_dead;
static struct linux_task_struct original_context;

OS_STK* OSTaskStkInit (void (*task)(void* pd), void* pdata, OS_STK* ptos, INT16U opt)
{
	/* 
	 * [ 0 --------------------------- ptos ]
	 * [ ****** Stack ********[ task_struct ]
	 *                        ^t
	 *         cpu_context.sp^  
	 */
	struct linux_task_struct *t = (struct linux_task_struct *) &ptos[ -sizeof(struct linux_task_struct) ];
	struct linux_thread_info *ti = info(t); 

	ti->cpu_context.arg = pdata;
	ti->cpu_context.r4 = 0x4;
	ti->cpu_context.r5 = 0x5;
	ti->cpu_context.r6 = 0x6;
	ti->cpu_context.r7 = 0x7;
	ti->cpu_context.r8 = 0x8;
	ti->cpu_context.r9 = 0x9;
	ti->cpu_context.sl = 0x10;
	ti->cpu_context.fp = (unsigned long)&(((OS_STK*)t)[-1]); /* Fake some room on the FP TODO */
	ti->cpu_context.sp = (unsigned long)&(((OS_STK*)t)[-8]);
	ti->cpu_context.pc = (unsigned long)task;

	return (OS_STK*)t;
}

/* OSStart -> OSStartHighRdy  */
void OSStartHighRdy(void)
{
	struct linux_task_struct *new;

	OSRunning = OS_TRUE;

	/* OSTCBCur == OSTCBHighRdy; OSPrioCur == OSPrioHighRdy; */
	new = (struct linux_task_struct *)OSTCBHighRdy->OSTCBStkPtr;

	linux_switch_to(new->thread_info.cpu_context.arg, &original_context, new);
	/* NO return */
}

/* OSIntEnter / OSIntExit -> OSIntCtxSw */
void OSIntCtxSw(void)
{
	struct linux_task_struct *new, *prev;
	
	prev = (struct linux_task_struct *)OSTCBCur->OSTCBStkPtr;
	new = (struct linux_task_struct *)OSTCBHighRdy->OSTCBStkPtr;

	/* MUST Set current context to highest priority */
	OSTCBCur = OSTCBHighRdy;
	OSPrioCur = OSPrioHighRdy;

	linux_switch_to(new->thread_info.cpu_context.arg, prev, new);
	/* NO return */
}

/* OS_Sched -> OS_TASK_SW */
void OS_TASK_SW()
{
	OSIntCtxSw();
}

void rollback(void)
{
	struct linux_task_struct dummy;
	linux_switch_to((void*)1, &dummy, &original_context);
}

