#include "list.h"

void list_add (LIST* list, PROCESS * new_proc){
    if(list->size == 0){
        list->first = new_proc;
        list->last = new_proc;
    }else{
        PROCESS * old_last = list->last;
        list->last = new_proc;
        old_last->next = new_proc;
    }
    list->size ++;
}

void list_remove(LIST* list){
    if(list->size <= 0){
        return;
    }else if(list->size == 1){
        list->first = 0;
        list->last = 0;
    }else{
        PROCESS * new_first = list->first->next;
        list->first->next = 0;
        list->first = new_first;
    }
    list->size --;
}



PUBLIC LIST list_table[3] = {
        //进程就绪队列,低级调度使用
        {"ready_list",0,0,0},
        {"sleep_list",0,0,0},
        {"some_list",0,0,0}
};
