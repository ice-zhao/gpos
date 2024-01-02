    .include "machine.inc"


    /*
	    void iprintk(char* str)
	parameter:
	    str:  the pointer of string to output
	*/
    .code32
    .section .text
    .globl iprintk
	.type iprintk, @function
iprintk:
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    pushl %esi
    pushl %edi

    movl 8(%ebp), %esi
    movl $cursor_pos, %eax
    movl (%eax), %edi       #load cursor position
    movb $0xF, %ah          #charactor color
    cmpl $4000, %edi        #output 25 line, go back to first line
    jb 1f
    movl $0, %edi

1:
    lodsb
    test %al, %al       #get string's end char 0?
    jz 2f
    cmp $0xA, %al       #is an enter?
    jnz 3f
    pushl %eax
    movl %edi, %eax
    mov $160, %bl
    div %bl
    and $0xFF, %eax
    inc %eax
    mov $160, %bl
    mul %bl
    movl %eax, %edi
    popl %eax
    jmp 1b
3:
    movw %ax, %gs:(%edi)
    add $2, %edi
    jmp 1b
2:
	movl $cursor_pos, %eax
    movl %edi, (%eax)        #save cursor current position

    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

    .section .data
_cursor_pos:
	    .4byte (80 * 0 + 0) * 2   # at 0 line, 0 column
	    .equ   cursor_pos, (KERNEL_PHY_ADDR + _cursor_pos)
