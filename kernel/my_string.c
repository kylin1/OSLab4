#include "const.h"

PUBLIC int str_length(char *str) {
    int i;
    int size = 0;

    for (i = 0; ; i++) {
        if (str[i] != '\0') {
            size++;
        } else {
            break;
        }
    }

    return size;
}


PUBLIC void int_to_str(const int input, char *str) {
    int num = input;
    int n = num % 10;
    char tmp[20];

    int i = 0;
    while (n > 0) {
        tmp[i++] = (char) (n + '0');
        num = (num - n) / 10;
        n = num % 10;
    }
    tmp[i] = '\0';

    for (i = 0; i <= str_length(tmp) - 1; i++) {
        str[i] = tmp[str_length(tmp) - i - 1];
    }
    str[i] = '\0';
}

/**
 * 查找input字符串中的target字符串
 * 返回结果int * result 第一位代表找到的数目
 * 其余各位代表发现target在input中的下标
 *
 * 例如    char *input = "abcabc123abc234abbc";
 *        char *target = "abc";
 *        则result指向 --> 3,0,3,9 这4个数字,代表在0,3,9这个下标开始发现3个abc
 */
PUBLIC void find_string(char *input, char *target, int *result) {

//    TTY *p_tty_normal = (tty_table);
//    p_tty_normal++;
//	out_string_no_color(p_tty_normal->p_console,"\ninput |",1);
//	out_string_no_color(p_tty_normal->p_console,input,1);
//	out_string_no_color(p_tty_normal->p_console,"|\n",1);
//	out_string_no_color(p_tty_normal->p_console,"target :",1);
//	out_string_no_color(p_tty_normal->p_console,target,1);
//	out_string_no_color(p_tty_normal->p_console,"|\n",1);

    int result_size = 0;
    result[0] = 0;

    int input_len = str_length(input);
    int target_len = str_length(target);

    int i;
    int j;
    for (i = 0; i < input_len; i++) {

        //发现首字母匹配
        if (input[i] == target[0]) {
            int index = i;
            //检查结果
            int match = 1;

            //检查是否匹配target剩余的字母
            for (j = 0; j < target_len; j++) {
                if (input[index] == target[j]) {
                    index++;
                    //发现有不一致的字母
                } else {
                    match = 0;
                }
            }

            //成功匹配目标字符串
            if (match) {
                result[0]++;
                result_size++;
                result[result_size] = i;
            }
        }
        //首字母都不匹配,继续查找下一位
    }
}
