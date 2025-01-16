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

void OSTaskCreateHook(OS_TCB *ptcb){}
void OSTaskDelHook (OS_TCB *ptcb){}
void OSTaskSwHook (void){}
void OSTaskStatHook (void){}
void OSTaskReturnHook(OS_TCB *ptcb){}
void OSSystemReset(void){}
void OSTCBInitHook(OS_TCB *ptcb){}
void OSTimeTickHook (void){}
void OSInitHookBegin(void){}
void OSTaskIdleHook(void){}
void OSInitHookEnd(void){}

int rtos_dead;

#define portFLAGS_INT_ENABLED           ( (OS_STK) 0x80 )

OS_STK* OSTaskStkInit (void (*task)(void* pd), void* pdata, OS_STK* pxTopOfStack, INT16U opt)
{
	uint16_t usAddress;
	//	fprintf(stderr,"PTOS %x\n", (unsigned int)pxTopOfStack);

	/* The start of the task code will be popped off the stack last, so place
	   it on first. */
	usAddress = ( uint16_t ) task;
	*pxTopOfStack = ( OS_STK ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	usAddress >>= 8;
	*pxTopOfStack = ( OS_STK ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

#if defined(__AVR_3_BYTE_PC__)
	/* The AVR ATmega2560/ATmega2561 have 256KBytes of program memory and a 17-bit
	 * program counter. When a code address is stored on the stack, it takes 3 bytes
	 * instead of 2 for the other ATmega* chips.
	 *
	 * Store 0 as the top byte since we force all task routines to the bottom 128K
	 * of flash. We do this by using the .lowtext label in the linker script. TODO
	 *
	 * In order to do this properly, we would need to get a full 3-byte pointer to
	 * pxCode. That requires a change to GCC. Not likely to happen any time soon.
	 */
	*pxTopOfStack = 0;
	pxTopOfStack--;
#endif

	/* Next simulate the stack as if after a call to portSAVE_CONTEXT().
	   portSAVE_CONTEXT places the flags on the stack immediately after r0
	   to ensure the interrupts get disabled as soon as possible, and so ensuring
	   the stack use is minimal should a context switch interrupt occur. */
	*pxTopOfStack = ( OS_STK ) 0x00;    /* R0 */
	pxTopOfStack--;
	*pxTopOfStack = portFLAGS_INT_ENABLED;
	pxTopOfStack--;

#if defined(__AVR_3_BYTE_PC__)
	/* If we have an ATmega256x, we are also saving the EIND register.
	 * We should default to 0.
	 */
	*pxTopOfStack = ( OS_STK ) 0x00;    /* EIND */
	pxTopOfStack--;
#endif

#if defined(__AVR_HAVE_RAMPZ__)
	/* We are saving the RAMPZ register.
	 * We should default to 0.
	 */
	*pxTopOfStack = ( OS_STK ) 0x00;    /* RAMPZ */
	pxTopOfStack--;
#endif

	/* Now the remaining registers. The compiler expects R1 to be 0. */
	*pxTopOfStack = ( OS_STK ) 0x00;    /* R1 */

	/* Leave R2 - R23 untouched */
	pxTopOfStack -= 23;

	/* Place the parameter on the stack in the expected location. */
	usAddress = ( uint16_t ) pdata;
	*pxTopOfStack = ( OS_STK ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	usAddress >>= 8;
	*pxTopOfStack = ( OS_STK ) ( usAddress & ( uint16_t ) 0x00ff );

	/* Leave register R26 - R31 untouched */
	pxTopOfStack -= 7;

	return pxTopOfStack;
}

/*
 * 	OSStartHighRdy():
 * 		OSStart() calls OSStartHighRdy() with IRQ Disabled
 * 		The frame is set by OSTaskStkInit().
 * 			In theory we could set the frame with portSAVE_CONTEXT() so that we could jump back after OSStart()
 * 		The task is started with portRESTORE_CONTEXT() which is enabling IRQ for the first task
 */
void OSStartHighRdy(void)
{
	OSTaskSwHook();
	OSRunning = OS_TRUE;
	portRESTORE_CONTEXT();
	__asm__ __volatile__ ( "ret" );
	/* NO return */
}

/*
 * IRQ handler construct:
 *
 *  ISR_NAKED ISR(TIMER0_OVF_vect){
 * 		portSAVE_CONTEXT()
 * 	  	OSIntEnter() nesting++
 *			do_something();
 * 	  	OSIntExit() -> OSIntCtxSw // SET OSTCBCur and OSPrioCur
 * 		portRESTORE_CONTEXT()
 * 		__asm__ __volatile__ ( "reti" );
 * 	}
 *
 * NOTE: portSAVE_CONTEXT() has SREG IRQ disabled ( because we are in IRQ ) SO if we restore 
 * 	that context from a regular task switch "OS_TASK_SW" then the task will have IRQ disabled.
 *
 * Two options:
 * a) portSAVE_CONTEXT() has special hook that store a SREG with IRQ enabled
 * 		- some register mangling required...
 * 		- During portRESTORE_CONTEXT, SREG will be restored with IRQ enabled _before_ the reti
 * b) Do a reti from OS_TASK_SW()
 * 		- The code calling OS_TASK_SW() from OS_Sched() always re-enabled IRQ right after so should be OK...
 * 		- Assuming that task_switch comes from an IRQ enabled code path
 */
void OSIntCtxSw(void)
{
	/* MUST Set current context to highest priority */
	OSTCBCur = OSTCBHighRdy;
	OSPrioCur = OSPrioHighRdy;
	/* Return to IRQ handler */
}

/* OS_Sched -> OS_TASK_SW */
void OS_TASK_SW()
{
	portSAVE_CONTEXT();
	OSTaskSwHook();

	/* MUST Set current context to highest priority */
	OSTCBCur = OSTCBHighRdy;
	OSPrioCur = OSPrioHighRdy;

	portRESTORE_CONTEXT();
	__asm__ __volatile__ ( "reti" );
	/* NO return */
}

