/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

int DEBUG = 1;
int NOT_DEBUG = 0;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define TTY_FIRST    (tty_table)
#define TTY_END        (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY *p_tty);

PRIVATE int tty_do_read(TTY *p_tty);

PRIVATE void tty_do_write(TTY *p_tty,int color);

PRIVATE void put_key(TTY *p_tty, u32 key);

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY *p_tty) {
    //当前缓存大小为0
    p_tty->inbuf_count = 0;
    //头尾指针都是输入缓冲区的起点
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    //初始化TTY对应的console信息
    init_screen(p_tty);
}

/*======================================================================*
                           task_tty
                        不断调用读写操作
                      task_table[NR_TASKS]的第0项
 *======================================================================*/
PUBLIC void task_tty() {
    TTY *p_tty;
    //初始化TTY任务
    init_keyboard();
    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }

    //默认显示第一个控制台
    select_console(1);
    p_tty = TTY_FIRST; p_tty++;

    //初始化时间
    int clear_second = 15;
    int milli_sec = 10000 * clear_second;
    int start_time = my_get_ticks();

    //无限循环处理TTY的读写操作
    while (1) {

        //1.控制台读取输入的字符并处理
        //2.tty_do_read    -->  keyboard_read   读取键盘缓存
        //3.keyboard_read  -->  in_process      处理输入的一个字符,缓存字符
        tty_do_read(p_tty);
        //4.输出TTY中的缓存字符到控制台
        tty_do_write(p_tty,0);

        //判断当前时间是否比起始时间多了clear_second秒
        if( ((my_get_ticks() - start_time) * 1000 / HZ) >= milli_sec ){
            //如果不在搜索模式中,clear_second结束,清空屏幕与缓存
            if(p_tty->p_console->in_search == 0){
                clear_console(p_tty->p_console);
                clear_console_cache(p_tty->p_console);
                //重新设置起始时间
                start_time = my_get_ticks();
            }//否则在搜索模式中,不会清空屏幕
        }

        //在搜索模式中,重启起始时间,使得退出搜索之后还有20秒
        if(p_tty->p_console->in_search == 1){
            start_time = my_get_ticks();
        }
    }
}



/*======================================================================*
				in_process
				//3.让当前控制台处理输入的一个字符
				让一个TTY处理当前的一个字符
 *======================================================================*/
PUBLIC void in_process(TTY *p_tty, u32 key) {
    int is_search = p_tty->p_console->in_search;

    //两个字符的字符串
    char output[2] = {'\0', '\0'};

    int raw_code = key & MASK_RAW;

    //如果进入只接受ESC的模式,且输入的不是ESC,则不处理
    if(p_tty->p_console->only_esc == 1 && raw_code!= ESC){
        return;
    }//否则可以向下进行

    //可打印字符,缓存区加入字符
    if (!(key & FLAG_EXT)) {
        put_key(p_tty, key);
    }

        //不可打印字符:控制性字符
    else {
        switch (raw_code) {
            //回车
            case ENTER:{
                //当前不是搜索模式
                if(is_search == 0){
                    put_key(p_tty, '\n');

                }else{
                    //搜索模式按下回车, 所有匹配的文本(区分大小写)以颜色显示
                    int search_result[50];
                    find_string(p_tty->p_console->cache_char,p_tty->p_console->target,search_result);

                    //目标字符串的长度
                    int target_len = str_length(p_tty->p_console->target);
                    //第一位代表发现的结果数目
                    int found_size = search_result[0];

                    int i;
                    //对于每一个发现的结果,将这个字符串变换颜色
                    for(i = 1; i <= found_size;i++){
                        //其余代表结果的起始的位置
                        int start_pos = search_result[i];
                        int index = start_pos;

                        //变换目标字符串长度个字符
                        int count;
                        for(count = 0;count<target_len;count ++){
                            p_tty->p_console->cache_color[index] = 1;
                            index++;
                        }
                    }
                    //清空屏幕
                    clear_console(p_tty->p_console);
                    //显示搜索结果
                    out_string(p_tty->p_console,p_tty->p_console->cache_char,
                               p_tty->p_console->cache_color,DEBUG);
                    out_string_color(p_tty->p_console,p_tty->p_console->target,DEBUG);

                    //并屏蔽除ESC之外任 何输入
                    p_tty->p_console->only_esc = 1;
                }
                break;
            }

                //退格
            case BACKSPACE:
                //不是搜索模式才退格
                if(is_search == 0){
                    put_key(p_tty, '\b');
                }
                break;

                //TAB
            case TAB:{
                put_key(p_tty, '\t');
                break;
            }

                //搜索功能
            case ESC:{
                char* str;
                //当前不是搜索模式,按ESC键进入查找模式
                if(is_search == 0){
                    p_tty->p_console->in_search = 1;
                    str = "\n~in search mood~\n";

                }else{
                    //再次按下ESC表示退出搜索模式
                    p_tty->p_console->in_search = 0;
                    //只接受ESC的模式也结束
                    p_tty->p_console->only_esc = 0;

                    //清空屏幕,但不清除原有的输入缓存
                    clear_console(p_tty->p_console);

                    //清空已有的缓存信息
                    p_tty->p_console->target[0] = '\0';
                    p_tty->p_console->current_target_pos = 0;

                    //所有文本恢复同一颜色
                    int index;
                    for(index = 0; index<p_tty->p_console->current_char_pos; index++){
                        p_tty->p_console->cache_color[index] = 0;
                    }

                    //显示原来的缓存信息,光标回到正确位置
                    out_string(p_tty->p_console,p_tty->p_console->cache_char,
                               p_tty->p_console->cache_color,DEBUG);

                }
                break;
            }

                //手动清空键盘
            case F3:
                clear_console(p_tty->p_console);
                clear_console_cache(p_tty->p_console);
                break;

            default:
                break;
        }
    }
}

/*======================================================================*
			      put_key
			      向TTY的缓存区加入一个字符
*======================================================================*/
PRIVATE void put_key(TTY *p_tty, u32 key) {
    //如果缓存区还有空间
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
        //把这个字符加入缓存区的下一个空闲位置
        *(p_tty->p_inbuf_head) = key;
        //空闲位置指针偏移
        p_tty->p_inbuf_head++;

        //如果下一个字符的位置等于 TTY输入缓冲区起始地址加容量
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            //下一个字符变为缓冲地址起点
            p_tty->p_inbuf_head = p_tty->in_buf;
        }

        //已缓存的数量递增
        p_tty->inbuf_count++;
    }
}


/*======================================================================*
			      tty_do_read
			      如果这个TTY对应当前的控制台,则让它读取输入的字符
 *======================================================================*/
PRIVATE int tty_do_read(TTY *p_tty) {
    int result = -1;
    //如果这个TTY对应当前的控制台,则让它读取输入
    if (is_current_console(p_tty->p_console)) {
        //传入TTY自己的指针,让read知道是谁在调用读取
        result = keyboard_read(p_tty);
    }
    return result;
}


/*======================================================================*
			      tty_do_write
			      输出一个TTY中的缓存字符到对应的控制台
 *======================================================================*/
PRIVATE void tty_do_write(TTY *p_tty,int color) {

    // 缓冲区中已经填充的字符数量大于0,还有字符没处理
    if (p_tty->inbuf_count) {
        //获取指向键盘任务应处理的键值
        char ch = *(p_tty->p_inbuf_tail);
        //指针前移,指向下一个要输出的字符
        p_tty->p_inbuf_tail++;

        //如果下一个字符的位置等于 TTY输入缓冲区起始地址加容量
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            //下一个字符变为缓冲地址起点
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }

        //缓冲区中已经填充的字符数量递减
        p_tty->inbuf_count--;

        //在TTY对应的窗口中输出字符
        out_char(p_tty->p_console, ch,color,0);
    }
}


#pragma clang diagnostic pop