#include "bootpack.h"

// 个人理解:鼠标和键盘受"键盘控制电路KBC"(某类似于一个芯片的东西)控制，
// 鼠标和键盘被按下或松开后就触发中断,然后将按键数据暂存于KBC,然后CPU从端口
// 0x0060(KBC接口)把数据取出来
struct Queue *keyfifo, *mousefifo;
int keydata0, mousedata0;

void inthandler21(int *esp){
	//来自PS/键盘的中断,每按一个键显示两个编码
	//一个表示键被按下，一个表示松开
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);	// 将"0x61"发送给OCW2,表示已经接到IRQ1中断，可以继续接收其他中断
	data = io_in8(PORT_KEYDAT);
	que_push(keyfifo, data+keydata0);
	return;
}
void inthandler2c(int *esp){
	//来自PS/鼠标的中断
	int data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	que_push(mousefifo, data+mousedata0);
	return;
}
void wait_KBC_sendready(void){
	//等待键盘控制电路完成
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}
void init_keyboard(struct Queue *fifo, int data0){
	keyfifo = fifo;
	keydata0 = data0;
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE); // 发出指令表示接下来要设定模式
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE); // 设定鼠标模式
	return;
}
void enable_mouse(struct Queue *fifo, int data0, struct MOUSE_DEC *mdec){
	mousefifo = fifo;
	mousedata0 = data0;
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);  //表示下一个数据是发给鼠标的
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);  //发给键盘控制器了，结合上一句话，此消息会被转给鼠标
	// 这是键盘控制会返回ACK信息也就是0xfa,不过这里没有管
	mdec->phase = 0;
	return; 
}
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat){
	if(mdec->phase==0){
		// 等待鼠标0xfa,即ACK
		if(dat == 0xfa){
			mdec->phase = 1;
		}
		return 0;
	}else if(mdec->phase==1){
		// 等待鼠标第一字节数据
		if((dat&0xc8)==0x08){
			// 如果第一字节正确
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}else if(mdec->phase==2){
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}else if(mdec->phase==3){
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0]&0x07;  // 鼠标按键信息只用取buf[0]低三位即可
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if((mdec->buf[0] & 0x10)!=0){
			mdec->x |= 0xffffff00;  // 因为初始是unsigned char这里转成补码
		}
		if((mdec->buf[0] & 0x20)!=0){
			mdec->y |= 0xffffff00;
		}
		mdec->y = -mdec->y;  // 鼠标往下移动，屏幕上的y坐标反而是增加
		return 1;
	}else
		return -1;
}
