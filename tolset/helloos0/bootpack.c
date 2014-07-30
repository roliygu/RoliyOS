#include <stdio.h>
#include "bootpack.h"

void task_b_main(struct SHEET *sht_back);


void HariMain(void){
	// BIOS
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	// Others
	int i;
	struct Queue fifo;
	int fifobuf[128];
	// Memory
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int memtotal;
	// Timer
	struct TIMER *timer;
	// Task
	struct TASK *task_a, *task_b[3];
	// Keyboard and Mouse
	char s[40];
	int mx, my,cursor_x,cursor_c;
	struct MOUSE_DEC mdec;
	unsigned char buf_mouse[256];
	// Sheets
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_b[3];
	unsigned char *buf_back, *buf_win, *buf_win_b;

	// 初始化
	init_gdtidt();
	init_pic();
	io_sti();  // IDT/PIC初始化完成，开放CPU中断
	init_pit();
	io_out8(PIC0_IMR, 0xf8); //开放PIT,PIC1和键盘中断
	io_out8(PIC1_IMR, 0xef); //开放鼠标中断
	que_init(&fifo, 128, fifobuf, 0);
	init_palette();
	
	//内存
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	// 多任务
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);
	//计时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	//键鼠
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	mx = (binfo->scrnx - 16) / 2; //让鼠标坐标在正中间
	my = (binfo->scrny - 28 - 16) / 2;
	//图层
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_win   = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); 
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	init_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	for (i = 0; i < 3; i++) {
		sht_win_b[i] = sheet_alloc(shtctl);
		buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
		sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1); /* “§–¾F‚È‚µ */
		sprintf(s, "task_b%d", i);
		init_window8(buf_win_b, 144, 52, s, 0);
		task_b[i] = task_alloc();
		task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 1 * 8;
		task_b[i]->tss.cs = 2 * 8;
		task_b[i]->tss.ss = 1 * 8;
		task_b[i]->tss.ds = 1 * 8;
		task_b[i]->tss.fs = 1 * 8;
		task_b[i]->tss.gs = 1 * 8;
		*((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];
		//task_run(task_b[i], 2, i+1);
	}
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win_b[0], 168,  56);
	sheet_slide(sht_win_b[1],   8, 116);
	sheet_slide(sht_win_b[2], 168, 116);
	sheet_slide(sht_win,        8,  56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back,     0);
	sheet_updown(sht_win_b[0], 1);
	sheet_updown(sht_win_b[1], 2);
	sheet_updown(sht_win_b[2], 3);
	sheet_updown(sht_win,      4);
	sheet_updown(sht_mouse,    5);

	// 显示
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

	for(;;){
		io_cli();   //屏蔽中断
		if(que_status(&fifo) == 0){
			task_sleep(task_a);
			io_sti();
		}
		else{
			i = que_pop(&fifo);
			io_sti();
			if(256 <= i && i <= 511){
				sprintf(s, "%02X", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if(i<0x54+256 && keytable[i-256]!=0 && cursor_x<144){
						s[0]=keytable[i-256],s[1]=0;
						putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_C6C6C6, s, 1);
						cursor_x +=8;
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
			}else if (i <= 1) { 
				if (i != 0) {
					timer_init(timer, &fifo, 0); 
					cursor_c = COL8_000000;
				} else {
					timer_init(timer, &fifo, 1); 
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
			}
		}
	}
}
void task_b_main(struct SHEET *sht_win_b){
	struct Queue fifo;
	struct TIMER *timer_1s;
	int i, fifobuf[128], count =0, count0 = 0;
	char s[12];
	
	que_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for(;;){
		count++;
		io_cli();
		if(que_status(&fifo) == 0){
			io_sti();
		}else{
			i = que_pop(&fifo);
			io_sti();
			if (i == 100) {
				sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(sht_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}