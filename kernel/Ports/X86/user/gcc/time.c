#include <ucos_ii.h>

volatile u64 prev = 0, tick = 0;
u64 cycle_per_os_tick = 0, cycle_per_usec;

/*
 *********************************************************************************************************
 * Monotonic Time
 *********************************************************************************************************
 */
#define DECLARE_ARGS(val, low, high)    unsigned low, high
#define EAX_EDX_VAL(val, low, high) ((low) | ((u64)(high) << 32))
#define EAX_EDX_ARGS(val, low, high)    "a" (low), "d" (high)
#define EAX_EDX_RET(val, low, high) "=a" (low), "=d" (high)
u64 get_monotonic_cycle(void)
{
    DECLARE_ARGS(val, low, high);
    asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));
    return EAX_EDX_VAL(val, low, high);
}

unsigned long get_monotonic_time(void)
{
	u64 t = get_monotonic_cycle();
	t = t / cycle_per_usec;
	return t;
}

/*
 *********************************************************************************************************
 * Timer wheel
 *********************************************************************************************************
 */
void platform_timer_init(void)
{
	int x;
	u64 t;

	PRINT("Calibrating\n");
	prev = get_monotonic_cycle();
	for(x = 0; x < 1000; x++)
		DELAY_USEC(1000);
	t = get_monotonic_cycle();
	cycle_per_os_tick = ((t - prev)/OS_TICKS_PER_SEC);
	PRINT("Calibration OS_TICKS_PER_SEC %d, CPU cycle %lld\n", OS_TICKS_PER_SEC, cycle_per_os_tick);

	cycle_per_usec = 0;
	prev = get_monotonic_cycle();
	for(x = 0; x < 1024; x++)
		DELAY_USEC(1000);
	t = get_monotonic_cycle();
	cycle_per_usec += (t - prev);
	cycle_per_usec = (cycle_per_usec / 1000) / 1024;
	PRINT("Calibration CPU cyle per uSec %lld\n", cycle_per_usec);

	/* Establish base time stamp */
	prev = get_monotonic_cycle();
	tick = prev;
}

