#include "bootpack.h"

void init_pic(void){
	io_out8(PIC0_IMR,  0xff  ); // 禁止所有中断
	io_out8(PIC1_IMR,  0xff  ); // 禁止所有中断

	io_out8(PIC0_ICW1, 0x11  ); // 边沿触发模式
	io_out8(PIC0_ICW2, 0x20  ); // IRQ0-7由INT20-27接收
	io_out8(PIC0_ICW3, 1 << 2); // PIC1由IRQ2连接
	io_out8(PIC0_ICW4, 0x01  ); // 无缓冲区模式

	io_out8(PIC1_ICW1, 0x11  ); // 边沿触发模式
	io_out8(PIC1_ICW2, 0x28  ); // IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3, 2     ); // PIC1由IRQ2连接
	io_out8(PIC1_ICW4, 0x01  ); // 无缓冲区模式

	io_out8(PIC0_IMR,  0xfb  ); // 11111011 PIC1以外全部禁止
	io_out8(PIC1_IMR,  0xff  ); // 11111111 禁止所有中断

	return;
}
unsigned int memtest(unsigned int start, unsigned int end){
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	// 386还是486以上
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; // AC-bit = 1 
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { // 如果是386，即使设定AC=1，AC的值还是会恢复成0 
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0 
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; // 禁止缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; // 允许缓存
		store_cr0(cr0);
	}

	return i;
}
void inthandler27(int *esp){
	io_out8(PIC0_OCW2, 0x67);  
	return;
}
