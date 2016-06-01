
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#include "const.h"
#include "global.h"
#include "list.h"
#include "proto.h"
#include "klib.h"

/*======================================================================*
                           clock_handler
                          时钟中断处理程序
            main.c里面有put_irq_handler(CLOCK_IRQ, clock_handler);
            也就是先disable_irq(irq);
            再为irq_table第irq个数值赋值irq_table[irq] = handler;
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//时钟中断处理数目加1
	ticks++;

	//当前进程的ticks-1
	p_proc_ready->ticks--;

	//如果中断重入,函数直接返回,不做任何操作
	if (k_reenter != 0) {
		return;
	}

	//调用进程调度函数
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

PUBLIC void init_clock() {
	//初始化 8253 PIT,去改变时钟中断的频率
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	/*
	 * 下面是开启一个中断的两个步骤
	 * put_irq_handler(KEYBOARD_IRQ, keyboard_handler);设定键盘中断处理程序
	 * enable_irq(KEYBOARD_IRQ);                     开键盘中断
	 * */

	//1.设定时钟中断处理程序,为irq_table第CLOCK_IRQ(0)个数值赋值,对应的处理程序是
	//clock_handler函数:PUBLIC void clock_handler(int irq)
	put_irq_handler(CLOCK_IRQ, clock_handler);

	//2.让8259A可以接收时钟中断
	enable_irq(CLOCK_IRQ);
	/* 让8259A可以接收时钟中断 */
}