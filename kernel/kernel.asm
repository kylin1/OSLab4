
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               kernel.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


%include "sconst.inc"

; 导入函数
extern	cstart
extern	kernel_main
extern	exception_handler
extern	spurious_irq
extern	clock_handler
extern	disp_str
extern	delay
extern	irq_table ;global.c : PUBLIC	irq_handler		irq_table[NR_IRQ];

; 导入全局变量
extern	gdt_ptr
extern	idt_ptr
extern	p_proc_ready
extern	tss
extern	disp_pos
extern	k_reenter
extern	sys_call_table

bits 32

[SECTION .data]
clock_int_msg		db	"^", 0

[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; 栈顶

[section .text]	; 代码在此

global _start	; 导出 _start

global restart
global sys_call

global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15


_start:
	; 此时内存看上去是这样的（更详细的内存情况在 LOADER.ASM 中有说明）：
	;              ┃                                    ┃
	;              ┃                 ...                ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■Page  Tables■■■■■■┃
	;              ┃■■■■■(大小由LOADER决定)■■■■┃ PageTblBase
	;    00101000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■Page Directory Table■■■■┃ PageDirBase = 1M
	;    00100000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□ Hardware  Reserved □□□□┃ B8000h ← gs
	;       9FC00h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■LOADER.BIN■■■■■■┃ somewhere in LOADER ← esp
	;       90000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■KERNEL.BIN■■■■■■┃
	;       80000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■KERNEL■■■■■■■┃ 30400h ← KERNEL 入口 (KernelEntryPointPhyAddr)
	;       30000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┋                 ...                ┋
	;              ┋                                    ┋
	;           0h ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
	;
	;
	; GDT 以及相应的描述符是这样的：
	;
	;		              Descriptors               Selectors
	;              ┏━━━━━━━━━━━━━━━━━━┓
	;              ┃         Dummy Descriptor           ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_C    (0～4G)     ┃   8h = cs
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_RW   (0～4G)     ┃  10h = ds, es, fs, ss
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_VIDEO                 ┃  1Bh = gs
	;              ┗━━━━━━━━━━━━━━━━━━┛
	;
	; 注意! 在使用 C 代码的时候一定要保证 ds, es, ss 这几个段寄存器的值是一样的
	; 因为编译器有可能编译出使用它们的代码, 而编译器默认它们是一样的. 比如串拷贝操作会用到 ds 和 es.
	;
	;


	; 把 esp 从 LOADER 挪到 KERNEL
	mov	esp, StackTop	; 堆栈在 bss 段中

	mov	dword [disp_pos], 0

	sgdt	[gdt_ptr]	; cstart() 中将会用到 gdt_ptr
	call	cstart		; 在此函数中改变了gdt_ptr，让它指向新的GDT
	lgdt	[gdt_ptr]	; 使用新的GDT

	lidt	[idt_ptr]

	jmp	SELECTOR_KERNEL_CS:csinit
csinit:		; “这个跳转指令强制使用刚刚初始化的结构”——<<OS:D&I 2nd>> P90.

	;jmp 0x40:0
	;ud2

    ;加载tr的代码
	xor	eax, eax
	mov	ax, SELECTOR_TSS
	ltr	ax    ;装载任务状态段寄存器TR,使其指向已预置好的任务的TSS
	;从GDT中取出相应的TSS段描述符，把TSS段描述符的基地址和界限等信息装入TR的高速缓冲寄存器中

	;sti 启动main.c的kernel_main函数,启动线程
	jmp	kernel_main

	;hlt


; 中断和异常 -- 硬件中断
; 使用宏避免了函数的压栈出栈,节省了时间,但是空间上有浪费
; 以空间换取时间
; ---------------------------------
%macro	hwint_master	1
	call	save            ;保存中断处理状态,寄存器的值
	;call函数无法使用ret返回是因为函数调用前后esp的值是完全不同的
	;必须事先将返回地址保存起来,最后使用jmp指令跳转回去

	in	al, INT_M_CTLMASK	; `.
	or	al, (1 << %1)		;  | 屏蔽当前中断
	out	INT_M_CTLMASK, al	; /  避免在处理当前中断的同事发生同样种类的中断

	mov	al, EOI			    ; `. 置EOI位,中断结束命令
	out	INT_M_CTL, al		; /

	sti	                    ; CPU在响应中断的过程中会自动关中断，这句之后就允许响应新的中断

	push	%1			            ; `.
	call	[irq_table + 4 * %1]	;  | 中断处理程序
	pop	ecx			                ; /  这是与当前中断相关的一个例程

	cli                     ;关闭中断

	in	al, INT_M_CTLMASK	; `.
	and	al, ~(1 << %1)		;  | 恢复接受当前中断
	out	INT_M_CTLMASK, al	; /

	;从中断返回?
	ret                     ;要跳转到 _restart处,restart another procs
%endmacro


ALIGN	16
hwint00:		;让时钟中断可以不停地发生而不是一次 Interrupt routine for irq 0 (the clock).
	hwint_master	0

ALIGN	16
hwint01:		; Interrupt routine for irq 1 (keyboard)
	hwint_master	1

ALIGN	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
	hwint_master	2

ALIGN	16
hwint03:		; Interrupt routine for irq 3 (second serial)
	hwint_master	3

ALIGN	16
hwint04:		; Interrupt routine for irq 4 (first serial)
	hwint_master	4

ALIGN	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwint_master	5

ALIGN	16
hwint06:		; Interrupt routine for irq 6 (floppy)
	hwint_master	6

ALIGN	16
hwint07:		; Interrupt routine for irq 7 (printer)
	hwint_master	7

; ---------------------------------
%macro	hwint_slave	1
	push	%1
	call	spurious_irq
	add	esp, 4
	hlt
%endmacro
; ---------------------------------

ALIGN	16
hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwint_slave	8

ALIGN	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
	hwint_slave	9

ALIGN	16
hwint10:		; Interrupt routine for irq 10
	hwint_slave	10

ALIGN	16
hwint11:		; Interrupt routine for irq 11
	hwint_slave	11

ALIGN	16
hwint12:		; Interrupt routine for irq 12
	hwint_slave	12

ALIGN	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwint_slave	13

ALIGN	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave	14

ALIGN	16
hwint15:		; Interrupt routine for irq 15
	hwint_slave	15



; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call	exception_handler
	add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

; ====================================================================================
;                                   save
; ====================================================================================
save:
        pushad          ; `.PUSHAD指令压入32位寄存器，其入栈顺序是:EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI .
        push    ds      ;  |
        push    es      ;  | 保存原寄存器值
        push    fs      ;  |
        push    gs      ; /

        mov     dx, ss  ;让ds = es = ss
        mov     ds, dx
        mov     es, dx

        mov     esi, esp                    ;esi = 进程表起始地址

        inc     dword [k_reenter]           ;k_reenter++;
        cmp     dword [k_reenter], 0        ;if(k_reenter ==0)如果中断重入,执行.1
        jne     .1                          ;{ mov     esp, StackTop

        ;否则不是重入顺序执行
        mov     esp, StackTop               ;  esp切换到内核栈
        push    restart                     ;  push restart


        ;P_STACKBASE = 0 RETADR在寄存器后面
        ;RETADR - P_STACKBASE是执行call save这条指令的时候
        ;压栈的返回地址相对于进程表起始地址的偏移

        ;也就是从save 函数返回,继续从inb INT_CTLMASK开始执行
        ;也就是return

        jmp     [esi + RETADR - P_STACKBASE];


.1:                                         ;} else {
        ;中断重入:已经在内核栈，不需要再切换
        push    restart_reenter             ;  push restart_reenter
        jmp     [esi + RETADR - P_STACKBASE];  return;
                                            ;}


; ====================================================================================
;                                 sys_call
;                   int INT_VECTOR_SYS_CALL 将由sys_call处理
;                           系统调用要到这里被处理
; ====================================================================================
sys_call:
        call    save
        ;call函数无法使用ret返回是因为函数调用前后esp的值是完全不同的
	    ;必须事先将返回地址保存起来,最后使用jmp指令跳转回去

        ;esi = 进程表起始地址
        sti     ;关中断



        push esi



        ;eax是在syscall.asm里面设置好的顺序
        ;调用sys_call_table的第eax个函数
        ;(里面存放了函数名,函数指针数组,例如sys_get_ticks是第0个)
        call    [sys_call_table + eax * 4]

        ;C语言函数调用约定,函数返回值在eax里面
        ;也就是说内核函数的返回值就在eax里面


        pop esi


        ;把函数  [sys_call_table + eax * 4]的返回值(eax)
        ;放在进程表中eax的位置,以便进程P被恢复执行的时候eax中是正确的返回值
        ;esi 在这里还是进程表起始地址
        mov     [esi + EAXREG - P_STACKBASE], eax

        cli     ;开中断
        ret


; ====================================================================================
;				    restart
; ====================================================================================
restart:
    ;这里的选择子必须与protect.h中的值保持一致
    ;p_proc_ready 是进程表指针指向下一个要启动的进程表的地址
	mov	esp, [p_proc_ready]

	;设置LDT,P_LDT_SE  equ P_STACKTOP  esp + P_LDT_SEL指向s_proc结构体中的成员ldt_sel,ldt_sel被别的地方,restart函数之前初始化,以便lldt这一行可以正确执行
	;每一个进程都有自己的LDT,所以进程切换的时候需要重新加载ldt
	lldt	[esp + P_LDT_SEL]

	;将s_proc结构体中的第一个成员regs的末地址赋给下面的数值
	lea	eax, [esp + P_STACKTOP]

	;tss + TSS3_S_SP0是tss结构体中ring()堆栈指针域esp,故下一次中断的时候,esp变成了regs末地址,然后ss,esp,eflags,cs,eip等寄存器被依次从高到低入栈,放到了reg结构体的后半部分中
	mov	dword [tss + TSS3_S_SP0], eax

restart_reenter:
	dec	dword [k_reenter]             ;将k_reenter数值减去1
	pop	gs                            ;返回了4个寄存器
	pop	fs
	pop	es
	pop	ds
	popad                             ;返回了更多寄存器(pushed by save())
	                                  ;这里eax会被恢复成系统调用之后,内核函数的返回值
	                                  ;完成了返回值从内核空间到用户空间的传递

	add	esp, 4                        ;将esp+4,跳过了u32 retaddr,以便执行下一行中断返回指令

	iretd        ;恢复寄存器:pushed by CPU during interrupt, ring0 -> ring1的转移,向外跳转
    ;最后一行指令后,eflags中的值会变成proc -> regs.eflags中的值
    ;由于预先设置了IF位,所以进程开始运行的时候,中段已经被打开了

    ;ring0 -> ring1系统到用户,堆栈切换在这里完成,各种寄存器从堆栈获取
    ;内层堆栈指针存放在当前任务的TSS中,从内层向外层转移时不需要访问TSS，
    ;而只需内层栈中保存的栈指针

    ;但ring1 -> ring0用户到系统(内核态)切换就要使用TSS,使用TSS保存ring0的堆栈信息
    ;外层堆栈指针保存在内层堆栈中,在从外层向内层变换时，要访问TSS

    ;进程在被中断切换到内核态的时候,当前的各个寄存器应该立刻被压栈
    ;也就是说,每个进程在运行的时候,tss.esp0应该是当前进程表中保存寄存器值的地方
    ;这个地方也就是s_proc结构体中s_stackfrme的最高地址处(regs最高处)
    ;这样,进程被挂起后才恰好保存寄存器到正确的位置

    ;进程X想要获得CPU之前,tss.esp0的值就会被修改为进程表X中相应的位置