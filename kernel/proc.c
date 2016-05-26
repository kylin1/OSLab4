
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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


/*======================================================================*
                              schedule
                            进程调度函数
                         被clock_handler调用
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {

		//对于每一个进程
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			//如果这个进程剩余的ticks比最大的大
			if (p->ticks > greatest_ticks) {

				//最大剩余ticks = 这个进程的值
				greatest_ticks = p->ticks;
				//下一个进程设为p
				p_proc_ready = p;
			}
		}
		//循环结束之后,下一个进程就是剩余ticks最大的进程

//		//当剩余最大的ticks就是0了,所有的进程都需要再次分配
//		if (!greatest_ticks) {
//			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
//				//ticks设置为初始值
//				p->ticks = p->priority;
//			}
//		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks() {
	return ticks;
}

/**
 * 调用此 System Call 的进程会在 mill_seconds 毫秒内不被进程调度函数分配时间片
 */
PUBLIC int sys_process_sleep(int mill_seconds) {
	return 11;
}

/**
 * 接受一个 char* str 参数, 打印出 字符串
 */
PUBLIC int sys_disp_str(char* str){
	return 22;
}

/**
 * 信号量的 PV 操作
 */
PUBLIC int sys_sem_p(){
	return 33;
}

/**
 * 信号量的 PV 操作
 */
PUBLIC int sys_sem_v(){
	return 44;
}