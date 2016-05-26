
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"

//声明进程表,NR_TASKS是最大允许进程数目
PUBLIC	PROCESS			proc_table[NR_TASKS];

//进程堆栈区域,分配给所有的进程独立使用的部分
PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {
		//一个task: 函数指针,    栈的大小,     进程的名字
					{TestA, STACK_SIZE_TESTA, "TestA"},
					{TestB, STACK_SIZE_TESTB, "TestB"},
					{TestC, STACK_SIZE_TESTC, "TestC"}};

//中断向量表,中断处理程序
//kernel.asm里面hwint_master 使用 call[irq_table + 4 * 参数]
PUBLIC	irq_handler		irq_table[NR_IRQ];

//系统调用表,与syscall.asm里面定义的函数名称相对应
//kernel.asm里面的sys_call 使用 call [sys_call_table + eax * 4]

//这是一个函数指针数组,每一个成员指向一个函数,用以处理相应的系统调用
//system_call是 void* 无论系统调用何种函数,都不会有编译时错误
//第0项是获取当前系统时钟中断发生的次数
PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {
					sys_get_ticks
					};

