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

int debug = 1;

static OS_EVENT *e;
static unsigned long ctr = 0;

void osdie(int err, int line)
{
	PRINT("DIE %d : %d\n", err, line);
	while(1){};
}

static OS_STK stack1[STACK_SIZE];
static void t1(void *p)
{
	int nr = 0;
	char *name = p;
	OS_CPU_SR cpu_sr = 0;
	INT8U err;

	OS_ENTER_CRITICAL();
	PRINT("******************************%s: %d\n",name, nr);
	OS_EXIT_CRITICAL();
	while (1) {
		OSSemPend(e, 0, &err);
		ctr++;
	}
}

static OS_STK stack2[STACK_SIZE];
static void t2(void *p)
{
	int nr = 0;
	char *name = p;
	OS_CPU_SR cpu_sr = 0;
	
	OS_ENTER_CRITICAL();
	PRINT("******************************%s: %d\n",name, nr);
	OS_EXIT_CRITICAL();
	while (1) {
		OSSemPost(e);
	}
}

static OS_STK stack0[STACK_SIZE];
static void t0(void *p)
{
	char *name = p;
	OS_CPU_SR cpu_sr = 0;

	/*
	 * This will roughly measure the number of context switch per second
	 * between t1 and t2
	 * atmega 328@16Mhz = 17143 per sec
	 * Intel(R) Xeon(R) W-2225 CPU @ 4.10GHz = 8269197 per sec
	 * 	Interestingly the math ( freq ratio ) adds up fine to a X2 factor
	 */
	while (1) {
		OS_ENTER_CRITICAL();
		PRINT("******************************%s: %ld\n",name, ctr);
		OS_EXIT_CRITICAL();
		OSTimeDlyHMSM(0,0,1,0);
	}
}

int main(void)
{
	OS_CPU_SR cpu_sr = 0;

	/* Don't let any IRQ come */
	OS_ENTER_CRITICAL();

	lib_init();
	PRINT("Entering main control loop\n");

	OSInit();

	e = OSSemCreate(0);

	OSTaskCreate(t0, "t0", &stack0[STK_HEAD(STACK_SIZE)], 1);
	OSTaskCreate(t1, "t1", &stack1[STK_HEAD(STACK_SIZE)], 2);
	OSTaskCreate(t2, "t2", &stack2[STK_HEAD(STACK_SIZE)], 3);

	/* IRQ are enabled when the first thread is started */
	OSStart();

	HANDLE_EXIT();
	OS_EXIT_CRITICAL();
	return 0;
}

