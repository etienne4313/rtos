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

void osdie(int err, int line)
{
	PRINT("DIE %d : %d\n", err, line);
	while(1){};
}

extern int rtos_dead;
static OS_STK stack1[STACK_SIZE];
static void t1(void *p)
{
	int nr = 0;
	char *name = p;
	OS_CPU_SR cpu_sr = 0;

	while (1) {
		OS_ENTER_CRITICAL();
		PRINT("******************************%s: %d\n",name, nr);
		OS_EXIT_CRITICAL();
		nr++;
		OSTimeDlyHMSM(0,0,5,0);
		rtos_dead = 1;
	}
}

static OS_STK stack2[STACK_SIZE];
static void t2(void *p)
{
	int nr = 0;
	char *name = p;
	OS_CPU_SR cpu_sr = 0;

	while (1) {
		OS_ENTER_CRITICAL();
		PRINT("##############################%s: %d\n",name, nr);
		OS_EXIT_CRITICAL();
		nr++;
		OSTimeDlyHMSM(0,0,1,0);
	}
}

static OS_STK stack3[STACK_SIZE];
static void t3(void *p)
{
	int nr = 0;
	char *name = p;
	OS_CPU_SR cpu_sr = 0;

	while (1) {
		OS_ENTER_CRITICAL();
		PRINT("##############################%s: %d\n",name, nr);
		OS_EXIT_CRITICAL();
		nr++;
		OSTimeDlyHMSM(0,0,1,0);
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

	OSTaskCreate(t1, "t1", (OS_STK *)&stack1[STK_HEAD(STACK_SIZE)], 4);
	OSTaskCreate(t2, "t2", (OS_STK *)&stack2[STK_HEAD(STACK_SIZE)], 5);
	OSTaskCreate(t3, "t3", (OS_STK *)&stack3[STK_HEAD(STACK_SIZE)], 1);

	/* IRQ are enabled when the first thread is started */
	OSStart();

	HANDLE_EXIT();
	OS_EXIT_CRITICAL();
	return 0;
}

