#ifndef MULTIPLEPROCESS_PROCC_H
#define MULTIPLEPROCESS_PROCC_H

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* Number of tasks */

#include "type.h"
#include "protect.h"
#include "const.h"

#define NR_TASKS	5

/* stacks of tasks */
//增加栈的声明
#define STACK_SIZE_TTY		0x8000
#define STACK_SIZE_TESTA	0x8000
#define STACK_SIZE_TESTB	0x8000
#define STACK_SIZE_TESTC	0x8000
#define STACK_SIZE_TESTD	0x8000
#define STACK_SIZE_TESTE	0x8000

//修改总栈的大小
#define STACK_SIZE_TOTAL	(STACK_SIZE_TTY + \
				STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC + \
				STACK_SIZE_TESTD + \
				STACK_SIZE_TESTE)

typedef struct s_stackframe {	/* proc_ptr points here				↑ Low			*/
    u32	gs;			/* ┓gs指向选择子(内部有描述符索引)						│			*/
    u32	fs;			/* ┃						│			*/
    u32	es;			/* ┃						│			*/
    u32	ds;			/* ┃						│			*/

    u32	edi;		/* ┃						│			*/
    u32	esi;		/* ┣ pushed by save()		│			*/
    u32	ebp;		/* ┃						│			*/
    u32	kernel_esp;	/* <- 'popad' will ignore it│			*/
    u32	ebx;		/* ┃						↑栈从高地址往低地址增长*/
    u32	edx;		/* ┃						│			*/
    u32	ecx;		/* ┃						│			*/
    u32	eax;		/* ┛						│			*/

    u32	retaddr;	/* return address for assembly code save()	│			*/

    u32	eip;		/*  ┓ 指令指针寄存器.存放一个进程当前指令的下一条指令的地址 					│			*/
    u32	cs;			/*  ┃ 把用户栈的%esp的值及相关寄存器压入内核栈中，系统调用通过iret指令返回*/
    u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
    u32	esp;		/*  ┃ 指向一个进程的堆栈						│			*/
    u32	ss;			/*  ┛						┷High			*/
}STACK_FRAME;


//进程的状态被放在s_proc这个结构体中,也就是进程表
//要恢复一个进程时,将esp指针指向这个结构体的开始处,然后一系列pop将寄存器的数值弹出
typedef struct s_proc {

    //前部分是所有相关寄存器的数值 process registers saved in stack frame
    STACK_FRAME regs;

    //GDT一个选择子(内含描述符索引,可得到描述符),给出了进程LDT表的基址与段限
    u16 ldt_sel;

    //进程LDT表 local descriptors for code and data
    DESCRIPTOR ldts[LDT_SIZE];

    //process id passed in from MM
    u32 pid;

    //name of the process
    char p_name[16];

    /*-----------------进程调度相关变量-------------------*/

    //标识这个进程是的状态
    int state;

    //表示在睡眠状态下的进程要睡眠的时间片数目
    int sleep_ticks;

    //remained ticks,递减从初值到0,减到0之后,此进程就不再获得执行的机会
    int ticks;

    //恒定不变的优先度,当所有的进程ticks都减到0之后,
    //再把各自的ticks赋值为priority,以便继续执行
    int priority;

    struct s_proc * next;

}PROCESS;


typedef struct s_task {
    //任务对应的函数指针
    task_f	initial_eip;

    //进程自己的栈
    int		stacksize;

    //名称
    char	name[32];
}TASK;

PUBLIC void schedule();

#endif //MULTIPLEPROCESS_PROCC_H
