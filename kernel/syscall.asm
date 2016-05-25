
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0       ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90    ;将系统调用对应的中断号设为90,linux终端号为80,不重复

; 导出符号
global	get_ticks

bits 32
[section .text]

; ====================================================================
;                              get_ticks
;                       返回当前总共发生了多少次时钟中断
; ====================================================================
get_ticks:

    ;eax的值是一个定义的数字,代表有人要求使用这个系统调用
	;eax = _NR_get_ticks = 0,而系统调用表第0项已经被初始化为sys_get_ticks函数
	;sys_get_ticks函数的实现在proc.c里面
	mov	eax, _NR_get_ticks

	;需要定义INT_VECTOR_SYS_CALL(90)对应的中断门
	;在protect.c里面的 init_prot()中初始化中断门
	int	INT_VECTOR_SYS_CALL
	ret

