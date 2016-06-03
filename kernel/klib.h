//
// Created by 王梦麟 on 16/5/25.
//

#include "const.h"

#ifndef MULTIPLEPROCESS_KLIB_H
#define MULTIPLEPROCESS_KLIB_H

PUBLIC void enable_irq(int irq);
PUBLIC void disp_int(int input,int color);
PUBLIC void delay(int time);
PUBLIC char * itoa(char * str, int num);

#endif //MULTIPLEPROCESS_KLIB_H

