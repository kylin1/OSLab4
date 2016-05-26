
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "include/type.h"
#include "include/const.h"
#include "include/protect.h"
#include "include/proto.h"
#include "include/string.h"
#include "include/proc.h"
#include "include/global.h"

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
PUBLIC int sys_get_ticks()
{
	return ticks;
}

