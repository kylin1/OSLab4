
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "include/type.h"
#include "include/const.h"
#include "include/proto.h"
#include "include/global.h"


/*======================================================================*
                           clock_handler
 *======================================================================*/
//时钟中断处理程序
PUBLIC void clock_handler(int irq)
{
	ticks++;
	p_proc_ready->ticks--;

	//如果中断重入,函数直接返回,不做任何操作
	if (k_reenter != 0) {
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;
	}

	schedule();

}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

