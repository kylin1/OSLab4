
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; 要跟 global.c 中 sys_call_table 的定义相对应！
; sys_call_table表中第0项是sys_get_ticks函数
;                   第1项是sys_process_sleep函数
;                   第2项是sys_disp_str函数
;                   第3项是sys_sem_p函数
;                   第4项是sys_sem_v函数

;添加系统调用之3:在syscall.asm里面添加global函数
;对应好sys_call_table中的下标,也就是对应处理函数
_NR_get_ticks       equ 0
_NR_process_sleep   equ 1
_NR_disp_str        equ 2
_NR_sem_p           equ 3
_NR_sem_v           equ 4

;将系统调用对应的中断号设为90,linux终端号为80,不重复
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	my_get_ticks
global	my_process_sleep
global	my_disp_str
global	my_sem_p
global	my_sem_v

bits 32
[section .text]

; ====================================================================
;                              my_get_ticks
;                     返回当前总共发生了多少次时钟中断
;                        函数调用也是通过中断实现的
; ====================================================================
my_get_ticks:

    ;一,eax的值是一个定义的数字,代表有人要求使用这个系统调用
	;eax = _NR_get_ticks = 0,而系统调用表第0项(为sys_get_ticks函数)
	;sys_get_ticks函数的实现在proc.c里面

	mov	eax, _NR_get_ticks

	;需要定义 INT_VECTOR_SYS_CALL = 90 对应的中断门
	;在protect.c里面的 init_prot()中初始化第90个中断门
	;二,int INT_VECTOR_SYS_CALL 将由sys_call处理

	;三,将寄存器的参数写入核心栈

	int	INT_VECTOR_SYS_CALL

	;四,调用的目标函数就和普通C函数一样处理,都从各自的堆栈中获取参数
    ;sys_call代码:
    ;call    save
    ;sti     ;关中断
    ;调用sys_call_table的第eax个函数
    ;call    [sys_call_table + eax * 4]

	;返回
	ret

my_process_sleep:
    ;应用进程在执行系统调用之前,将参数写入寄存器ebx.ecx等
    mov	eax, _NR_process_sleep
    int	INT_VECTOR_SYS_CALL
    ret

;本次实验要求加入一个系统调用,通过系统调用模式打印字符串。

;对用户展现的函数接口
;int my_disp_str(char *string);

my_disp_str:

    ;系统调用号
    mov	eax, _NR_disp_str

    ;应用进程在执行系统调用之前向约定的寄存器写入参数
    mov ebx, [esp+4]          ;string address

    int	INT_VECTOR_SYS_CALL
    ret

my_sem_p:
    mov	eax, _NR_sem_p
    int	INT_VECTOR_SYS_CALL
    ret

my_sem_v:
    mov	eax, _NR_sem_v
    int	INT_VECTOR_SYS_CALL
    ret
