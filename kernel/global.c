
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "global.h"

#include "proto.h"



//声明进程表,NR_TASKS是最大允许进程数目
PUBLIC	PROCESS			proc_table[NR_TASKS];

//进程堆栈区域,分配给所有的进程独立使用的部分
PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {
		//一个task: 函数指针,    栈的大小,     进程的名字
					{TestA, STACK_SIZE_TESTA, "Normal "},
					{TaskB, STACK_SIZE_TESTB, "Barber "},
					{TaskC, STACK_SIZE_TESTC, "CustomerC "},
					{TaskD, STACK_SIZE_TESTD, "CustomerD "},
					{TaskE, STACK_SIZE_TESTE, "CustomerE "}
				};


PUBLIC	TTY		tty_table[NR_CONSOLES];
PUBLIC	CONSOLE		console_table[NR_CONSOLES];






//中断向量表,中断处理程序
//kernel.asm里面hwint_master 使用 call[irq_table + 4 * 参数]
//被put_irq_handler(CLOCK_IRQ, clock_handler);设置中断处理程序
PUBLIC	irq_handler		irq_table[NR_IRQ];



//系统调用表,与syscall.asm里面定义的函数名称相对应
//kernel.asm里面的 sys_call 使用 call [sys_call_table + eax * 4]

//这是一个函数指针数组,每一个成员指向一个函数,用以处理相应的系统调用
//system_call是 void* 无论系统调用何种函数,都不会有编译时错误
//第0项是获取当前系统时钟中断发生的次数

//添加系统调用之1:加入系统调用处理函数
PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {
					sys_get_ticks,
					sys_process_sleep,
					sys_disp_str,
					sys_sem_p,
					sys_sem_v
				};

