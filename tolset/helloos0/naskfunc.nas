; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式
[INSTRSET "i486p"]				; 这个程序是给486用的
[BITS 32]						; 制作32位模式用的机器语言
[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt,_write_mem8		; 程序中包含的函数名

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET
		