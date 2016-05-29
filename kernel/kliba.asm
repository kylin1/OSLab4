
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;			       klib.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;							Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; 导入全局变量
extern	disp_pos


[SECTION .text]

; 导出函数
global	disp_str


global	sys_disp_str


global	disp_color_str
global	out_byte
global	in_byte
global	enable_irq
global	disable_irq
global	enable_int
global	disable_int


; ========================================================================
;		   void sys_disp_str(char * info);
; ========================================================================
sys_disp_str:
	push	ebp         ;保护先前EBP指针(执行函数前,主函数的堆栈指针)
    mov     ebp, esp    ;ESP就是一直指向栈顶的指针

	;取出EBX里面存储的字符串地址到esi寄存器
	;esi是进程表起始地址
	mov	esi, [esi + EBXREG - P_STACKBASE]	; pszInfo

	mov	edi, [disp_pos] ; edi保存当前显示字符串的位置
	mov	ah, 0Fh
.1:
	lodsb               ;把esi指向的存储单元读入累加器AL,然后SI自动增加或减小1或2位
	test	al, al      ;根据and的结果设置flags寄存器的各种标志
	jz	.2              ;ZF=1(表示本次运算结果为0)，则跳转结束
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3      ;不是回车跳转

    ;是回车
	push	eax

	mov	eax, edi        ;当前字符串显示的位置
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax

	pop	eax

	jmp	.1
.3:
	mov	[gs:edi], ax    ;向显存写入字符
	add	edi, 2          ;下一个显示的位置递增2
	jmp	.1

.2:
	mov	[disp_pos], edi ;保存当前显存位置

	pop	ebp             ; 恢复ebp
	ret                 ; 恢复进入函数前的堆栈



; ========================================================================
;		   void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp         ;保护先前EBP指针(执行函数前,主函数的堆栈指针)
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos] ; edi保存当前显示字符串的位置
	mov	ah, 0Fh
.1:
	lodsb               ;把esi指向的存储单元读入累加器AL,然后SI自动增加或减小1或2位
	test	al, al      ;根据and的结果设置flags寄存器的各种标志
	jz	.2              ;ZF=1(表示本次运算结果为0)，则跳转结束
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3      ;不是回车跳转

	push	eax
	mov	eax, edi        ;当前字符串显示的位置
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax

	jmp	.1
.3:
	mov	[gs:edi], ax    ;向显存写入字符
	add	edi, 2          ;下一个显示的位置递增2
	jmp	.1

.2:
	mov	[disp_pos], edi ;保存当前显存位置

	pop	ebp             ; 恢复ebp
	ret                 ; 恢复进入函数前的堆栈

; ========================================================================
;		   void disp_color_str(char * info, int color);
; ========================================================================
disp_color_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12]	; color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

; ========================================================================
;		   void out_byte(u16 port, u8 value);
; ========================================================================
out_byte:
	mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;		   u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx ;把一个字节或一个字由一个输入端口（port），传送至AL
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;		   void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;	}
disable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl			; ah = (1 << (irq % 8))
	cmp	cl, 8
	jae	disable_8		; disable irq >= 8 at the slave 8259
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
dis_already:
	popf
	xor	eax, eax		; already disabled
	ret

; ========================================================================
;		   void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;	}
;
enable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl			; ah = ~(1 << (irq % 8))
	cmp	cl, 8
	jae	enable_8		; enable irq >= 8 at the slave 8259
enable_0:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al	; clear bit at master 8259
	popf
	ret
enable_8:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al	; clear bit at slave 8259
	popf
	ret

; ========================================================================
;		   void disable_int();
; ========================================================================
disable_int:
	cli
	ret

; ========================================================================
;		   void enable_int();
; ========================================================================
enable_int:
	sti
	ret


