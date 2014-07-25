#include <stdio.h>
#include "bootpack.h"

extern struct Queue keyfifo, mousefifo;
extern struct TIMERCTL timerctl;

void HariMain(void){
	// BIOS
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	// Timer
	struct Queue timerfifo,timerfifo2,timerfifo3;
	struct TIMER *timer, *timer2, *timer3;
	char timerbuf[8],timerbuf2[8],timerbuf3[8];
	// Keyboard and Mouse
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my;
	struct MOUSE_DEC mdec;
	unsigned char buf_mouse[256];
	// Memory
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	// Sheets
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, *buf_win;
	// Others
	int i,count=0;

	// 初始化
	init_gdtidt();
	init_pic();
	io_sti();  // IDT/PIC初始化完成，开放CPU中断
	init_pit();
	io_out8(PIC0_IMR, 0xf8); //开放PIT,PIC1和键盘中断
	io_out8(PIC1_IMR, 0xef); //开放鼠标中断

	//计时器
	que_init(&timerfifo, 8, timerbuf);
	que_init(&timerfifo2, 8, timerbuf2);
	que_init(&timerfifo3, 8, timerbuf3);
	timer = timer_alloc();
	timer2 = timer_alloc();
	timer3 = timer_alloc();
	timer_init(timer, &timerfifo, 1);
	timer_init(timer2, &timerfifo2, 1);
	timer_init(timer3, &timerfifo3, 1);
	timer_settime(timer, 1000);
	timer_settime(timer2, 300);
	timer_settime(timer3, 50);
	//键鼠
	que_init(&keyfifo, 32, keybuf);
	que_init(&mousefifo, 128, mousebuf);
	init_keyboard();
	enable_mouse();
	mx = (binfo->scrnx - 16) / 2; //让鼠标坐标在正中间
	my = (binfo->scrny - 28 - 16) / 2;
	//内存
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	//图层
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win   = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* “§–¾F‚È‚µ */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); /* “§–¾F‚È‚µ */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	init_window8(buf_win, 160, 52, "counter");
	sheet_slide(sht_back, 0, 0);	
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2);
	// 显示
	init_palette();
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

	for(;;){
		sprintf(s, "%010d", timerctl.count);
		boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);

		io_cli();   //屏蔽中断
		if(que_status(&keyfifo) + que_status(&mousefifo)+
			que_status(&timerfifo)+que_status(&timerfifo2)+que_status(&timerfifo3)==0){
			io_sti();
		}else{
			if (que_status(&keyfifo) != 0) {
				i = que_pop(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);
			} else if (que_status(&mousefifo) != 0) {
				i = que_pop(&mousefifo);
				io_sti();
				if(mouse_decode(&mdec, i) != 0){
					// 三个字节凑齐就显示出来
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if((mdec.btn & 0x01)!=0)
						s[1] = 'L';
					else if((mdec.btn & 0x02)!=0)
						s[3] = 'R';
					else if((mdec.btn & 0x04)!=0)
						s[2] = 'C';
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32+15*8-1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 32, 16, 32+15*8, 32);
					// 鼠标指针移动
					mx+=mdec.x;
					my+=mdec.y;
					mx = (mx>0)?mx:0;
					my = (my>0)?my:0;
					mx = (mx>binfo->scrnx-1)?binfo->scrnx-1:mx;
					my = (my>binfo->scrny-1)?binfo->scrny-1:my;
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 0, 0, 80, 16); 
					sheet_slide(sht_mouse, mx, my);
				}
			}else if(que_status(&timerfifo)!=0){
				i = que_pop(&timerfifo);
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 64, COL8_FFFFFF, "sec[10]");
				sheet_refresh(sht_back, 0, 64, 56, 80);
			}else if(que_status(&timerfifo2)!=0){
				i = que_pop(&timerfifo2);
				io_sti();
				putfonts8_asc(buf_back, binfo->scrnx, 0, 80, COL8_FFFFFF, "sec[3]");
				sheet_refresh(sht_back, 0, 80, 48, 96);
			}else if(que_status(&timerfifo3)!=0){
				i = que_pop(&timerfifo3);
				io_sti();
				if(i!=0){
					timer_init(timer3, &timerfifo3, 0);
					boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				}else{
					timer_init(timer3, &timerfifo3, 1);
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
				}
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}