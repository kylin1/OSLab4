//
// Created by 王梦麟 on 16/6/1.
//

#ifndef MULTIPLEPROCESS_MY_LIST_H
#define MULTIPLEPROCESS_MY_LIST_H

#include "proc.h"

//队列数据结构
typedef struct proc_list {
    //名称
    char name[32];

    PROCESS * first;

    PROCESS * last;

    int size;

}LIST;

//信号量结构体
typedef struct semaphore {
    //名称
    char name[32];

    //信号量数值
    int	value;

    //信号量等待队列
    LIST * waiting_list;

}SIGNAL;

void list_add (LIST* list, PROCESS * new_proc);

void list_remove(LIST* list);


/* syscall.asm */
PUBLIC  int     my_get_ticks();
PUBLIC  int     my_process_sleep(int mill_seconds);
PUBLIC  int     my_disp_str(char* str,int color);
PUBLIC  int     my_sem_p(SIGNAL* signal);
PUBLIC  int     my_sem_v(SIGNAL* signal);


extern LIST list_table[];

#endif //MULTIPLEPROCESS_MY_LIST_H