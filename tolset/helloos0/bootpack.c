#include <stdio.h>
#include "bootpack.h"

void console_task(struct SHEET *sheet);

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
	struct TASK *task_a, *task_cons;
	// Keyboard and Mouse
	char s[40];
	int mx, my, cursor_x, cursor_c, key_to=0, key_shift=0, 
		key_leds=(binfo->leds>>4)&7, keycmd_wait=-1, keycmd_buf[32];
	struct MOUSE_DEC mdec;
	struct Queue keycmd;
	unsigned char buf_mouse[256];
	// Sheets
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, *buf_win, *buf_cons;

	// 初始化
	init_gdtidt();
	init_pic();
	io_sti();  						// IDT/PIC初始化完成，开放CPU中断
	init_pit(); 					// 计时器
	que_init(&fifo, 128, fifobuf, 0);
	init_keyboard(&fifo, 256);		// 键盘
	init_mouse(&fifo, 512, &mdec);  // 鼠标
	io_out8(PIC0_IMR, 0xf8); //开放PIT,PIC1和键盘中断
	io_out8(PIC1_IMR, 0xef); //开放鼠标中断
	que_init(&keycmd, 32, keycmd_buf, 0);
	init_palette();
	
	//内存
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	// 多任务
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	//计时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);
	//键鼠
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;
	//图层
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	// 四个图层
	// 1. background
	sht_back  = sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); 
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	sheet_slide(sht_back, 0, 0);
	sheet_updown(sht_back,  0);	
	// 2. window_cons
	sht_cons  = sheet_alloc(shtctl);
	buf_cons  = (unsigned char *) memman_alloc_4k(memman, 256*165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	init_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	sheet_slide(sht_cons, 32, 4);
	sheet_updown(sht_cons, 1);
	// 3. window_one
	sht_win   = sheet_alloc(shtctl);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1); 
	init_window8(buf_win, 144, 52, "Window", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	sheet_slide(sht_win, 8, 56);
	sheet_updown(sht_win, 2);
	// 4. mouse
	sht_mouse = sheet_alloc(shtctl);
	// 鼠标图层不用额外分配buf
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);	
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_mouse, 3);
	// ******给console分配一个进程******
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	task_run(task_cons, 2, 2);
	// ******=====================******
	// 显示
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	que_push(&keycmd, KEYCMD_LED);
	que_push(&keycmd, key_leds);

	for(;;){
		if(que_status(&keycmd)>0 && keycmd_wait<0){
			// 如果存在向键盘控制器发送的数据
			keycmd_wait = que_pop(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();   
		if(que_status(&fifo) == 0){
			task_sleep(task_a);
			io_sti();
		}else{
			i = que_pop(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { // 键盘数据
				if (i < 0x80 + 256) { // 将按键编码转换为字符编码
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				}else{
					s[0]=0;
				}
				if('A'<=s[0] && s[0]<='Z'){
					if(((key_leds & 4) == 0 && key_shift == 0) ||
							((key_leds & 4) != 0 && key_shift != 0)){
						s[0] += 0x20;
					}
				}
				if (s[0] != 0) { // 一般字符
					if (key_to == 0) {	// 发送给任务A
						if (cursor_x < 128) {
							// 显示一个字符之后将光标后移一位
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	// 发送给命令行窗口
						que_push(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {	// 退格键
					if (key_to == 0) {	
						if (cursor_x > 8) {
							
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	
						que_push(&task_cons->fifo, 8 + 256);
					}
				}
				if (i == 256 + 0x0f) {	// Tab 
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x+7, 43);
						que_push(&task_cons->fifo, 2); // 命令行窗口光标ON
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;
						que_push(&task_cons->fifo, 3);
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				if (i == 256 + 0x2a) {	// 左Shift ON 
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	// 右Shift ON 
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	// 左Shift OFF 
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6) {	// 右Shift OFF 
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a) {	// CapsLock 
					key_leds ^= 4;
					que_push(&keycmd, KEYCMD_LED);
					que_push(&keycmd, key_leds);
				}
				if (i == 256 + 0x45) {	// NumLock 
					key_leds ^= 2;
					que_push(&keycmd, KEYCMD_LED);
					que_push(&keycmd, key_leds);
				}
				if (i == 256 + 0x46) {	// ScrollLock 
					key_leds ^= 1;
					que_push(&keycmd, KEYCMD_LED);
					que_push(&keycmd, key_leds);
				}
				if (i == 256 + 0xfa) {	// 键盘成功接收到数据
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe) {	// 键盘没有成功接收到数据
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
				if(i==256+0x1c)
					if(key_to!=0)
						que_push(&task_cons->fifo, 10+256);
				if(cursor_c>=0){
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}else if(512 <= i && i <= 767){
				if(mouse_decode(&mdec, i-512) != 0){
					// 
				}
			}else if (i <= 1) { // 光标用定时器
				if (i != 0) {
					timer_init(timer, &fifo, 0); 
					if(cursor_c>=0)
						cursor_c = COL8_000000;
				} else {
					timer_init(timer, &fifo, 1); 
					if(cursor_c>=0)
						cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);
				if(cursor_c>=0){
						boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
				}
			}
		}
	}
}
void console_task(struct SHEET *sheet){
	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, x, y, fifobuf[128], cursor_x = 16, cursor_y=28, cursor_c = -1;
	char s[30], cmdline[30];
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	que_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	// 显示提示符
	putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (que_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = que_pop(&task->fifo);
			io_sti();
			if (i <= 1) { // 光标用定时器
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); // 接下来置0
					if(cursor_c>=0)
						cursor_c = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1); // 接下来置1
					if(cursor_c>=0)
						cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
			}
			if(i==2)
				cursor_c = COL8_FFFFFF;
			if(i==3){
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x+7, 43);
				cursor_c = -1;
			}
			if (256 <= i && i <= 511) { // 键盘数据(通过任务A)
				if (i == 8 + 256) {
					// 退格键
					if (cursor_x > 16) {
						// 用空白擦除光标后将光标前移一位
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				}else if(i==10+256) {
					/* Enter */
					/* ƒJ[ƒ\ƒ‹‚ðƒXƒy[ƒX‚ÅÁ‚· */
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					if (cursor_y < 28 + 112) {
						cursor_y += 16; /* ŽŸ‚Ìs‚Ö */
					} else {
						/* ƒXƒNƒ[ƒ‹ */
						for (y = 28; y < 28 + 112; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
							}
						}
						for (y = 28 + 112; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
					}
					/* ƒvƒƒ“ƒvƒg•\Ž¦ */
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				}else{
					// 一般字符
					if (cursor_x < 240) {
						// 显示一个字符之后将光标后移一位
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			// 重新显示光标
			if(cursor_c>=0)
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, 44);
		}
	}
}
