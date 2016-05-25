
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "include/string.h"
#include "include/type.h"
#include "include/const.h"
#include "include/protect.h"
#include "include/proto.h"
#include "include/proc.h"
#include "include/global.h"
#include "lib/klib.h"
#include "include/clock.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	//第三步,初始化进程表

	//任务表与进程信息,在global.c里面初始化
	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;

	//所有进程的栈,在global.c里面声明了空间
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;

	//为每一个进程在GDT中分配一个描述符来对应进程的LDT
	u16		selector_ldt	= SELECTOR_LDT_FIRST;

	//初始化每一个进程
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		//进程的名字是p_task 的名字
		strcpy(p_proc->p_name, p_task->name);
		// 进程ID : pid
		p_proc->pid = i;

		//GDT一个选择子,给出此进程的LDT表的地址信息,解决了where的问题
		p_proc->ldt_sel = selector_ldt;

		//拷贝两个DESCRIPTOR数组项目,对应代码段与数据段
		int size = sizeof(DESCRIPTOR);
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], size);
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;

		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], size);
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;

		//cs指向LDT中第一个描述符
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;

		//ds,es,fs,ss都指向LDT中第二个描述符
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;

		//gs还是指向显存
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		//eip(指令地址)指向  initial_eip(task_f类型,函数指针)
		p_proc->regs.eip = (u32)p_task->initial_eip;

		//esp指向单独的堆栈栈顶,大小为STACK_SIZE_TOTAL
		p_proc->regs.esp = (u32)p_task_stack;

		//设置eflags:使得进程可以使用I/O指令,IF=1,中断会在iretd执行的时候被打开
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		//栈的大小减去分配的这一个(从高向低分配空间)
		p_task_stack -= p_task->stacksize;

		//下一个进程的信息
		p_proc++;
		p_task++;

		//为下一个进程初始化下一个描述符空间(+8)
		selector_ldt += 1 << 3;
	}

	//设定优先级别
	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority =  5;
	proc_table[2].ticks = proc_table[2].priority =  3;

	//中断重入
	k_reenter = 0;

	//初始化系统调用次数ticks = 0;
	ticks = 0;

	//准备开始运行各个进程(kernel.asm里面的restart函数使用)
	p_proc_ready	= proc_table;

	//初始化 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	//设定时钟中断处理程序,为irq_table第CLOCK_IRQ(0)个数值赋值
	//PUBLIC void clock_handler(int irq)
	put_irq_handler(CLOCK_IRQ, clock_handler);

	//让8259A可以接收时钟中断
	enable_irq(CLOCK_IRQ);

	//ring0 到 ring1的跳转
	//第四步,调用kernel.asm的restart函数
	restart();

	while(1){}
}

/*======================================================================*
                               TestA

 *======================================================================*/
//第一步,准备一个小的进程体
//进程体在内核被LOADER放置到内存之后就准备好了
void TestA()
{
	int i = 0;
	while (1) {
		disp_str("A.");
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	int i = 0x1000;
	while(1){
		disp_str("B.");
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestC()
{
	int i = 0x2000;
	while(1){
		disp_str("C.");
		milli_delay(10);
	}
}
