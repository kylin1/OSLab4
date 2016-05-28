;C语言声明:
;从nasm再次封装成新的C语言函数给外部使用
;本质上还就是下面这一段代码在执行

;int my_print(char *charactor, int size, int color);
;名   称:   my_print
;功   能:   按照指定颜色输出给定长度的字符串
;入口参数:   第一个参数 = 字符串的地址(char *)
;           第二个参数 = 字符串长度(int)
;           第三个参数 = 字符串颜色(int)
;出口参数:   无
global my_print

section .data
  colorMsg: db `\033[1;34m0\033[0m`   ;彩色输出字符串 
  ;30:黑 31:红 32:绿 33:黄 34:蓝色 35:紫色 36:深绿 37:白色 
  colorSize: equ $-colorMsg
section .bss
  digit: resb 1 

section .text
  ;linux系统调用,这是我要写的系统调用的目标
  ;这是给用户使用的接口
  ;名   称:   04号输出系统调用宏
  ;功   能:   输出指定长度的字符串
  ;入口参数:   第一个参数 = 字符串的地址
  ;           第二个参数 = 字符串长度
  ;出口参数:   无
  %macro write 2 
    mov   eax, 4
    mov   ebx, 1
    mov   ecx, %1  ;msg
    mov   edx, %2  ;length
    int   80h
  %endmacro

my_print:
  mov ebx, [esp+4]          ;string address
  mov ecx, [esp+8]          ;string length 
  mov eax, [esp+12]         ;color
  cmp eax,0                 ;0表示无颜色,否则带颜色输出
  je without_color
color:
  mov esi,[esp+4]
  add al,48
  mov byte[colorMsg+5],al   ;设置颜色
;彩色输出每一位
print_digit:
  mov dl,byte[esi]
  mov byte[colorMsg+7],dl   ;设置字符
  push ecx
  write colorMsg,colorSize  ;彩色输出这一个字符
  pop ecx
  inc esi                   ;下一个字符
  loop print_digit
  jmp end

;无颜色输出
without_color: 
  write [esp+4],[esp+8]  
end:
  ret
