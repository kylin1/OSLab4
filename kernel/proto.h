
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "const.h"
#include "type.h"

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);


/* 以下是系统调用相关 */
/* proc.c */

// 0,返回ticks的数值
PUBLIC  int sys_get_ticks();        /* sys_call */

// 1,接受一个 int 型参数 mill_seconds,调用此 System Call 的进程会在 mill_seconds 毫秒内不被 进程调度函数分配时间片。
PUBLIC int sys_process_sleep(int mill_seconds);

// 2,接受一个 char* str 参数, 打印出 字符串。
PUBLIC int sys_disp_str(char* str);

// 3,信号量的 PV 操作
PUBLIC int sys_sem_p();

// 4,信号量的 PV 操作
PUBLIC int sys_sem_v();



/* int_handler */
PUBLIC  void    sys_call();


/* syscall.asm */
PUBLIC  int     my_get_ticks();
PUBLIC  int     my_process_sleep(int mill_seconds);
PUBLIC  int     my_disp_str(char* str);
PUBLIC  int     my_sem_p();
PUBLIC  int     my_sem_v();

