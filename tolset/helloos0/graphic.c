#include "bootpack.h"

void init_palette(void){
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	//  0:黑
		0xff, 0x00, 0x00,	//  1:亮红 
		0x00, 0xff, 0x00,	//  2:亮绿 
		0xff, 0xff, 0x00,	//  3:亮黄 
		0x00, 0x00, 0xff,	//  4:亮蓝 
		0xff, 0x00, 0xff,	//  5:亮紫
		0x00, 0xff, 0xff,	//  6:浅亮蓝
		0xff, 0xff, 0xff,	//  7:白
		0xc6, 0xc6, 0xc6,	//  8:亮灰 
		0x84, 0x00, 0x00,	//  9:暗红
		0x00, 0x84, 0x00,	// 10:暗绿
		0x84, 0x84, 0x00,	// 11:暗黄
		0x00, 0x00, 0x84,	// 12:暗青
		0x84, 0x00, 0x84,	// 13:暗紫
		0x00, 0x84, 0x84,	// 14:浅暗蓝
		0x84, 0x84, 0x84	// 15:暗灰
	};
	set_palette(table_rgb);
	return;
}
void set_palette(unsigned char *rgb){
	int i, eflags;
	eflags = io_load_eflags();	// 记录中断许可标志的值
	io_cli(); 					// 将中断许可标志置为0，禁止中断
	io_out8(0x03c8, 0); 		// 设置显卡
	for (i = 0; i <= 15; i++) {
		io_out8(0x03c9, rgb[0] / 4); // 设置R，G，B值
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	// 复原中断许可标志
	return;
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1){
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l){
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void init_screen8(char *vram, int x, int y){
	//绘制桌面
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	return;
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font){
	// 打印单个字符
	int i;
	char *p, d;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0)  p[0] = c; 
		if ((d & 0x40) != 0)  p[1] = c; 
		if ((d & 0x20) != 0)  p[2] = c; 
		if ((d & 0x10) != 0)  p[3] = c; 
		if ((d & 0x08) != 0)  p[4] = c; 
		if ((d & 0x04) != 0)  p[5] = c; 
		if ((d & 0x02) != 0)  p[6] = c; 
		if ((d & 0x01) != 0)  p[7] = c; 
	}
	return;
}

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s){
	// 打印字符串
	extern char hankaku[4096];
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor8(char *mouse, char bc){
	// 初始化鼠标,bc是BackColor
	static char cursor[16][16] = {
		// 鼠标指针
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act){
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize-4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}
void init_window8(unsigned char *buf, int xsize, int ysize, char *title, char act){	
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize){
	// 显示鼠标图标
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c){
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize){
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize*ysize);
	if(ctl->map == 0){
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; // 刚初始化,还没有图层
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; // 标记未使用
		ctl->sheets0[i].ctl = ctl; // 每个图层记录所属的数组
	}
err:
	return ctl;
}
struct SHEET *sheet_alloc(struct SHTCTL *ctl){
	// 将新生成的未使用的图层初始化
	struct SHEET *sht;
	int i;
	for(i=0;i <MAX_SHEETS;i++){
		if(ctl->sheets0[i].flags == 0){
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; // 标记正在使用
			sht->height = -1; // 不显示
			return sht;
		}
	}
	return 0;
}
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv){
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}
void sheet_updown(struct SHEET *sht,int height){
	// 设置某图层的高度，然后依次更改其他图层高度
	int h, old = sht->height;
	struct SHTCTL *ctl = sht->ctl;
	// 如果指定的高度过高或过低，进行修正
	height = (height > ctl->top+1)?(ctl->top + 1):height;
	height = (height < -1)?-1:height;

	sht->height = height;// 设定高度

	if(old > height){
		// 修改的高度比以前低
		if(height >= 0){
			// 修改指针数组,将[height,old)的元素往上挪一位，更新其高度
			for(h = old; h>height; h--){
				ctl->sheets[h] = ctl->sheets[h-1];
				ctl->sheets[h]->height = h;
			}
			// 把sht的地址贴到height位置上
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		}else{
			// height<0表示将该层设为隐藏
			if(ctl->top > old){
				// 如果old上面还有图层的话，则将上面的图层都向下挪一位
				for(h = old; h<ctl->top; h++){
					ctl->sheets[h] = ctl->sheets[h+1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
		
	}else if(old < height){
		// 修改的高度比以前高
		if(old >= 0){
			// 将[old,height)往下挪一位
			for(h=old; h<height;h++){
				ctl->sheets[h] = ctl->sheets[h+1];
				ctl->sheets[h]->height=h;
			}
			ctl->sheets[height] = sht;
		}else{
			// 说明原来图层是不显示的
			// 需要把height上面的上移一位，才能插入sht
			for(h=ctl->top; h>=height;h--){
				ctl->sheets[h+1] = ctl->sheets[h];
				ctl->sheets[h+1]->height = h+1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1){
	if(sht->height >= 0){
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
	return;
}
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0){
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	if (vx0 < 0) { vx0 = 0; }
	if (vy0 < 0) { vy0 = 0; }
	if (vx1 > ctl->xsize) { vx1 = ctl->xsize; }
	if (vy1 > ctl->ysize) { vy1 = ctl->ysize; }
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		sid = sht - ctl->sheets0; // 相对于sheets0的偏移作为图层号码
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (buf[by * sht->bxsize + bx] != sht->col_inv) {
					map[vy * ctl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1){
	// 指定刷新{vx0,vy0,vx1,vy1}的块
	int h, bx, by, vx, vy, bx0, by0, bx1, by1, index;
	unsigned char *buf, sid,*map = ctl->map, *vram = ctl->vram;
	struct SHEET *sht;

	vx0 = (vx0<0)?0:vx0;
	vy0 = (vy0<0)?0:vy0;
	vx1 = (vx1>ctl->xsize)?(ctl->xsize):vx1;
	vy1 = (vy1>ctl->ysize)?(ctl->ysize):vy1;

	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;

		sid = sht - ctl->sheets0;
		// 下面的变换值得关注
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		// 限定刷新范围
		bx0 = (bx0<0)?0:bx0;
		by0 = (by0<0)?0:by0;
		bx1 = (bx1>sht->bxsize)?(sht->bxsize):bx1;
		by1 = (by1>sht->bysize)?(sht->bysize):by1;

		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				index = vy * ctl->xsize + vx;
				if (map[index]==sid) {
					// 和‘透明色’相同就不刷新
					vram[index] = buf[by*sht->bxsize+bx];
				}
			}
		}
	}
	return;
}
void sheet_slide(struct SHEET *sht, int vx0, int vy0){
	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { // 只刷新移动前的图层块和移动后的图层块
		sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshmap(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}
void sheet_free(struct SHEET *sht){
	struct SHTCTL *ctl = sht->ctl;
	if(sht->height>=0){
		sheet_updown(sht,-1);
	}
	sht->flags=0;
	return;
}
