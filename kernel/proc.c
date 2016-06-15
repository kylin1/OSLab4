
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "string.h"
#include "proto.h"

#include "global.h"
#include "list.h"
#include "klib.h"

/*======================================================================*
                              schedule
                            进程调度函数
                         被clock_handler调用(唯一一次被调用)
 *======================================================================*/


//切换到进程表中的下一个进程
void change_proc(){
	p_proc_ready ++;
	if (p_proc_ready >= proc_table + NR_TASKS) {
		p_proc_ready = proc_table;
	}
}

PUBLIC void schedule() {
	PROCESS* p;
	//调整按照时间片睡眠进程的睡眠时间
	for(p = proc_table; p < proc_table+NR_TASKS; p++){
		if(p->state == SLEEP && p->sleep_ticks > 0){
			p->sleep_ticks--;
		}
		//睡眠时间到了,可以被唤醒
		else if(p->state == SLEEP && p->sleep_ticks <= 0){
			p->state = RUNNABLE;
		}
	}

	//首先要切换到下一个进程
	change_proc();

	//如果这个进程不是可运行的,则继续切换
	while(p_proc_ready->state != RUNNABLE){
		//找到下一个是可运行状态的进程
		change_proc();
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
void sys_process_sleep() {
	//参数:睡眠时间
	int mill_seconds = p_proc_ready->regs.ebx;

	//睡眠当前进程
	p_proc_ready->state = SLEEP;
	//需要睡眠的时间片个数 = 睡眠时间ms/一个时间片的时间ms
	p_proc_ready->sleep_ticks = mill_seconds/ms_per_ticks + 1;

	if(DEBUG){
		disp_color_str(p_proc_ready->p_name,BRIGHT);
		disp_color_str("will sleep ticks:",BRIGHT);
		disp_int(p_proc_ready->sleep_ticks,BRIGHT);
	}

	schedule();
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_p(){

	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;

	if(DEBUG){
		disp_color_str(p_proc_ready->p_name,ORANGE);
		disp_color_str(" ask for:",ORANGE);
		disp_color_str(signal->name,ORANGE);
	}

	//申请使用信号量物理值
	signal->value --;

	//如果信号量不可以用
	if(signal->value < 0){

		//则调用此方法的进程阻塞自己,设置自己为等待此信号量状态
		PROCESS * proc_tobe_sleep = p_proc_ready;
		
		//从就绪队列中移走
//		list_remove2(runnable_list,proc_tobe_sleep);

		//无限睡眠直到被唤醒
		proc_tobe_sleep->state = SLEEP;
		proc_tobe_sleep->sleep_ticks = 100000000;

		//移入信号量等待队列
		list_add(signal->waiting_list,proc_tobe_sleep);

		if(DEBUG){
			disp_color_str(proc_tobe_sleep->p_name,ORANGE);
			disp_color_str(" :fail going sleep",ORANGE);
		}
		//释放CPU,转向调度程序
		schedule();

	//信号量可以用
	}else{
		if(DEBUG){
			disp_color_str(p_proc_ready->p_name,ORANGE);
			disp_color_str(" :success",ORANGE);
		}
	}
}

/**
 * 信号量的 PV 操作
 */
void sys_sem_v(){
	//在kernel.asm里面已经关闭了中断,下面的整个过程都是关中断的
	SIGNAL* signal = (SIGNAL *) p_proc_ready->regs.ebx;

	if(DEBUG){
		disp_color_str(p_proc_ready->p_name,GREEN);
		disp_color_str(" release : ",GREEN);
		disp_color_str(signal->name,GREEN);
	}

	//归还信号量物理值
	signal->value ++;

	//有别的进程在等待
	if(signal->value <= 0){
		if(DEBUG){
			disp_color_str(p_proc_ready->p_name,GREEN);
			disp_color_str(" will wakeup:",GREEN);
		}

		//释放第一个等待信号量S的进程
		LIST * list = signal->waiting_list;
		PROCESS * tobe_wake_up = list->first;
		//改为可以运行状态
		tobe_wake_up->state = RUNNABLE;
		tobe_wake_up->sleep_ticks = 0;
//
//		//进入就绪队列
//		list_add(runnable_list,tobe_wake_up);

		//从等待队列中移除
		list_remove(list);

		if(DEBUG){
			disp_color_str(tobe_wake_up->p_name,GREEN);
		}

	//没有进程在等待
	}else{
		if(DEBUG){
			disp_color_str(" wake no one ",GREEN);
		}
	}
	//执行V操作的进程继续执行
}