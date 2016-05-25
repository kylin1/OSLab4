#include "type.h"
#include "protect.h"
#include "const.h"

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


typedef struct s_stackframe {	/* proc_ptr points here				↑ Low			*/
	u32	gs;		/* ┓gs指向选择子(内部有描述符索引)						│			*/
	u32	fs;		/* ┃						│			*/
	u32	es;		/* ┃						│			*/
	u32	ds;		/* ┃						│			*/

	u32	edi;		/* ┃						│			*/
	u32	esi;		/* ┣ pushed by save()				│			*/
	u32	ebp;		/* ┃						│			*/
	u32	kernel_esp;	/* <- 'popad' will ignore it			│			*/
	u32	ebx;		/* ┃						↑栈从高地址往低地址增长*/
	u32	edx;		/* ┃						│			*/
	u32	ecx;		/* ┃						│			*/
	u32	eax;		/* ┛						│			*/

	u32	retaddr;	/* return address for assembly code save()	│			*/

	u32	eip;		/*  ┓ 指令指针寄存器.存放一个进程当前指令的下一条指令的地址 					│			*/
	u32	cs;			/*  ┃						│			*/
	u32	eflags;		/*  ┣ these are pushed by CPU during interrupt	│			*/
	u32	esp;		/*  ┃ 指向一个进程的堆栈						│			*/
	u32	ss;			/*  ┛						┷High			*/
}STACK_FRAME;


//进程的状态被放在s_proc这个结构体中,也就是进程表
//要恢复一个进程时,将esp指针指向这个结构体的开始出,然后一系列pop将寄存器的数值弹出
typedef struct s_proc {
	STACK_FRAME regs;          /* 前部分是所有相关寄存器的数值 process registers saved in stack frame */

	//GDT一个选择子(内含描述符索引,可得到描述符),给出了进程LDT表的基址与段限
	u16 ldt_sel;               /* gdt selector giving ldt base and limit */

	//进程LDT表
	DESCRIPTOR ldts[LDT_SIZE]; /* local descriptors for code and data */

        int ticks;                 /* remained ticks */
        int priority;

	u32 pid;                   /* process id passed in from MM */
	char p_name[16];           /* name of the process */
}PROCESS;

typedef struct s_task {
	task_f	initial_eip;
	int	stacksize;
	char	name[32];
}TASK;


/* Number of tasks */
#define NR_TASKS	3

/* stacks of tasks */
#define STACK_SIZE_TESTA	0x8000
#define STACK_SIZE_TESTB	0x8000
#define STACK_SIZE_TESTC	0x8000

#define STACK_SIZE_TOTAL	(STACK_SIZE_TESTA + \
				STACK_SIZE_TESTB + \
				STACK_SIZE_TESTC)

PUBLIC void schedule();