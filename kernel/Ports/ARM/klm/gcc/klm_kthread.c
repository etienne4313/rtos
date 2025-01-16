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

#ifdef __KTHREAD_MODE__

extern int rtos_dead;
extern int main(void);
static int curr_cpu = -1;

int x86_rtos_entry(void *arg);
static struct task_struct *t1;
//static DEFINE_SPINLOCK(irq_mc_lock);

int x86_rtos_entry(void *arg)
{
	schedule_timeout(HZ); // Give some breathing room to insmod

	__set_current_state(TASK_RUNNING);

	PRINT("Starting RTOS on CPU %d:%d\n", curr_cpu, smp_processor_id());
	__module_get(THIS_MODULE);
//	spin_lock_irq(&irq_mc_lock);
	main();
//	spin_unlock_irq(&irq_mc_lock);
	while (!kthread_should_stop()) {
		cond_resched();
	}
	PRINT("Kthread return\n");
	return 0;
}

/*
 * Readind /proc/rtos signal the RTOS event loop via the rtos_dead flag.
 * The RTOS in turns set the stack back to the original value and return
 * from x86_rtos_entry
 */
static int rtos_kill(struct seq_file *m, void *v)
{
	char buf[16];
	snprintf(buf, 16, "%d", curr_cpu);
	seq_puts(m, "RTOS kill: ");
	seq_puts(m, buf);
	seq_putc(m, '\n');
	rtos_dead = 1;
	module_put(THIS_MODULE);
	kthread_stop(t1);
	return 0;
}

static int __init init(void)
{
	char buf[16];

	if(curr_cpu < 0 || curr_cpu >= CONFIG_NR_CPUS)
		return -EINVAL;
	snprintf(buf, 16, "rtos_%d", curr_cpu);
	proc_create_single(buf, 0, NULL, rtos_kill);
	rtos_dead = 0;
	t1 = kthread_create_on_cpu(x86_rtos_entry, NULL, curr_cpu, buf);
	wake_up_process(t1);
	return 0;
}

static void __exit fini(void)
{
	char buf[16];
	snprintf(buf, 16, "rtos_%d", curr_cpu);
	remove_proc_entry(buf, NULL);
}

MODULE_PARM_DESC(curr_cpu, "RTOS executing CPU");
module_param(curr_cpu, int, 0644);

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL v2");

#endif /* __KTHREAD_MODE__ */
