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
#ifndef __LIB__H__
#define __LIB__H__

#define DIE(error_code) do {\
	osdie(error_code, __LINE__); \
} while(0)

#define DEBUG(format, ...) do {\
	if(debug) \
		PRINTF(format, ## __VA_ARGS__);\
} while(0)

#define PRINT(format, ...) do {\
	PRINTF(format, ## __VA_ARGS__);\
} while(0)

#define FORCE_PRINT(format, ...) do {\
	PRINTF(format, ## __VA_ARGS__);\
} while(0)

extern int debug;
extern void osdie(int err, int line);

enum lib_error_condition{
	TIMER = -15,
	IRQ = -14,
	FATAL = -13,
};

/******************************************************************************/
/* Time */
/******************************************************************************/
#ifndef __KERNEL__
#define NSEC_PER_SEC 1000000000ULL
#define USEC_PER_SEC 1000000UL
#define USEC_PER_MSEC 1000UL
#endif

typedef void(*work_t)(int arg, unsigned long usec_time);

struct w{
	work_t callback;
	int arg;
	cpu_cycle_t time;
	struct w *next;
};

void timer_init(void);
cpu_cycle_t get_monotonic_cycle(void);
unsigned long get_monotonic_time(void);

/* Schedule work to happen at a specific absolute timestamp in usec */
void schedule_work_absolute(work_t s1, int arg, unsigned long timestamp);

void timer_isr(void);
void poll_timer_wheel(cpu_cycle_t t);
void poll(void);

/******************************************************************************/
/* Library initialization */
/******************************************************************************/
static inline void lib_init(void)
{
	uart_init();
	PRINT("Lib init\n");
	timer_init();
}

/******************************************************************************/
/* Math */
/******************************************************************************/
unsigned int divu10(unsigned int n);

#endif
