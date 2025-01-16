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
#ifndef __PLATFORM_LIB__H__
#define __PLATFORM_LIB__H__

#define PRINTF printf

/******************************************************************************/
/* Time */
/******************************************************************************/
#define DELAY_USEC(d) usleep(d)
#define DELAY_MSEC(d) do {\
	int x;  \
	for(x=0; x<1000; x++) \
		DELAY_USEC(1000); \
} while(0)
#define USEC_TO_MONOTONIC_CYCLE(x) ( x * cycle_per_usec )
#define MONOTONIC_CYCLE_TO_USEC(x) ( x / cycle_per_usec )

void poll_timer(u64 t);
extern volatile u64 prev, tick;
extern u64 cycle_per_os_tick, cycle_per_usec;
void platform_timer_init(void);
static inline void timer_wheel_disable(void){}
static inline void timer_wheel_enable(void){}
static inline void timer_wheel_set(unsigned short t){}
static inline void prog_timer(unsigned long cycle){}


/******************************************************************************/
/* Uart */
/******************************************************************************/
static inline void uart_init(void){}
static inline int USART_data_available(void){ return 0; }
 
/******************************************************************************/
/* Watchdog */
/******************************************************************************/
#define WATCHDOG_OFF 0
#define WATCHDOG_16MS 0
#define WATCHDOG_32MS 0
#define WATCHDOG_64MS 0
#define WATCHDOG_125MS 0
#define WATCHDOG_250MS 0
#define WATCHDOG_500MS 0
#define WATCHDOG_1S 0
#define WATCHDOG_2S 0
#define WATCHDOG_4S 0
#define WATCHDOG_8S 0
#define wdt_reset()
static inline void watchdog_enable(int x){}

/******************************************************************************/
/* OS */
/******************************************************************************/
#define portSAVE_CONTEXT()
#define portRESTORE_CONTEXT()

#define HANDLE_EXIT() do {\
    PRINT("RTOS exit\n"); \
} while(0)

void exit_critical(void);
 
#endif
