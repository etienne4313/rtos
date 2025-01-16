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

#define REPEAT 100

int debug = 1;

static OS_EVENT *a, *b;
static cpu_cycle_t T1;

void osdie(int err, int line)
{
	PRINT("DIE %d : %d\n", err, line);
	while(1){};
}

static OS_STK stack1[STACK_SIZE];

/*
 * Results from mega328@16Mhz / 62.5nsec per cycle
 * Monotonic time rely on Timer#1 with 1 count per cycle
 */
static void t1(void *p)
{
	int x;
	INT8U err;
	cpu_cycle_t t1, t2;
	OS_CPU_SR cpu_sr = 0;

	while(1){

		/*
		 * Compare busy loop against monotonic time when IRQ are disabled
		 * => 8039
		 * Above 8039 correspond to 502.43 usec
		 */
		PRINT("\nT1\n");
		for(x=0; x<REPEAT; x++){
			OS_ENTER_CRITICAL();
			t1 = get_monotonic_cycle();
			DELAY_USEC(500);
			t2 = get_monotonic_cycle();
			OS_EXIT_CRITICAL();
			PRINT("%ld ", (unsigned long)(t2-t1));
		}

		/*
		 * Compare busy loop against monotonic time when IRQ are enabled
		 * => 8039 8420 8039 8039 8039 8039 8420
		 * Here we are getting preempted by the Timer IRQ every so often which is taking 381 cycle total
		 */
		PRINT("\nT2\n");
		for(x=0; x<REPEAT; x++){
			t1 = get_monotonic_cycle();
			DELAY_USEC(500);
			t2 = get_monotonic_cycle();
			PRINT("%ld ", (unsigned long)(t2-t1));
		}

		/*
		 * Let's isolate the context Save/Restore aspect which is present in the Timer IRQ
		 * => 194
		 * From T2 we know that the Timer IRQ takes a total of 381 cycle which include context Save/Restore
		 * hence the RTOS scheduler invocation takes 381 - 194 = 187 cycle
		 */
		PRINT("\nT3\n");
		for(x=0; x<REPEAT; x++){
			OS_ENTER_CRITICAL();
			t1 = get_monotonic_cycle();
			portSAVE_CONTEXT();
			portRESTORE_CONTEXT();
			t2 = get_monotonic_cycle();
			OS_EXIT_CRITICAL();
			PRINT("%ld ", (unsigned long)(t2-t1));
		}

		/*
		 * Measure semaphore wakeup time
		 * => 524 ( 32.75 usec )
		 *
		 * Thread T1:
		 * SemPend Code, OS schedule, Save Context T1, Restore Context -> T2
		 *
		 * Thread T2:
		 * (Time T1) SemPost Code, OS schedule, Save context, Restore Context -> T1 ( Time t2 )
		 *
		 * 1X context Save/Restore : 194
		 * 1X RTOS scheduler: 187
		 * => Semaphore code = 524 - 194 - 187 = 143
		 */
		PRINT("\nT4\n");
		for(x=0; x<REPEAT; x++){
			OSSemPend(a, 0, &err);
			t2 = get_monotonic_cycle();
			PRINT("%ld ", (unsigned long)(t2-T1));
		}

		/*
		 * Measure semaphore post time
		 * => 86
		 */
		PRINT("\nT5\n");
		for(x=0; x<REPEAT; x++){
			OS_ENTER_CRITICAL();
			t1 = get_monotonic_cycle();
			OSSemPost(b);
			t2 = get_monotonic_cycle();
			OS_EXIT_CRITICAL();
			PRINT("%ld ", (unsigned long)(t2-t1));
		}

		/*
		 * Measure semaphore pend time
		 * => 117
		 * Which is close enough to Test T4
		 */
		PRINT("\nT6\n");
		for(x=0; x<REPEAT; x++){
			OS_ENTER_CRITICAL();
			t1 = get_monotonic_cycle();
			OSSemPend(b, 0, &err);
			t2 = get_monotonic_cycle();
			OS_EXIT_CRITICAL();
			PRINT("%ld ", (unsigned long)(t2-t1));
		}
	}
}

static OS_STK stack2[STACK_SIZE];
static void t2(void *p)
{
	while (1) {
		T1 = get_monotonic_cycle();
		OSSemPost(a);
	}
}

int main(void)
{
	OS_CPU_SR cpu_sr = 0;

	/* Don't let any IRQ come */
	OS_ENTER_CRITICAL();

	lib_init();
	PRINT("Entering main loop\n");

	OSInit();

	a = OSSemCreate(0);
	b = OSSemCreate(0);
	OSTaskCreate(t1, "t1", &stack1[STK_HEAD(STACK_SIZE)], 1);
	OSTaskCreate(t2, "t2", &stack2[STK_HEAD(STACK_SIZE)], 2);

	/* IRQ are enabled when the first thread is started */
	OSStart();

	HANDLE_EXIT();
	OS_EXIT_CRITICAL();
	return 0;
}

