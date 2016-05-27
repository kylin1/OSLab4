
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


// 共有五个进程,A进程普通进程,B进程是理发师,C进程是顾客,D进程是顾客,E进程是顾客。
// 要求有一把理发椅,并支持等待椅子的数目分 别为 1、2、3
// (必须都能够支持,并且可以现场修改,助教检查时在其中 随机选择数目),
// 开始时理发师处于沉睡状态。理发师理发消耗两个时间片。
PUBLIC	TASK	task_table[NR_TASKS] = {
		//一个task: 函数指针,    栈的大小,     进程的名字
					{TestA, STACK_SIZE_TESTA, "Normal"},
					{TaskB, STACK_SIZE_TESTB, "Hairdresser"},
					{TaskC, STACK_SIZE_TESTC, "CustomerC"},
					{TaskD, STACK_SIZE_TESTD, "CustomerD"},
					{TaskE, STACK_SIZE_TESTE, "CustomerE"}
				};

//普通进程、理发师进程和顾客进程用不同颜色打印,
// 其中顾客要打印递增的顾客ID,并打印基本操作
// 比如理发师剪发,顾客得到服务,顾客到来 并等待,顾客离开等。

//A 进程是不调用 sleep 的 (相当于不可以被 sleep 的系统进程),
// 检查作业时不会要求 A 进程调用 sleep



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

