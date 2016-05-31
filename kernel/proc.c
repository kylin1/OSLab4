
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#include "string.h"

#include "proc.h"
#include "proto.h"

#include "global.h"

/*======================================================================*
                              schedule
                            进程调度函数
                         被clock_handler调用(唯一一次被调用)
 *======================================================================*/
PUBLIC void schedule() {

	//当下一个进程还有ticks就返回,使得其他进程不会有机会获得执行
	if (p_proc_ready->ticks > 0) {
		return;
	}

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
int sys_process_sleep(int mill_seconds) {
	//关中断
	disable_int();

	//睡眠当前进程
	p_proc_ready->is_sleep = 1;
	p_proc_ready->sleep_time = mill_seconds;

	schedule();

	//开中断
	enable_int();
	return 0x11;
}

/**
 * 信号量的 PV 操作
 */
int sys_sem_p(SIGNAL* signal){
	//关中断
	disable_int();

	//申请使用信号量物理值
	signal->value --;

	//如果信号量不可以用
	if(signal->value < 0){
		//则调用此方法的进程阻塞自己,设置自己为等待此信号量状态
		PROCESS * proc_tobe_sleep = p_proc_ready;
		proc_tobe_sleep->is_sleep = 1;

		//加入等待队列的空闲位置,移入信号量等待队列,
		int available = signal->availbale;

		signal->waiting_list[available] = proc_tobe_sleep;
		//移动空闲指针
		signal->availbale = (signal->availbale + 1) % signal->max_size;
		signal->size ++;
	}
	//转向调度程序
	schedule();

	//开中断
	enable_int();
	return 0x33;
}

/**
 * 信号量的 PV 操作
 */
int sys_sem_v(SIGNAL* signal){
	return 0x44;
}