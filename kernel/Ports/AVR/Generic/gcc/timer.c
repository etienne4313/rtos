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
 *********************************************************************************************************
 * TIMER 1
 * 1 Cycle per increment ( 16Mhz = 62.5 nsec ), 65536 increments => Overflow at 4096uSec
 *********************************************************************************************************
 */
#define BY_1 (0<<CS02   | 0<<CS01  | 1<<CS00) /* TIMER 1 prescale */
#define TIMER1_WRAP	(65536) /* TIMER 1 wrap around count */

static void timer1_setup()
{
	/* Normal mode, no Compare output */
	TCCR1A = 0<<COM1A0 | 0<<COM1A1 | 0<<COM1B0 | 0<<COM1B1 | 0<<WGM10 | 0<<WGM11;
	/* Timer1 is stopped, No input capture, No Waveform */
	TCCR1B = 0<<WGM12 | 0<<WGM13 | 0<<CS10 | 0<<CS11 | 0<<CS12 | 0<<ICNC1 | 0<<ICES1;
	/* No force output compare */
	TCCR1C = 0<<FOC1B | 0<<FOC1A;

	OCR1AH = 0;
	OCR1AL = 0;

	OCR1BH = 0;
	OCR1BL = 0;

	/* 
	 * ISR(TIMER1_CAPT_vect) 
	 * ISR(TIMER1_OVF_vect)
	 * ISR(TIMER1_COMPA_vect)
	 * ISR(TIMER1_COMPB_vect)
	 * No input capture, no output compare, Overflow interrupt
	 */
	TIMSK1 =  0<<ICIE1 | 0<<OCIE1A | 0<<OCIE1B | 1<<TOIE1;
}

/*
 * Setting prescale will kick start timer 1
 */
static void timer1_set_prescale(unsigned char prescale)
{
	unsigned char t;
	t = TCCR1B;
	t &= ~(1<<CS02   | 1<<CS01  | 1<<CS00);
	t = t | prescale;
	TCCR1B = t;
}

/*
 *********************************************************************************************************
 * OS timer
 * TIMER1_OVF_vect IRQ
 * -> Call OSTimeTick() so _must_ match OS_TICKS_PER_SEC 244 in os_cfg.h
 *
 * NOTE this is a NAKED IRQ 'no context saved by gcc' because we are saving our own
 * context specific to the RTOS so that we can context switch to different tasks
 *********************************************************************************************************
 */
ISR_NAKED ISR(TIMER1_OVF_vect)
{
	portSAVE_CONTEXT(); /* Save the full context on the task that got interrupted */
	OSIntEnter();
	OSTimeTick(); /* OSTime++ */
	OSIntExit(); /* Determine if there is a higher priority task ready, if so change OSTCBCur accordingly */
	portRESTORE_CONTEXT(); /* Point the stack to OSTCBCur ( may OR may have change ) and restore the context */
	__asm__ __volatile__ ( "reti" );
}

/*
 *********************************************************************************************************
 * Monotonic Time
 *********************************************************************************************************
 */
unsigned long get_monotonic_cycle(void)
{
	unsigned short c;
	unsigned long t;
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();

	/*
	 * There is an overflow irq pending so TCNT1 has rolled over and for that reason OSTime is 
	 * not incremented yet
	 * The only way this could happen is if A) the overflow irq comes right in-between OS_ENTER_CRITICAL
	 * and below check OR someone calling get_monotonic_time() has IRQ disabled for some period of time
	 * Read TCNT1 and add OSTime+1
	 */
	if(TIFR1 & (1 << TOV1)){
		c = TCNT1;
		t = c + ( OSTime + 1) * TIMER1_WRAP;
		goto out;
	}

	/* 
	 * Here we capture TCNT1 and OSTime but we don't know yet if we can directly used those
	 * values because an overflow irq may happen meanwhile
	 */
	t = OSTime;
	c = TCNT1;

	/* 
	 * So here we check if an overflow IRQ occured and if yes then we take another reading 
	 * of TCNT1 and add OSTime+1
	 */
	if(TIFR1 & (1 << TOV1)){
		c = TCNT1;
		t = c + (OSTime + 1) * TIMER1_WRAP;
		goto out;
	}

	/*
	 * Then here we know that the value captured above was somewhere in the middle of the
	 * TCNT1 cycle i.e. no overflow / no roll over so we take that value directly
	 */
	t = c + t * TIMER1_WRAP;
out:
	OS_EXIT_CRITICAL();
	return t;
}

unsigned long get_monotonic_time(void)
{
	return MONOTONIC_CYCLE_TO_USEC(get_monotonic_cycle());
}

/*
 *********************************************************************************************************
 * Timer wheel
 *********************************************************************************************************
 */
void timer_wheel_disable(void)
{
	/* Disable Output compare A IRQ */
	TIMSK1 &= ~(1<<OCIE1A);
	/* Write a 1 to clear the Output compare A IRQ flag */
	TIFR1 |= (1<<OCF1A);
}
void timer_wheel_enable(void)
{
	/* Write a 1 to clear the Output compare A IRQ flag */
	TIFR1 |= (1<<OCF1A);
	/* Enable  Output compare A IRQ */
	TIMSK1 |= (1<<OCIE1A);
}

void timer_wheel_set(unsigned short t)
{
	OCR1A = t;
}

void prog_timer(unsigned long cycle)
{
	unsigned short c;

	c = TCNT1;
	if( cycle > TIMER1_WRAP){ /* Go for a full period */
		timer_wheel_set(c - 1);
		return;
	}

	if( cycle < (TIMER1_WRAP - c)){ /* Fit in the remaining period */
		timer_wheel_set(c + cycle);
		return;
	}

	timer_wheel_set(cycle - (TIMER1_WRAP - c));
}

/*
 * NOTE this IRQ is handled by gcc hence we are not expecting any RTOS context switch to happen.
 * This IRQ drives the timer wheel and process the callback
 */
ISR(TIMER1_COMPA_vect)
{
	timer_isr();
}

void platform_timer_init(void)
{
	timer1_setup();
	timer1_set_prescale(BY_1);
}

