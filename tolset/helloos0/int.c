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

struct Queue keyfifo，mousefifo;

void inthandler21(int *esp){
	//来自PS/键盘的中断,每按一个键显示两个编码
	//一个表示键被按下，一个表示松开
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	// 将"0x61"发送给OCW2,表示已经接到IRQ1中断，可以继续接收其他中断
	data = io_in8(PORT_KEYDAT);
	que_push(&keyfifo, data);
	return;
}
void inthandler2c(int *esp){
	//来自PS/鼠标的中断
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	que_push(&mousefifo, data);
	return;
}
void inthandler27(int *esp){
	io_out8(PIC0_OCW2, 0x67);  
	return;
}
