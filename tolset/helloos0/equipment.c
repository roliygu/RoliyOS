#include "bootpack.h"

// 个人理解:鼠标和键盘受"键盘控制电路KBC"(某类似于一个芯片的东西)控制，
// 鼠标和键盘被按下或松开后就触发中断,然后将按键数据暂存于KBC,然后CPU从端口
// 0x0060(KBC接口)把数据取出来

void wait_KBC_sendready(void){
	//等待键盘控制电路完成
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
void init_keyboard(void){
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); // 发出指令表示接下来要设定模式
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE); // 设定鼠标模式
	return;
}
void enable_mouse(void){
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  //表示下一个数据是发给鼠标的
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);  //发给键盘控制器了，结合上一句话，此消息会被转给鼠标
	// 这是键盘控制会返回ACK信息也就是0xfa,不过这里没有管
	return; 
}
