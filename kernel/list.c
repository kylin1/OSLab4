#include "list.h"
#include "const.h"
#include "proto.h"
#include "klib.h"

void print_list(LIST *list) {
    my_disp_str("size :",RED);
    disp_int(list->size,RED);
    PROCESS *this_one = list->first;
    PROCESS *last = list->last;
    disp_str("first:");
    disp_str(this_one->p_name);
    disp_str("last:");
    disp_str(last->p_name);
    while (this_one != 0) {
        disp_str(this_one->p_name);
        disp_str(" --> ");
        this_one = this_one->next;
    }
}


void list_add(LIST *list, PROCESS *new_proc) {

    if (list->size == 0) {
        list->first = new_proc;
        list->last = new_proc;
    } else {
        PROCESS *old_last = list->last;
        list->last = new_proc;
        old_last->next = new_proc;
    }
//    new_proc->next = 0;
    list->size++;
//    print_list(list);
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
//    print_list(list);
}

void list_remove_last(LIST *list){
    PROCESS *this_one = list->first;
    while (this_one->next != 0) {
        //下一个就是要被删除的队尾
        if (this_one->next->pid == list->last->pid) {
            //新的队尾是这一个
            PROCESS *new_last = this_one;
            new_last->next = 0;
            //清楚要被删除的节点的下一个
            list->last->next = 0;
            //产生新的队尾
            list->last = new_last;
            list->size--;
            return;
        }
        this_one = this_one->next;
    }
}

void list_remove2(LIST *list, PROCESS *to_delete) {
    if (list->size <= 0) {
        return;
        //要删除的是队首
    } else if (list->first->pid == to_delete->pid) {
        list_remove(list);
        return;
        //要删除的是队尾
    } else if (list->last->pid == to_delete->pid) {
        PROCESS *this_one = list->first;
        while (this_one->next != 0) {
            //下一个就是要被删除的队尾
            if (this_one->next->pid == list->last->pid) {
                //新的队尾是这一个
                PROCESS *new_last = this_one;
                new_last->next = 0;
                //清楚要被删除的节点的下一个
                list->last->next = 0;
                //产生新的队尾
                list->last = new_last;
                list->size--;
                return;
            }
            this_one = this_one->next;
        }
    } else {
        //遍历链表查询删除
        PROCESS *this_one = list->first;
        while (this_one->next != 0) {
            PROCESS *this_next = this_one->next;

            //要删除下一个
            if (this_next->pid == to_delete->pid) {
                //清除被删除的节点的下一个连接
                this_next->next = 0;
                //跳过被删除的目标
                PROCESS *new_this_next = this_next->next;
                this_one->next = new_this_next;
                //删除了目标,返回
                list->size--;
                return;
            }
            //继续查找目标
            this_one = this_one->next;
        }
    }
}


void show_list(LIST* list){
    my_disp_str("list name :",RED);
    my_disp_str(list->name,RED);
    my_disp_str("size :",RED);
    disp_int(list->size,RED);
    PROCESS * this_one = list->first;
    while (this_one != 0){
        my_disp_str(this_one->p_name,RED);
        my_disp_str(" --> ",RED);
        this_one = this_one->next;
    }
    my_disp_str(" list end",RED);
}


