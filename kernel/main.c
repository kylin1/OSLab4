
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "klib.h"
#include "clock.h"

/*-----------变量区域--------------------*/

//给用户等待用的椅子数目
//要求有一把理发椅,并支持等待椅子的数目分 别为 1、2、3
PRIVATE int num_of_chair;

//进入理发店的顾客ID
PRIVATE int customer_id;

//控制变量:等候理发的顾客 占用的椅子的数目
PRIVATE int waiting;

//信号量表,初始化信号量
PRIVATE SIGNAL signal_table[3] = {
		//value, name,		list,first,available
		{0,	"!customers! ",		0,	0,	0},
		{0,	"!barbers! ",		0,	0,	0},
		{1,	"!mutex! ",			0,	0,	0}
};

//信号量指针
PRIVATE SIGNAL* p_customers = signal_table;
PRIVATE SIGNAL* p_barbers = signal_table + 1;
PRIVATE SIGNAL* p_mutex = signal_table + 2;


/*-----------私有方法区域--------------------*/

//理发师理发消耗两个时间片
PRIVATE void cut_hair(){
	// 打印基本操作:理发师剪发
	my_disp_str("barber is serving customer ",GREEN);
}

PRIVATE void get_haircut(int customer_id){
	// 打印基本操作:理发师剪发
	my_disp_str("customer ",GREEN);
	disp_int(customer_id);
	my_disp_str("is being served ",GREEN);
	//顾客理发结束
	my_disp_str("customer id = ",RED);
	disp_int(customer_id);
	my_disp_str("DONE! ",RED);
}

PRIVATE void clear_screen() {
	int i;
	disp_pos = 0;
	for (i = 0; i < 80 * 20; i++) {
		disp_str(" ");
	}
	disp_pos = 0;
}

PRIVATE void show_process_name(){
	my_disp_str("process name : ",GREY);
	my_disp_str(p_proc_ready->p_name,GREY);
}

PRIVATE void wait_sometime(){
	milli_delay(WAIT_TIMES);
}

PRIVATE void check_int(int input,char * str){
	my_disp_str(str,RED);
	disp_int(input);
}

/*======================================================================*
                            kernel_main
                           主函数开始
 *======================================================================*/
PUBLIC int kernel_main()
{
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
		p_proc->pid = (u32) i;

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

		/**------------进程调度信息-------------*/
		p_proc->is_sleep = 0;
		p_proc->sleep_time = 0;

		//为下一个进程初始化:下一个描述符空间(+8)
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//初始化优先级别与可以获得的ticks数目
	proc_table[0].ticks = proc_table[0].priority =  100;
	proc_table[1].ticks = proc_table[1].priority =  100;
	proc_table[2].ticks = proc_table[2].priority =  100;
	proc_table[3].ticks = proc_table[3].priority =  100;
	proc_table[4].ticks = proc_table[4].priority =  100;

	//中断重入
	k_reenter = 0;
	//初始化系统调用次数ticks = 0;
	ticks = 0;


	/*------------理发师问题数据初始化------------*/
	num_of_chair = 3;
	waiting = 0;
	customer_id = 0;


	//设置首先启动的进程
	p_proc_ready	= proc_table;
	//初始时钟中断
	init_clock();

	//ring0 到 ring1(系统到用户)跳转
	//第四步,调用kernel.asm的restart函数,启动用户进程
	restart();

	//开始时理发师处于沉睡状态。
	clear_screen();
	while(1){}
}

/*======================================================================*
                               TestA

 *======================================================================*/
//A进程普通进程
void TestA() {
	while (1) {
		//普通进程、理发师进程和顾客进程用不同颜色打印
		show_process_name();
		wait_sometime();
	}
}


//B进程是理发师
void TaskB() {
	while (1) {
		show_process_name();

		//申请顾客customers-1,判断是否有顾客,无顾客,理发师去睡觉
		my_sem_p(p_customers);

		//运行至此,说明被顾客唤醒

		/*-----------进入临界区 mutex-1-----------*/
		my_sem_p(p_mutex);

		//等候理发的顾客,占用的椅子数目-1
		waiting --;

		//理发师释放barbers+1,唤醒顾客,准备理发
		my_sem_v(p_barbers);
		my_sem_v(p_mutex);
		/*-----------退出临界区 mutex+1-----------*/

		//理发
		cut_hair();

		//体现过程
		wait_sometime();
	}
}


PRIVATE void customer_task(){

	/*-----------进入临界区 mutex-1-----------*/
	my_sem_p(p_mutex);

	// 其中顾客要打印递增的顾客ID
	customer_id ++;
	check_int(customer_id,"customer arrive , id = ");
	check_int(waiting,"waiting = ");
	check_int(num_of_chair,"num_of_chair = ");

	//如果还有空的椅子,顾客先坐下
	if(waiting < num_of_chair){
		//等待的顾客占用的椅子数目+1
		waiting ++;
		//增加顾客的数目,customers+1,唤醒p操作等待的理发师
		my_sem_v(p_customers);
		my_sem_v(p_mutex);
	/*-----------离开临界区 mutex+1-----------*/

		//申请理发师,barbers-1,若理发师忙,则顾客坐着等待
		my_sem_p(p_barbers);

		//走到这里,说明理发师不忙,可以理发,顾客得到服务
		get_haircut(customer_id);

	//没有空的椅子,人满了,顾客离开
	}else{
		my_disp_str("FULL! customer leave , id = ",RED);
	}
	// 顾客到来 并等待

}


//C进程是顾客
void TaskC() {
	while (1) {
		show_process_name();
		customer_task();
		wait_sometime();
	}
}

//D进程是顾客
void TaskD() {
	while (1) {
		show_process_name();
		customer_task();
		wait_sometime();
	}
}

//E进程是顾客
void TaskE() {
	while (1) {
		show_process_name();
		customer_task();
		wait_sometime();
	}
}

