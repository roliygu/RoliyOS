#include <stdio.h>
#include "bootpack.h"

extern struct Queue keyfifo, mousefifo;

void HariMain(void){
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my,i;

	init_gdtidt();
	init_pic();
	io_sti();
	que_init(&keyfifo, 32, keybuf);
	que_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef);
	
	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	enable_mouse();

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
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 47, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
			}
		}
	}
}

