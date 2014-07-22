#include <stdio.h>
#include "bootpack.h"

extern struct Queue keyfifo, mousefifo;

#define EFLAGS_AC_BIT      0x00040000
#define CR0_CACHE_DISABLE  0x60000000
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

void HariMain(void){
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my,i;
	struct MOUSE_DEC mdec;

	init_gdtidt();
	init_pic();
	io_sti();  // IDT/PIC初始化完成，开放CPU中断
	que_init(&keyfifo, 32, keybuf);
	que_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); //开放PIC1和键盘中断
	io_out8(PIC1_IMR, 0xef); //开放鼠标中断
	
	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	//enable_mouse();
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
	sprintf(s, "memory %dMB", i);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for(;;){
		io_cli();   //屏蔽中断
		if(que_status(&keyfifo) + que_status(&mousefifo)==0){
			io_stihlt();
		}else{
			if (que_status(&keyfifo) != 0) {
				i = que_pop(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			} else if (que_status(&mousefifo) != 0) {
				i = que_pop(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec, i) != 0){
					// 三个字节凑齐就显示出来
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					s[1]='l',s[2]='c',s[3]='r';
					if((mdec.btn & 0x01)!=0)
						s[1] = 'L';
					else if((mdec.btn & 0x02)!=0)
						s[3] = 'R';
					else if((mdec.btn & 0x04)!=0)
						s[2] = 'C';
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					// 鼠标指针移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx+15, my+15);
					mx+=mdec.x;
					my+=mdec.y;
					mx = (mx>0)?mx:0;
					my = (my>0)?my:0;
					mx = (mx>binfo->scrnx-16)?binfo->scrnx-16:mx;
					my = (my>binfo->scrny-16)?binfo->scrny-16:my;
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}

