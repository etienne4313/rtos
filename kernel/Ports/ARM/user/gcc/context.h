#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#define barrier() __asm__ __volatile__("": : :"memory")
struct linux_cpu_context_save {
	INT32U   r4;
	INT32U   r5;
	INT32U   r6;
	INT32U   r7;
	INT32U   r8;
	INT32U   r9;
	INT32U   sl;
	INT32U   fp;
	INT32U   sp;
	INT32U   pc;
	void 	*arg;
};

struct linux_thread_info {
	struct linux_cpu_context_save cpu_context;    /* cpu context */
};

struct linux_task_struct {
	struct linux_thread_info thread_info;
};

static inline struct linux_thread_info *info(struct linux_task_struct *task)
{
    return &task->thread_info;
}

#define linux_switch_to(args,prev,next)                   \
do {                                    \
    __linux_switch_to(args, info(prev), info(next));    \
	barrier(); \
} while (0)

/* Assembly support in switch.S */
extern void __linux_switch_to (void *arg, struct linux_thread_info *, struct linux_thread_info *);

#endif
