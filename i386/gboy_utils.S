.globl dec2bcd
.type dec2bcd, @function
.code32
.text
dec2bcd:
	pushl %ebp
	movl %esp, %ebp	
	subl $128, %esp # local variables
	movl 8(%ebp), %esi # address containing word
	movl (%esi), %eax # load word
	movl %eax, -32(%ebp) # store word
	movl 12(%ebp), %eax # number of bytes
	movl $4, -36(%ebp) # word by default
	cmpl $1, %eax # one byte
	jne 1f
	shll $24, -32(%ebp) # mask byte
	movl $1, -36(%ebp) # copy one byte
1:
	cmpl $2, %eax # one half-word
	jne 2f
	shll $16, -32(%ebp) # mask byte
	movl $2, -36(%ebp) # copy one byte
2:
	movl $0, -28(%ebp) # zero low BCD word
	movl $0, -24(%ebp) # zero high BCD word
	movl $0, -20(%ebp) # zero high BCD word

	/* 
	 * Shift and add 3 algorithm.
	 */
	movl -36(%ebp), %eax
	movl %eax, -70(%ebp) # number of shifts
	shll $3, -70(%ebp) # multiply by 8
	movl -70(%ebp), %ecx # loop value
loop_shft:
	shll $1, -32(%ebp) # shift 1 bit
	pushf # save carry
	shll $1, -28(%ebp)
	pushf # save carry
	shll $1, -24(%ebp)
	popl %eax # restore carry
	andl $1, %eax # mask carry
	orl %eax, -24(%ebp) # 'shift in' bit
	popl %eax # restore carry
	andl $1, %eax # mask carry
	orl %eax, -28(%ebp) # 'shift in' bit
	incl -20(%ebp)
	movl -20(%ebp), %eax
	movl -70(%ebp), %ebx
	cmpl %ebx, %eax
	je end_bin2bcd
	movl $0xf, -52(%ebp) # shift variable
	movl $0xf, -48(%ebp) # shift variable
	movl $0, -56(%ebp) # counter
	movl $0, -42(%ebp) # counter
3:
	movl -52(%ebp), %eax
	testl %eax, %eax
	jz high_bcd_word
	movl -52(%ebp), %eax # load variable
	shll $4, -52(%ebp) # shift a nibble
	jmp low_bcd_word
high_bcd_word:
	movl -48(%ebp), %eax # load variable
	shll $4, -48(%ebp) # shift a nibble
	movl -24(%ebp), %ebx # load bcd high
	pushl %eax
	movl -42(%ebp), %eax
	shll $2, %eax
	movl %eax, %ecx
	shrl %cl, %ebx # shift to start of word
	movl %ebx, %edx
	andl $0xf, %edx
	cmpl $5, %edx # add 3?
	pushf # save comparison
	incl -42(%ebp)
	popl %edi #  in %edi
	popl %eax # restore
	pushl %edi # restore comparison
	popf #  in eflags
	jb 5f # no; leave nibble unchanged
	addl $3, %ebx # add 3
	pushl %eax
	movl -42(%ebp), %eax
	decl %eax
	shll $2, %eax
	movl %eax, %ecx
	shll %cl, %ebx # shift to start of word
	popl %eax
	movl -42(%ebp), %ecx
	jmp shf_1
do_shf_1:
	shll $4, shf_hlp
shf_1:
	loop do_shf_1
	notl shf_hlp
	movl shf_hlp, %edx
	andl %edx, -24(%ebp)
	movl $0xffffffff, shf_hlp
	orl %ebx, -24(%ebp) # store new nibble
	jmp 5f
low_bcd_word:
	movl -28(%ebp), %ebx # load bcd low
	pushl %eax
	movl -56(%ebp), %eax
	shll $2, %eax
	movl %eax, %ecx
	shrl %cl, %ebx # shift to start of word
	movl %ebx, %edx
	andl $0xf, %edx
	cmpl $5, %edx # add 3?
	pushf # save comparison
	incl -56(%ebp)
	popl %edi #  in %edi
	popl %eax # restore
	pushl %edi # restore comparison
	popf #  in eflags
	jb 5f # no; leave nibble unchanged
	addl $3, %ebx # add 3
	pushl %eax
	movl -56(%ebp), %eax
	decl %eax
	shll $2, %eax
	movl %eax, %ecx
	shll %cl, %ebx # shift to start of word
	popl %eax
	movl -56(%ebp), %ecx
	jmp shf_2
do_shf_2:
	shll $4, shf_hlp
shf_2:
	loop do_shf_2
	notl shf_hlp
	movl shf_hlp, %edx
	andl %edx, -28(%ebp)
	movl $0xffffffff, shf_hlp
	orl %ebx, -28(%ebp) # store new word
5:
	movl -48(%ebp), %eax
	cmpl $0xf00, %eax # last nibble
	jne 3b
	jmp loop_shft
end_bin2bcd:
	movl $4, -60(%ebp) # control variable
	movl $28, -64(%ebp) # control variable
	movl $0, -68(%ebp) # control variable
	movl $0, -72(%ebp) # control variable
loop_putbuf:
	movl -60(%ebp), %eax
	cmpl $-4, %eax
	je 6f
	movl -24(%ebp), %eax
	movl -60(%ebp), %ecx
	shrl %cl, %eax
	subl $4, -60(%ebp)
	andl $0x0f, %eax
	testl %eax, %eax
	jnz putbuf
	movl -68(%ebp), %ecx
	testl %ecx, %ecx
	jnz putbuf
	jmp end_putbuf
6:
	movl -28(%ebp), %eax
	movl -64(%ebp), %ecx
	shrl %cl, %eax
	subl $4, -64(%ebp)
	andl $0x0f, %eax
	testl %eax, %eax
	jnz putbuf
	movl -68(%ebp), %ecx
	testl %ecx, %ecx
	jnz putbuf
	jmp end_putbuf
putbuf:
	movl $1, -68(%ebp)
	movl 16(%ebp), %esi # buffer
	addl -72(%ebp), %esi # position buffer
	incl -72(%ebp)
	addb $0x30, %al
	movb %al, (%esi)
end_putbuf:
	movl -64(%ebp), %eax
	cmpl $-4, %eax
	jne loop_putbuf
	movl 16(%ebp), %esi
	movl (%esi), %eax
	testl %eax, %eax
	jnz 1f
	movb $0x30, (%esi)
1:
	addl $128, %esp
	popl %ebp
	ret

.data
shf_hlp:
	.long 0xffffffff
