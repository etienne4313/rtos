# rax = FCN(rdi, rsi, rdx, rcx, r8, r9)

.global __switch_to_asm # void FCN(rdi, rsi)
__switch_to_asm:
	pushq	%r8
	pushq	%r9
	pushq	%r10
	pushq	%r11
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	pushq	%rax
	pushq	%rbx
	pushq	%rcx
	pushq	%rdx
	pushq	%rsi
	pushq	%rdi
	pushq	%rbp
	movq	%rsp,(%rsi)	# Remember prev stack in *RSI
	movq    %rdi,%rsp 	# Pivot to next stack in RDI
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rax
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	retq

.global __start_to_asm # void FCN(rdi, rsi)
__start_to_asm:
	movq	%rsp,(%rsi)	# Remember prev stack in *RSI
	movq    %rdi,%rsp 	# Pivot to next stack in RDI
	popq	%rbp
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rax
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	retq

.global __set_stack # void FCN(rdi)
__set_stack:
	movq    %rdi,%rsp 	# Pivot to next stack in RDI
	retq

