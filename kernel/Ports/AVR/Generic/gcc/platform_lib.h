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
#define DELAY_USEC(d) _delay_us(d)
#define DELAY_MSEC(d) _delay_ms(d)
#define MONOTONIC_CYCLE_TO_USEC(x) ( x >> 4 ) /* 16 Timer1 cycle in 1 uSec assuming 16Mhz CPU */
#define USEC_TO_MONOTONIC_CYCLE(x) ( x << 4 )
void platform_timer_init(void);
void timer_wheel_disable(void);
void timer_wheel_enable(void);
void timer_wheel_set(unsigned short t);
void prog_timer(unsigned long cycle);

/******************************************************************************/
/* Uart */
/******************************************************************************/
void uart_init(void);
void USART_Transmit( unsigned char data );
unsigned char USART_Receive( void );
int USART_data_available(void);
int USART_Flush( void );

/******************************************************************************/
/* Watchdog */
/******************************************************************************/
#define WATCHDOG_OFF    (0)
#define WATCHDOG_16MS   (_BV(WDE))
#define WATCHDOG_32MS   (_BV(WDP0) | _BV(WDE))
#define WATCHDOG_64MS   (_BV(WDP1) | _BV(WDE))
#define WATCHDOG_125MS  (_BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_250MS  (_BV(WDP2) | _BV(WDE))
#define WATCHDOG_500MS  (_BV(WDP2) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_1S     (_BV(WDP2) | _BV(WDP1) | _BV(WDE))
#define WATCHDOG_2S     (_BV(WDP2) | _BV(WDP1) | _BV(WDP0) | _BV(WDE))
#define WATCHDOG_4S     (_BV(WDP3) | _BV(WDE))
#define WATCHDOG_8S     (_BV(WDP3) | _BV(WDP0) | _BV(WDE))
#define wdt_reset() __asm__ __volatile__ ("wdr")
static inline void watchdog_enable(uint8_t x)
{
	MCUSR = 0;
	WDTCSR = _BV(WDCE) | _BV(WDE);
	WDTCSR = x;
}

/******************************************************************************/
/* OS */
/******************************************************************************/
#define HANDLE_EXIT() DIE(-1)
#define OS_POLLING_EN	0u   /* OS works in polling mode or IRQ mode */

#endif
