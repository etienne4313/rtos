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

#ifdef __AMP_MODE__

extern int rtos_dead;
extern int main(void);
static int curr_cpu = -1;

extern void (*rtos_entry[CONFIG_NR_CPUS])(void); /* Kernel patch */
void x86_rtos_entry(void);

/*
 * When the module is loaded, *rtos_entry is set to x86_rtos_entry()
 * Then upon taking the CPU offline using
 * 	echo 0 >/sys/devices/system/cpu/cpu'n'/online
 * the dying CPU will call rtos_entry(); which is jst before going in mwait()
 * from native_play_dead()
 * From this point on, don't let the module go unless the RTOS has terminated
 */
void x86_rtos_entry(void)
{
	PRINT("Starting RTOS on CPU %d:%d\n", curr_cpu, smp_processor_id());
	__module_get(THIS_MODULE);
	main();
}

/*
 * Readind /proc/rtos signal the RTOS event loop via the rtos_dead flag.
 * The RTOS in turns set the stack back to the original value and return
 * from *rtos_entry hence continue the execution in native_play_dead() which
 * will park the CPU in MWAIT.
 *
 * Then at that point the CPU can be taken back online using
 *  echo 1 >/sys/devices/system/cpu/cpu'n'/online
 *
 * NOTE Somehow the CPU can be taken back online without terminating the RTOS
 *  This is specific to the CPU type / feature
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
	return 0;
}

static int __init init(void)
{
	char buf[16];

	if(curr_cpu < 0 || curr_cpu >= CONFIG_NR_CPUS)
		return -EINVAL;
	if(rtos_entry[curr_cpu])
		return -EBUSY;
	rtos_entry[curr_cpu] = x86_rtos_entry;
	snprintf(buf, 16, "rtos_%d", curr_cpu);
	proc_create_single(buf, 0, NULL, rtos_kill);
	rtos_dead = 0;
	return 0;
}

static void __exit fini(void)
{
	char buf[16];
	snprintf(buf, 16, "rtos_%d", curr_cpu);
	remove_proc_entry(buf, NULL);
	rtos_entry[curr_cpu] = NULL;
}

MODULE_PARM_DESC(curr_cpu, "RTOS executing CPU");
module_param(curr_cpu, int, 0644);

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL v2");

#endif /* __AMP_MODE__ */
