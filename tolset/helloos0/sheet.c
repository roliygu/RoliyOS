/* ƒ}ƒEƒX‚âƒEƒBƒ“ƒhƒE‚Ìd‚Ë‡‚í‚¹ˆ— */

#include "bootpack.h"

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* ƒV[ƒg‚Íˆê–‡‚à‚È‚¢ */
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; /* –¢Žg—pƒ}[ƒN */
	}
err:
	return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE; /* Žg—p’†ƒ}[ƒN */
			sht->height = -1; /* ”ñ•\Ž¦’† */
			return sht;
		}
	}
	return 0;	/* ‘S‚Ä‚ÌƒV[ƒg‚ªŽg—p’†‚¾‚Á‚½ */
}

void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; // 存储设置前的高度信息 

	// 如果指定的高度过高或过低，进行修正
	height = (height > ctl->top+1)?(ctl->top + 1):height;
	height = (height < -1)?-1:height;

	sht->height = height;// 设定高度

	// 对sheets[]重新排列
	if (old > height) {	/* ˆÈ‘O‚æ‚è‚à’á‚­‚È‚é */
		if (height >= 0) {
			/* ŠÔ‚Ì‚à‚Ì‚ðˆø‚«ã‚°‚é */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ”ñ•\Ž¦‰» */
			if (ctl->top > old) {
				/* ã‚É‚È‚Á‚Ä‚¢‚é‚à‚Ì‚ð‚¨‚ë‚· */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* •\Ž¦’†‚Ì‰º‚¶‚«‚ªˆê‚ÂŒ¸‚é‚Ì‚ÅAˆê”Ôã‚Ì‚‚³‚ªŒ¸‚é */
		}
		sheet_refresh(ctl); /* V‚µ‚¢‰º‚¶‚«‚Ìî•ñ‚É‰ˆ‚Á‚Ä‰æ–Ê‚ð•`‚«’¼‚· */
	} else if (old < height) {	/* ˆÈ‘O‚æ‚è‚à‚‚­‚È‚é */
		if (old >= 0) {
			/* ŠÔ‚Ì‚à‚Ì‚ð‰Ÿ‚µ‰º‚°‚é */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		} else {	/* ”ñ•\Ž¦ó‘Ô‚©‚ç•\Ž¦ó‘Ô‚Ö */
			/* ã‚É‚È‚é‚à‚Ì‚ðŽ‚¿ã‚°‚é */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* •\Ž¦’†‚Ì‰º‚¶‚«‚ªˆê‚Â‘‚¦‚é‚Ì‚ÅAˆê”Ôã‚Ì‚‚³‚ª‘‚¦‚é */
		}
		sheet_refresh(ctl); /* V‚µ‚¢‰º‚¶‚«‚Ìî•ñ‚É‰ˆ‚Á‚Ä‰æ–Ê‚ð•`‚«’¼‚· */
	}
	return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
	return;
}

void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { /* ‚à‚µ‚à•\Ž¦’†‚È‚ç */
		sheet_refresh(ctl); /* V‚µ‚¢‰º‚¶‚«‚Ìî•ñ‚É‰ˆ‚Á‚Ä‰æ–Ê‚ð•`‚«’¼‚· */
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); /* •\Ž¦’†‚È‚ç‚Ü‚¸”ñ•\Ž¦‚É‚·‚é */
	}
	sht->flags = 0; /* –¢Žg—pƒ}[ƒN */
	return;
}
