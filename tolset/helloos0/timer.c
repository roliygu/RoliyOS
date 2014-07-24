#include "bootpack.h"

struct TIMERCTL timerctl;
int i=0;

void init_pit(){
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	return;
}
void inthandler20(int *esp){
	io_out8(PIC0_OCW2, 0x60);  // 接收完IRQ-00信号通知PIC
	if(i==999){
		timerctl.count++;
		i=0;
	}		
	else
		i++;
	return ;
}