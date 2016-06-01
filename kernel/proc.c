
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

//	//当下一个进程还有ticks就返回,使得其他进程不会有机会获得执行
//	if (p_proc_ready->ticks > 0) {
//		return;
//	}

	PROCESS* p;
	int	greatest_ticks = 0;

	while (!greatest_ticks) {

		//对于每一个进程
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			//如果这个进程在睡眠状态,则不分配时间片,跳过它
			if(p->is_sleep == 1){
				continue;
			}
			//如果这个进程剩余的ticks比最大的大
			else if (p->ticks > greatest_ticks) {

				//最大剩余ticks = 这个进程的值
				greatest_ticks = p->ticks;
				//下一个进程设为p
				p_proc_ready = p;
			}
		}//循环结束之后,下一个进程就是剩余ticks最大的进程


		//下面的代码会导致系统无限循环打印ABC而一段时间后产生异常

		//当剩余最大的ticks就是0了,所有的进程都需要再次分配
		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				//ticks设置为初始值
				p->ticks = p->priority;
			}
		}
	}
	//讲道理下面应该会切换到restart函数
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
	int mill_seconds = p_proc_ready->regs.ebx;
	disp_int(mill_seconds);

	//关中断
	disable_int();

	//睡眠当前进程
	p_proc_ready->is_sleep = 1;
	p_proc_ready->sleep_time = mill_seconds;

	//调度进程
	schedule();

	//开中断
	enable_int();
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_p(){
	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;
	disp_str("!p ask for : ");
	disp_str(signal->name);

	//关中断
	disable_int();

	//申请使用信号量物理值
	signal->value --;

	//如果信号量不可以用
	if(signal->value < 0){
		//则调用此方法的进程阻塞自己,设置自己为等待此信号量状态
		PROCESS * proc_tobe_sleep = p_proc_ready;
		proc_tobe_sleep->is_sleep = 1;

		//移入信号量等待队列
		list_add(signal->waiting_list,proc_tobe_sleep);
	}
	//转向调度程序
	schedule();

	//开中断
	enable_int();
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_v(){
	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;
	disp_str("!v release : ");
	disp_str(signal->name);
	//关中断
	disable_int();

	//归还信号量物理值
	signal->value ++;

	//有别的进程在等待
	if(signal->value <= 0){
		//释放第一个等待信号量S的进程,改成就绪状态
		LIST * list = signal->waiting_list;
		PROCESS * tobe_wake_up = list->first;
		list_remove(list);

		//移入就绪队列

	}
	enable_int();
	//执行V操作的进程继续执行
}