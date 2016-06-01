#include "list.h"
#include "const.h"
#include "proto.h"



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
//    show_list(list);
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
//    show_list(list);
}

void show_list(LIST* list){
    my_disp_str("list name :",RED);
    my_disp_str(list->name,RED);
    my_disp_str("size :",RED);
    disp_int(list->size);
    PROCESS * this_one = list->first;
    while (this_one != 0){
        my_disp_str(this_one->p_name,RED);
        my_disp_str(" --> ",RED);
        this_one = this_one->next;
    }
    my_disp_str(" list end",RED);
}


