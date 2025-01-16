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

/*
 * The RTOS is running in polling mode e.g. no interrupts. This approach gives a big
 * portability advantage since there is no need to hook in the IRQ controller
 *
 * Most of the architecture support the concept of free running monotonic counter which is
 * function of the CPU frequency. On X86 there is the TSC counter and on ARM there is
 * arch_timer_read_counter(). In user-space similar behavior can be obtained via
 * clock_gettime(CLOCK_MONOTONIC, &tp);
 *
 * The monotonic timebase is calibrated at intialization time. The output from calibration
 * provides "cycle_per_os_tick" which correspond to the number of CPU cycle within a timer
 * period
 *
 * UcosII preemption model EX: TimerIRQ
 * <<< IRQ handle
 * OSIntEnter();
 * <<< Do stuff
 * OSTimeTick(); <<< Adjust RDY tasks
 *  OSIntExit();
 *   OS_SchedNew(); <<< Find high ready
 *   OSIntCtxSw(); <<< Context switch
 * 
 * Limitation / work-around:
 *  A) We need to call OSTimeTick manually every so often to maintain the timebase. There is
 *     no real preemption
 * 	B) Low priority task running spinloop will block all timestamp and scheduling operation
 * 		- Easy to catch during code inspection
 * 	B) All Tasks going to sleep; Here the idle task will run in tight loop doing
 * 		OS_ENTER_CRITICAL / OS_EXIT_CRITICAL
 * 	C) Low priority tasks doing ping-pong sem wait/pend while another High Priority task gets ready
 * 		- Here the scheduler OS_Sched() is involved for every context switch
 *
 * Implementation:
 * OS_EXIT_CRITICAL is the perfect point for polling. Care must be take to avoid re-rentrancy
 * since some of the API are also relying on OS_EXIT_CRITICAL
 *
 */
void exit_critical(void)
{
	struct linux_task_struct dummy;
	static int in_critical = 0;
	u64 t;

	if(in_critical)
		return;

	in_critical = 1;

	if(rtos_dead){
		linux_switch_to((void*)1, &dummy, &original_context);
		/* NO return */
		DIE(-1); /* Never reach */
	}

	t = get_monotonic_cycle();

	poll_timer(t);

	tick = t;

	if( (t - prev) > cycle_per_os_tick){
		prev = t;
		OSIntEnter();
//		PRINT("###");
		OSTimeTick();
		OSIntExit(); /* OSIntCtxSw to Task ready to run */
	}

	in_critical = 0;
}
