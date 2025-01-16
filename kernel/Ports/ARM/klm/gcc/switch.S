#
# r0 / r1 = fcn ( r0, r1, r2, r3 ) Scratch
# r4 to r12: Local variables. Must restore
#
# r11 frame pointer
# r13 Stack pointer
# r14 Link register
# r15 Program counter
#

/*
 * Register switch for ARMv7-M processors.
 * r0 = ARGS, r1 = previous thread_info, r2 = next thread_info
 * previous and next are guaranteed not to be the same.
 */
.global __linux_switch_to
__linux_switch_to:
    .fnstart
    .cantunwind
    add ip, r1, #0 /* offsetof(struct thread_info, cpu_context) */
    stmia   ip!, {r4 - r11}     @ Store most regs on stack
    str sp, [ip], #4
    str lr, [ip], #4

#	Debug; Return here and dump register
#	bx	lr
    
	add ip, r2, #0 /* offsetof(struct thread_info, cpu_context) */
    ldmia   ip!, {r4 - r11}     @ Load all regs saved previously
    ldr sp, [ip]
    ldr pc, [ip, #4]!

    .fnend

