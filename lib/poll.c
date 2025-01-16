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
#if OS_POLLING_EN
void poll(void)
{
	static int in_critical = 0;
	u64 t;

	if(in_critical)
		return;

	in_critical = 1;

	/* Rtos exit hook for KLM */
	if(rtos_dead){
		rollback();
		/* NO return */
		DIE(-1); /* Never reach */
	}

	t = get_monotonic_cycle();

	/* Tick the timer wheel */
	poll_timer_wheel(t);

	tick = t;

	/* OS time stamp */
	if( (t - prev) > cycle_per_os_tick){
		prev = t;
		OSIntEnter();
//		PRINT("###");
		OSTimeTick();
		OSIntExit(); /* OSIntCtxSw to Task ready to run */
	}

	in_critical = 0;
}
#endif
