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

extern void __switch_to_asm(OS_STK *next, OS_STK **prev);
extern void __start_to_asm(OS_STK *next, OS_STK **prev);
extern void __set_stack(OS_STK *next);

int rtos_dead;
static OS_STK *original_stack; /* Hold the original stack just before starting the first thread */

void OSTaskCreateHook(OS_TCB *ptcb){}
void OSTaskDelHook (OS_TCB *ptcb){}
void OSTaskStatHook (void){}
void OSTaskReturnHook(OS_TCB *ptcb){}
void OSTCBInitHook(OS_TCB *ptcb){}
void OSTimeTickHook (void){}
void OSInitHookEnd(void){}
void OSTaskIdleHook(void){}
void OSInitHookBegin(void){}

OS_STK* OSTaskStkInit (void (*task)(void* pd), void* pdata, OS_STK* ptos, INT16U opt)
{
	*ptos-- = (OS_STK)task; // RIP
	*ptos-- = 0;    // R8
	*ptos-- = 0;    // R9
	*ptos-- = 0;    // R10
	*ptos-- = 0;    // R11
	*ptos-- = 0;    // R12
	*ptos-- = 0;    // R13
	*ptos-- = 0;    // R14
	*ptos-- = 0;    // R15
	*ptos-- = 0;    // RAX
	*ptos-- = 0;    // RBX
	*ptos-- = 0;    // RCX
	*ptos-- = 0;    // RDX
	*ptos-- = 0;    // RSI
	*ptos-- = (unsigned long)pdata;   // RDI
	*ptos = 0;  	// RBP
//	DEBUG("PTOS %lx\n", (unsigned long)ptos);
	return ptos;
}

/* OSStart -> OSStartHighRdy  */
void OSStartHighRdy(void)
{
	OSRunning = OS_TRUE;

	/* 
	 * OSTCBCur == OSTCBHighRdy; OSPrioCur == OSPrioHighRdy;
	 *
	 * This function will return back to the caller 'OSStart' _only_
	 * if some thread calls into EXIT()
	 */
	__start_to_asm(OSTCBHighRdy->OSTCBStkPtr, &original_stack);
}

/* OSIntEnter / OSIntExit -> OSIntCtxSw */
void OSIntCtxSw(void)
{
	OS_TCB *prev = OSTCBCur; /* Remember current TCB */
	
	/* MUST Set current context to highest priority */
	OSTCBCur = OSTCBHighRdy;
	OSPrioCur = OSPrioHighRdy;
	__switch_to_asm(OSTCBHighRdy->OSTCBStkPtr, &prev->OSTCBStkPtr);
}

/* OS_Sched -> OS_TASK_SW */
void OS_TASK_SW()
{
	OSIntCtxSw();
}

void rollback(void)
{
	__set_stack(original_stack);
}
