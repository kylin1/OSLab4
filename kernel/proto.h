
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

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_int(int input);
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
/* proc.c */

/* int_handler */
PUBLIC  void    sys_call();

#endif //MULTIPLEPROCESS_PROTO_H