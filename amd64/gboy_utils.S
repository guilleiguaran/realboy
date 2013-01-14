.globl dec2bcd
.type dec2bcd, @function
.code64
.text
dec2bcd:
	pushq %rbp
	movq %rsp, %rbp	
	subq $128, %rsp # local variables
	movq 8(%rbp), %rsi # address containing word
	movq (%rsi), %rax # load word
	movq %rax, -32(%rbp) # store word
	movq 12(%rbp), %rax # number of bytes
	movq $4, -36(%rbp) # word by default
	cmpq $1, %rax # one byte
	jne 1f
	shlq $24, -32(%rbp) # mask byte
	movq $1, -36(%rbp) # copy one byte
1:
	cmpq $2, %rax # one half-word
	jne 2f
	shlq $16, -32(%rbp) # mask byte
	movq $2, -36(%rbp) # copy one byte
2:
	movq $0, -28(%rbp) # zero low BCD word
	movq $0, -24(%rbp) # zero high BCD word
	movq $0, -20(%rbp) # zero high BCD word

	/* 
	 * Shift and add 3 algorithm.
	 */
	movq -36(%rbp), %rax
	movq %rax, -70(%rbp) # number of shifts
	shlq $3, -70(%rbp) # multiply by 8
	movq -70(%rbp), %rcx # loop value
loop_shft:
	shlq $1, -32(%rbp) # shift 1 bit
	pushf # save carry
	shlq $1, -28(%rbp)
	pushf # save carry
	shlq $1, -24(%rbp)
	popq %rax # restore carry
	andq $1, %rax # mask carry
	orq %rax, -24(%rbp) # 'shift in' bit
	popq %rax # restore carry
	andq $1, %rax # mask carry
	orq %rax, -28(%rbp) # 'shift in' bit
	incq -20(%rbp)
	movq -20(%rbp), %rax
	movq -70(%rbp), %rbx
	cmpq %rbx, %rax
	je end_bin2bcd
	movq $0xf, -52(%rbp) # shift variable
	movq $0xf, -48(%rbp) # shift variable
	movq $0, -56(%rbp) # counter
	movq $0, -42(%rbp) # counter
3:
	movq -52(%rbp), %rax
	testq %rax, %rax
	jz high_bcd_word
	movq -52(%rbp), %rax # load variable
	shlq $4, -52(%rbp) # shift a nibble
	jmp low_bcd_word
high_bcd_word:
	movq -48(%rbp), %rax # load variable
	shlq $4, -48(%rbp) # shift a nibble
	movq -24(%rbp), %rbx # load bcd high
	pushq %rax
	movq -42(%rbp), %rax
	shlq $2, %rax
	movq %rax, %rcx
	shrq %cl, %rbx # shift to start of word
	movq %rbx, %rdx
	andq $0xf, %rdx
	cmpq $5, %rdx # add 3?
	pushf # save comparison
	incq -42(%rbp)
	popq %rdi #  in %rdi
	popq %rax # restore
	pushq %rdi # restore comparison
	popf #  in eflags
	jb 5f # no; leave nibble unchanged
	addq $3, %rbx # add 3
	pushq %rax
	movq -42(%rbp), %rax
	decq %rax
	shlq $2, %rax
	movq %rax, %rcx
	shlq %cl, %rbx # shift to start of word
	popq %rax
	movq -42(%rbp), %rcx
	jmp shf_1
do_shf_1:
	shlq $4, shf_hlp
shf_1:
	loop do_shf_1
	notq shf_hlp
	movq shf_hlp, %rdx
	andq %rdx, -24(%rbp)
	movl $0xffffffff, shf_hlp
	orq %rbx, -24(%rbp) # store new nibble
	jmp 5f
low_bcd_word:
	movq -28(%rbp), %rbx # load bcd low
	pushq %rax
	movq -56(%rbp), %rax
	shlq $2, %rax
	movq %rax, %rcx
	shrq %cl, %rbx # shift to start of word
	movq %rbx, %rdx
	andq $0xf, %rdx
	cmpq $5, %rdx # add 3?
	pushf # save comparison
	incq -56(%rbp)
	popq %rdi #  in %rdi
	popq %rax # restore
	pushq %rdi # restore comparison
	popf #  in eflags
	jb 5f # no; leave nibble unchanged
	addq $3, %rbx # add 3
	pushq %rax
	movq -56(%rbp), %rax
	decq %rax
	shlq $2, %rax
	movq %rax, %rcx
	shlq %cl, %rbx # shift to start of word
	popq %rax
	movq -56(%rbp), %rcx
	jmp shf_2
do_shf_2:
	shlq $4, shf_hlp
shf_2:
	loop do_shf_2
	notq shf_hlp
	movq shf_hlp, %rdx
	andq %rdx, -28(%rbp)
	movl $0xffffffff, shf_hlp
	orq %rbx, -28(%rbp) # store new word
5:
	movq -48(%rbp), %rax
	cmpq $0xf00, %rax # last nibble
	jne 3b
	jmp loop_shft
end_bin2bcd:
	movq $4, -60(%rbp) # control variable
	movq $28, -64(%rbp) # control variable
	movq $0, -68(%rbp) # control variable
	movq $0, -72(%rbp) # control variable
loop_putbuf:
	movq -60(%rbp), %rax
	cmpq $-4, %rax
	je 6f
	movq -24(%rbp), %rax
	movq -60(%rbp), %rcx
	shrq %cl, %rax
	subq $4, -60(%rbp)
	andq $0x0f, %rax
	testq %rax, %rax
	jnz putbuf
	movq -68(%rbp), %rcx
	testq %rcx, %rcx
	jnz putbuf
	jmp end_putbuf
6:
	movq -28(%rbp), %rax
	movq -64(%rbp), %rcx
	shrq %cl, %rax
	subq $4, -64(%rbp)
	andq $0x0f, %rax
	testq %rax, %rax
	jnz putbuf
	movq -68(%rbp), %rcx
	testq %rcx, %rcx
	jnz putbuf
	jmp end_putbuf
putbuf:
	movq $1, -68(%rbp)
	movq 16(%rbp), %rsi # buffer
	addq -72(%rbp), %rsi # position buffer
	incq -72(%rbp)
	addb $0x30, %al
	movb %al, (%rsi)
end_putbuf:
	movq -64(%rbp), %rax
	cmpq $-4, %rax
	jne loop_putbuf
	movq 16(%rbp), %rsi
	movq (%rsi), %rax
	testq %rax, %rax
	jnz 1f
	movb $0x30, (%rsi)
1:
	addq $128, %rsp
	popq %rbp
	ret

.data
shf_hlp:
	.long 0xffffffff
