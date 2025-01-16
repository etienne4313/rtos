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

/* Static table */
#define MAX_SCHEDULE 12
static struct w work_table[MAX_SCHEDULE];
static struct w *current_w;

/* IRQ context */
void timer_isr(void)
{
	unsigned long t;

	if(!current_w) /* spurious IRQ */
		DIE(TIMER);

	t = get_monotonic_cycle();
	if(current_w->time > t){ /* Some time left */
		prog_timer(current_w->time - t);
		return;
	}

expiry:
	/* Expiry process the callback */
	if(!current_w->callback)
		DIE(TIMER);
	current_w->callback(current_w->arg, MONOTONIC_CYCLE_TO_USEC(t));
	current_w->callback = NULL; /* Mark this callback completed */

	if(!current_w->next){ /* The end */
		timer_wheel_disable(); /* Stop the timer */
		current_w = NULL;
		return;
	}

	current_w = current_w->next; /* Get to the next schedule */
	if(current_w->time > t){ /* Some time left */
		prog_timer(current_w->time - t);
		return;
	}
	goto expiry;
}

void poll_timer(cpu_cycle_t t)
{
	if(!current_w)
		return;
	if(current_w->time > t){ /* Some time left */
		return;
	}
expiry:
	/* Expiry process the callback */
	if(!current_w->callback)
		DIE(TIMER);
	current_w->callback(current_w->arg, MONOTONIC_CYCLE_TO_USEC(t));
	current_w->callback = NULL; /* Mark this callback completed */

	if(!current_w->next){ /* The end */
		timer_wheel_disable(); /* Stop the timer */
		current_w = NULL;
		return;
	}

	current_w = current_w->next; /* Get to the next schedule */
	if(current_w->time > t){ /* Some time left */
		prog_timer(current_w->time - t);
		return;
	}
	goto expiry;
}

void schedule_work_absolute(work_t s1, int arg, unsigned long ts)
{
	int x;
	cpu_cycle_t t, timestamp;
	struct w *a, *prev, *curr;

	for(x=0; x<MAX_SCHEDULE; x++){ /* Find an empty spot */
		a = &work_table[x];
		if(a->callback) /* Schedule is still active */
			continue;
		break;
	}
	if(a->callback)
		DIE(TIMER);

	/* Put everything in Timer Cycle */
	timestamp = USEC_TO_MONOTONIC_CYCLE(ts);

	a->callback = s1;
	a->arg = arg;
	a->time = timestamp;
	a->next = NULL;

	t = get_monotonic_cycle(); /* Get current timestamp */
	if(timestamp <= t){ /* Past expiration ?  process the callback */
		if(!a->callback)
			DIE(TIMER);
		a->callback(a->arg, MONOTONIC_CYCLE_TO_USEC(t));
		a->callback = NULL; /* Mark this callback completed */
		return;
	}

	if(!current_w){ /* Nothing is running so start fresh */
		timer_wheel_disable(); /* Make sure the timer is stopped */
		current_w = a; /* Set current schedule to be this one */
		prog_timer(current_w->time - t); /* Program the timer */
		timer_wheel_enable(); /* Start the timer */
		return;
	}

	/* 
	 * There is an active schedule but this schedule will expire sooner than
	 * the current one so reprogram the timer with this new one
	 */
	if(timestamp < current_w->time){
		prev = current_w; /* Remember current */
		current_w = a; /* Replace current with this one */
		current_w->next = prev; /* Old current is the next one */
		prog_timer(current_w->time - t); /* Program the timer */
		return;
	}

	/* 
	 * This schedule will expire later than the current one so insert time sorted
	 * in the list. We don't need to reprogram the timer here
	 */
	curr = current_w;
	while(curr != NULL){ /* Traverse the list */
		if(!curr->next)
			break; /* Insert at the end */
		prev = curr;
		curr = curr->next;
		if(timestamp < curr->time){
			a->next = curr;
			curr = prev;
			break; /* Insert in the middle */
		}
	}
	curr->next = a;
	return;
}

void timer_init(void)
{
	platform_timer_init();
	timer_wheel_disable();
	memset(work_table, 0, sizeof(struct w) * MAX_SCHEDULE);
	current_w = NULL;
}

