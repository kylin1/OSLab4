
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifndef MULTIPLEPROCESS_PROTO_H
#define MULTIPLEPROCESS_PROTO_H


#include "const.h"
#include "type.h"
#include "tty.h"
#include "console.h"
#include "list.h"

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

/* kliba.asm */
PUBLIC void disable_int();
PUBLIC void enable_int();

/* kernel.asm */
void restart();

/* main.c */
void TestA();
//自己定义的进程任务
void TaskB();
void TaskC();
void TaskD();
void TaskE();

/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);

/* keyboard.c */
PUBLIC void init_keyboard();

/* tty.c */
PUBLIC void task_tty();
PUBLIC void in_process(TTY* p_tty, u32 key);

/* console.c */
PUBLIC void out_char(CONSOLE* p_con, char ch, int color,int debug);
PUBLIC void scroll_screen(CONSOLE* p_con, int direction);


/* 以下是系统调用相关 */

/* syscall.asm */
PUBLIC  int     my_get_ticks();
PUBLIC  int     my_process_sleep(int mill_seconds);
PUBLIC  int     my_disp_str(char* str,int color);
PUBLIC  int     my_sem_p(SIGNAL* signal);
PUBLIC  int     my_sem_v(SIGNAL* signal);


/* proc.c */
// 0,返回ticks的数值
PUBLIC int sys_get_ticks();

// 1,接受一个 int 型参数 mill_seconds,调用此 System Call 的进程会在 mill_seconds 毫秒内不被 进程调度函数分配时间片。
PUBLIC void sys_process_sleep();

// 2,接受一个 char* str 参数, 打印出 字符串。
PUBLIC void sys_disp_str();

// 3,信号量的 PV 操作
PUBLIC void sys_sem_p();
// 4,信号量的 PV 操作
PUBLIC void sys_sem_v();


/* int_handler */
PUBLIC  void    sys_call();

#endif //MULTIPLEPROCESS_PROTO_H