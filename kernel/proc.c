
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "string.h"
#include "proto.h"

#include "global.h"
#include "list.h"

/*======================================================================*
                              schedule
                            进程调度函数
                         被clock_handler调用(唯一一次被调用)
 *======================================================================*/
PUBLIC void schedule() {

	//CPU调度使用的就绪队列
	LIST * ready_list = list_table;

//	PROCESS* p;
//	int	greatest_ticks = 0;
//
//	//找到剩余ticks的最大值
//	for (p = proc_table; p < proc_table+NR_TASKS; p++) {
//		if (p->ticks > greatest_ticks) {
//			greatest_ticks = p->ticks;
//		}
//	}
//	//当剩余最大的ticks就是0了,所有的进程都需要再次分配
//	if (!greatest_ticks) {
//		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
//			//ticks设置为初始值
//			p->ticks = p->priority;
//		}
//	}

	//----------在就绪队列中执行时间片轮转调度算法----------

	//把正在运行的当前进程暂时保存
	PROCESS * tobe_last = p_proc_ready;

	//下一个进程是当前就绪队列的第一个进程
	p_proc_ready = ready_list->first;
	p_proc_ready->state = RUNNING;

	//从就绪队列中移除即将被执行的进程
	list_remove(ready_list);

	//将现在正在运行的进程加入就绪队列的尾部,等候下一轮调度
	tobe_last->state = RUNNABLE;
	list_add(ready_list,tobe_last);

}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
int sys_get_ticks() {
	return ticks;
}

/**
 * 调用此 System Call 的进程会在 mill_seconds 毫秒内不被进程调度函数分配时间片
 */
void sys_process_sleep() {
	//参数:睡眠时间
	int mill_seconds = p_proc_ready->regs.ebx;
	disp_int(mill_seconds);

	//睡眠当前进程
	p_proc_ready->state = SLEEP;
	p_proc_ready->sleep_time = mill_seconds;

	//调度进程
	schedule();
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_p(){
	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;
	disp_str("!p ask for : ");
	disp_str(signal->name);

	//申请使用信号量物理值
	signal->value --;

	//如果信号量不可以用
	if(signal->value < 0){
		//则调用此方法的进程阻塞自己,设置自己为等待此信号量状态
		PROCESS * proc_tobe_sleep = p_proc_ready;
		proc_tobe_sleep->state = SLEEP;

		//移入信号量等待队列
		list_add(signal->waiting_list,proc_tobe_sleep);
	}
	//转向调度程序
	schedule();
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_v(){
	//在kernel.asm里面已经关闭了中断,下面的整个过程都是关中断的
	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;
	disp_str("!v release : ");
	disp_str(signal->name);

	//归还信号量物理值
	signal->value ++;

	//有别的进程在等待
	if(signal->value <= 0){
		//释放第一个等待信号量S的进程,改成就绪状态
		LIST * list = signal->waiting_list;
		PROCESS * tobe_wake_up = list->first;
		tobe_wake_up->state = RUNNABLE;

		list_remove(list);

		//CPU调度使用的就绪队列
		LIST * ready_list = list_table;

		//移入就绪队列
		list_add(ready_list,tobe_wake_up);
	}
	//执行V操作的进程继续执行
}