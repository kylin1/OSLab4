
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
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
                           clock_handler
                          时钟中断处理程序
            main.c里面有put_irq_handler(CLOCK_IRQ, clock_handler);
            也就是disable_irq(irq);
            再为irq_table第irq个数值赋值irq_table[irq] = handler;
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//时钟中断处理数目加1
	ticks++;

	//下一个进程的ticks-1
	p_proc_ready->ticks--;

	//如果中断重入,函数直接返回,不做任何操作
	if (k_reenter != 0) {
		return;
	}

	//当下一个进程还有ticks就返回,使得其他进程不会有机会获得执行
	if (p_proc_ready->ticks > 0) {
		return;
	}

	//下一个进程的ticks变成了0,调用进程调度函数
	schedule();
}

/*======================================================================*
                              milli_delay
                             运行在用户态
                      本质上还是给予分配了 时间片的,
		只不过在分配的时间片里在 mills_delay 函数中什么也没做
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = my_get_ticks();

        while(((my_get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

