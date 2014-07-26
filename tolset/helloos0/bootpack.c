#include <stdio.h>
#include "bootpack.h"

extern struct TIMERCTL timerctl;
static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};

void HariMain(void){
	// BIOS
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	// Timer
	struct TIMER *timer, *timer2, *timer3;
	// Keyboard and Mouse
	char s[40];
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
	int i,count=0,cursor_x,cursor_c;
	struct Queue fifo;
	int fifobuf[128];


	// 初始化
	init_gdtidt();
	init_pic();
	io_sti();  // IDT/PIC初始化完成，开放CPU中断
	init_pit();
	io_out8(PIC0_IMR, 0xf8); //开放PIT,PIC1和键盘中断
	io_out8(PIC1_IMR, 0xef); //开放鼠标中断
	que_init(&fifo, 128, fifobuf);
	init_palette();

	//计时器
	timer = timer_alloc();
	timer2 = timer_alloc();
	timer3 = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_init(timer2, &fifo, 3);
	timer_init(timer3, &fifo, 1);
	timer_settime(timer, 1000);
	timer_settime(timer2, 300);
	timer_settime(timer3, 50);
	//键鼠
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
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
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); 
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	init_window8(buf_win, 160, 52, "Window");
	sheet_slide(sht_back, 0, 0);	
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_win,   1);
	sheet_updown(sht_mouse, 2);
	// 显示
	
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	for(;;){
		count++;

		io_cli();   //屏蔽中断
		if(que_status(&fifo) == 0)
			io_stihlt();
		else{
			i = que_pop(&fifo);
			io_sti();
			if(256 <= i && i <= 511){
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if(i < 0x54 + 256){
					if(keytable[i-256]!=0 && cursor_x<144){
						s[0]=keytable[i-256];
						s[1]=0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_C6C6C6, s, 1);
						cursor_x +=8;
					}
				}
				if(i ==256+0x0e && cursor_x > 8){
					putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					cursor_x -= 8;
				}	
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);			
			}else if(512 <= i && i <= 767){
				if(mouse_decode(&mdec, i-512) != 0){
					// 三个字节凑齐就显示出来
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if((mdec.btn & 0x01)!=0)
						s[1] = 'L';
					else if((mdec.btn & 0x02)!=0)
						s[3] = 'R';
					else if((mdec.btn & 0x04)!=0)
						s[2] = 'C';
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					// 鼠标指针移动
					mx+=mdec.x;
					my+=mdec.y;
					mx = (mx>0)?mx:0;
					my = (my>0)?my:0;
					mx = (mx>binfo->scrnx-1)?binfo->scrnx-1:mx;
					my = (my>binfo->scrny-1)?binfo->scrny-1:my;
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if((mdec.btn & 0x01) != 0){
						sheet_slide(sht_win, mx-80, my-8);
					}
				}
			}else if (i == 10) { // data=10的timer
				//putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
				//sprintf(s, "%010d", count);
				//putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);
			} else if (i == 3) { // data=3的timer
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
				count = 0; // ‘ª’èŠJŽn 
			} else if (i == 1) { // data=1的timer
				timer_init(timer3, &fifo, 0); 
				boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			} else if (i == 0) { // 相当于跟i=1反着来，开关来回开的感觉，制造0.5秒闪烁的感觉
				timer_init(timer3, &fifo, 1); 
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111);
				timer_settime(timer3, 50);
				sheet_refresh(sht_back, 8, 96, 16, 112);
			}
		}
	}
}
