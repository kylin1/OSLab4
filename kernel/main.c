
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "global.h"
#include "list.h"
#include "klib.h"
#include "proto.h"
#include "clock.h"
#include "string.h"

/*-----------变量区域--------------------*/

//给用户等待用的椅子数目
//要求有一把理发椅,并支持等待椅子的数目分 别为 1、2、3

//进入理发店的顾客ID
PRIVATE int customer_id;

//控制变量:等候理发的顾客 占用的椅子的数目
PRIVATE int waiting;

//信号量表,初始化信号量
PRIVATE SIGNAL signal_table[3] = {
		//	name		value,  list
		{"!customers! ",	0,	0},
		{"!barbers! ",		0,	0},
		{"!mutex! ",		1,	0}
};

PRIVATE LIST waiting_list_table[3] = {
		{"list1 cus",0,0,0},
		{"list2 bar",0,0,0},
		{"list3 mutex",0,0,0}
};

//信号量指针
PRIVATE SIGNAL* p_customers;
PRIVATE SIGNAL* p_barbers;
PRIVATE SIGNAL* p_mutex;


/*-----------私有方法区域--------------------*/


//初始化信号量指针与信号量的等待队列
PRIVATE void init_waiting_list(){
	p_customers = signal_table;
	p_barbers = signal_table + 1;
	p_mutex = signal_table + 2;

	p_customers->waiting_list = waiting_list_table;
	p_barbers->waiting_list = waiting_list_table+1;
	p_mutex->waiting_list = waiting_list_table+2;

	p_mutex->value = 1;
}


PRIVATE void clear_screen() {
	int i;
	disp_pos = 0;
	for (i = 0; i < 80 * 20; i++) {
		disp_color_str(" ",GREY);
	}
	disp_pos = 0;
}

PRIVATE void show_process_name(int color){
	disp_color_str("\n",color);
	disp_color_str("process name : ",color);
	disp_color_str(p_proc_ready->p_name,color);
}


PUBLIC void check_int(char * str,int input,int color){
	disp_color_str(" ",color);
	disp_color_str(str,color);
	disp_int(input,color);
	disp_color_str(" \n",color);
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

	PROCESS*	proc_barber = proc_table + 1;

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
		p_proc->state = RUNNABLE;
		p_proc->sleep_ticks = 0;



		//为下一个进程初始化:下一个描述符空间(+8)
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

//	//CPU调度使用的就绪队列
//	LIST * ready_list = list_table;
//	list_add(ready_list,p_proc);

	//初始化优先级别与可以获得的ticks数目
	proc_table[0].ticks = proc_table[0].priority =  5;
	proc_table[1].ticks = proc_table[1].priority =  5;
	proc_table[2].ticks = proc_table[2].priority =  5;
	proc_table[3].ticks = proc_table[3].priority =  5;
	proc_table[4].ticks = proc_table[4].priority =  5;

	//中断重入
	k_reenter = 0;
	//初始化系统调用次数ticks = 0;
	ticks = 0;

	/*------------理发师问题数据初始化------------*/

	waiting = 0;
	customer_id = 1;

	//一个时间片的时间长度 (ms为单位)
	//例如 HZ = 100,则time_piece = 10ms,时间片长10MS一个
	ms_per_ticks = 1000/HZ;

	clear_screen();
	init_waiting_list();

	//设置首先启动的进程
	p_proc_ready	= proc_table;
	//初始时钟中断
	init_clock();

	//第四步,调用kernel.asm的restart函数,启动用户进程
	restart();
	while(1){}
}




/*======================================================================*
							进程的任务
 *======================================================================*/

void disp_ticks() {
	disp_str(" ");
	int tic = sys_get_ticks();
	disp_int(tic,GREY);
}

/**
 * A进程普通进程
 */
void TestA() {
	while (1) {
		//普通进程、理发师进程和顾客进程用不同颜色打印
		disp_color_str(" normal ",WHITE);
		delay_ticks(1000);
	}
}

/**
 * 理发师作业
 */
void TaskB() {
	while (1) {
		if(DEBUG){
			disp_color_str("~~~BB start",GREEN);
		}

		//申请顾客customers-1,判断是否有顾客,无顾客,理发师去睡觉
		my_sem_p(p_customers);

		//运行至此,说明被顾客唤醒
		disp_color_str("barber wake up by cus",GREEN);

		/*-----------进入临界区 mutex-1-----------*/
		my_sem_p(p_mutex);

		//等候理发的顾客,占用的椅子数目-1
		waiting --;

		//理发师释放barbers+1,唤醒顾客,准备理发
		my_sem_v(p_barbers);
		my_sem_v(p_mutex);
		/*-----------退出临界区 mutex+1-----------*/

		//打印基本操作:理发师剪发
		disp_color_str(" BB is cutting\n",GREEN);

		// 理发师理发消耗
		delay_ticks(2000);

		if(DEBUG){
			disp_color_str(" BB sleep end~~~\n",GREEN);
		}
	}
}

/**
 * 顾客共同的进程作业
 */
void customer_same(int customer_id){

	if(DEBUG){
		disp_color_str("~~~cus start",ORANGE);
	}

	/*-----------进入临界区 mutex-1-----------*/
	my_sem_p(p_mutex);

	// 其中顾客要打印递增的顾客ID
	check_int(" cus arrive. id:",customer_id,ORANGE);

	//如果还有空的椅子,顾客先坐下
	if(waiting < CHAIR_NUM){
		check_int(" cus waiting. id:",customer_id,ORANGE);
		//等待的顾客占用的椅子数目+1
		waiting ++;
		//增加顾客的数目,customers+1,唤醒p操作等待的理发师
		my_sem_v(p_customers);
		my_sem_v(p_mutex);
		/*-----------离开临界区 mutex+1-----------*/

		//申请理发师,barbers-1,若理发师忙,则顾客坐着等待
		my_sem_p(p_barbers);

		//走到这里,说明理发师不忙,可以理发,顾客得到服务
		disp_color_str(" cus ",ORANGE);
		disp_int(customer_id,ORANGE);
		disp_color_str("get service\n",ORANGE);


		check_int("service done and leave! id:",customer_id,ORANGE);
	}else{
		//没有空的椅子,人满了,顾客离开
		check_int("FULL! cus leave. id:",customer_id,ORANGE);
		my_sem_v(p_mutex);
	}

	my_process_sleep(1000*ms_per_ticks);
}



//C进程是顾客
void TaskC() {
	while (1) {
		customer_same(customer_id++);
	}
}

//D进程是顾客
void TaskD() {
	while (1) {
		customer_same(customer_id++);
	}
}

//E进程是顾客
void TaskE() {
	while (1) {
		customer_same(customer_id++);
	}
}

