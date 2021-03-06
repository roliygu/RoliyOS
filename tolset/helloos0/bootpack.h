#ifndef BOOTPACK
#define BOOTPACK

// asmhead.nas
struct BOOTINFO {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};
#define ADR_BOOTINFO	0x00000ff0

// naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

// memory.c
#define MEMMAN_FREES 			4090
#define MEMMAN_ADDR 			0x003c0000
struct FREEINFO{
	// 使用块起始地址和块大小来表示某段内存
	// 1单位4KB
	unsigned int addr, size;
};
struct MEMMAN{
	// frees:可用块数，maxfrees:frees最大值
	// lostsize:释放失败的内存大小总和 losts:失败次数
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

int insertmemory(struct MEMMAN *man, int index, unsigned int addr, unsigned int size);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

// graphic.c
#define MAX_SHEETS 		256
#define SHEET_USE 		1
struct SHEET{
	// bxsize*bysize:图层的大小;(vx0, vy0):图层在画面上位置的坐标
	// col_inv:透明色号;height:图层高度;Flags:各种设定信息
	// *ctl用来指向所属的数组
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};
struct SHTCTL{
	// (vram, xsize, ysize):(VRAM地址,画面大小)
	// map:各个像素点所属的图层
	// top:最上层图层高度
	// sheets各个图层的地址
	// sheets0各个图层
	unsigned char *vram,*map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
void init_palette(void);
void set_palette(unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void init_screen8(char *vram, int x, int y);
void init_mouse_cursor8(char *mouse, char bc);
void init_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize);
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht,int bx0,int by0,int bx1,int by1);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

// datastructure.c
typedef int Type;
struct Queue {
	Type *buf;
	int start, end, size, free, flags;
	struct TASK *task;
};
void que_init(struct Queue *Q, int size, Type *buf, struct TASK *task);
int que_push(struct Queue *Q, Type data);
int que_pop(struct Queue *Q);
int que_status(struct Queue *Q);

// equipment.c
struct MOUSE_DEC{
	// 每次三个字节的数据表示鼠标状态
	// 第一个字节的前四位表示移动,点击[左键,右键,中键]
	// 第二第三字节表示横轴和纵轴偏移量
	// phase是接收过程中用于区分是第几个的标记位
	unsigned char buf[3],phase;
	int x, y, btn;
};
void inthandler21(int *esp);
void inthandler2c(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct Queue *fifo, int data0);
void init_mouse(struct Queue *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);
#define PORT_KEYSTA				0x0064    //设备编码
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02      // 这个只是构造出来为了检测第二位是否为0
#define KEYCMD_WRITE_MODE		0x60 	  // 模式设定
#define KBC_MODE				0x47 	  // 利用鼠标模式
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4
#define KEYCMD_LED 				0xed
extern char keytable0[0x80];
extern char keytable1[0x80];
// dsctbl.c 
struct SEGMENT_DESCRIPTOR {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_TSS32 		0x0089
#define AR_INTGATE32	0x008e

//inc.c
void init_pic(void);
void inthandler27(int *esp);
unsigned int memtest(unsigned int start, unsigned int end);
#define EFLAGS_AC_BIT      0x00040000
#define CR0_CACHE_DISABLE  0x60000000
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1
#define PORT_KEYDAT     0x0060  // (固定的)键盘/鼠标设备的内存/端口

// timer.c
#define PIT_CTRL 		0x0043
#define PIT_CNT0 		0x0040
#define MAX_TIMER 		500
#define TIMER_FLAGS_ALLOC 		1
#define TIMER_FLAGS_USING 		2
struct TIMER{
	// next:  下一个timer的指针；timeout:时间进行到这个值的时候报告给CPU
	// flags: 这个timer是否有效
	// fifo:  指向缓冲区
	// data:  用来标记不同timer
	struct TIMER *next;
	unsigned int timeout, flags;
	struct Queue *fifo;
	char data;
};
struct TIMERCTL{
	// count:  从开始到现在8254发过来的计数次数
	// next:   指向大于当前时刻的第一个timer的timeout
	// t0:	   指向大于当前时刻的第一个timer
	// timers0:timer数组，相当于提前分配好内存了
	unsigned int count,next;
	struct TIMER *t0;
	struct TIMER timers0[MAX_TIMER];
};
void init_pit(void);
void asm_inthandler20(void);
void inthandler20(int *esp);
struct TIMER *timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct Queue *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);

// task.c
#define MAX_TASKS 		1000
#define TASK_GDT0 		3
#define MAX_TASKS_LV	100
#define MAX_TASKLEVELS	10
struct TSS32{
	int backlink, esp0, ss0, eap1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap;
};
struct TASK{
	// sel存放段号,
	// flags=2表示本进程在CPU中执行,flags=0表示本进程的内存未使用,
	//flags=1表示本进程已经被初始化,但不是CPU运行状态
	// priority表示权限
	int sel, flags;
	int level, priority;
	struct Queue fifo;
	struct TSS32 tss;
};
struct TASKLEVEL{
	// running:进程数组中进程个数
	// now:正在运行的任务下标
	int running;
	int now;
	struct TASK *tasks[MAX_TASKS_LV];
};
struct TASKCTL{
	// now_lv 现在活动中的Level
	// 下次任务切换时,是否需要修改Level
	int now_lv;
	char lv_change;
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};
extern struct TIMER *task_timer;
void load_tr(int t);
void farjmp(int eip, int cs);
struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_run(struct TASK *task, int level, int priority);
void task_switch(void);
void task_switchsub(void);
void task_sleep(struct TASK *task);
void task_idle(void);

#endif
