#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman){
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; // 活动中标志
	taskctl->running = 1;
	taskctl->now = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}
struct TASK *task_alloc(void){
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; // 正在使用的标志
			task->tss.eflags = 0x00000202; // IF = 1; 
			task->tss.eax = 0; // 这里先置为0
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0; // 全部正在使用
}

void task_run(struct TASK *task){
	task->flags = 2; // 活动中标志
	taskctl->tasks[taskctl->running] = task;
	taskctl->running++;
	return;
}

void task_switch(void){
	// 切换进程
	timer_settime(task_timer, 2);
	if (taskctl->running >= 2) {
		// 普通的顺序表
		taskctl->now = (taskctl->now+1)%(taskctl->running);
		farjmp(0, taskctl->tasks[taskctl->now]->sel);
	}
	return;
}
void task_sleep(struct TASK *task){
	// 指定task转入休眠状态
	// 休眠状态的意思是从taskctl维护的进程数组中移去,需要唤醒的时候再加进来
	int i;
	char ts = 0;
	if (task->flags == 2) {		// 如果指定任务处于唤醒状态
		if (task == taskctl->tasks[taskctl->now]) {
			ts = 1; // 稍后再进行任务切换,延迟休眠自己是确保立马休眠的话,不造成不稳定状态..
		}
		// 从顺序表中移掉一个元素的固定操作
		// 寻找task所在的位置
		for (i = 0; i < taskctl->running; i++) 
			if (taskctl->tasks[i] == task) 
				break;
		taskctl->running--;
		if (i < taskctl->now) 
			// 如果休眠(去掉)的进程是running态后面的,则不需要对正在运行的进程号做修正
			// 否则,去掉以后,running态的进程向前挪一位,相对要修改taskctl的now标记
			// 如果相等的话,也是不用做修正的,因为将其去掉以后,后面一个元素顶了i的位置,任务切换刚好切换到下一个进程
			taskctl->now--; 
		// 移动成员
		for (; i < taskctl->running; i++) 
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		task->flags = 1; // 不工作状态
		if (ts != 0) {
			// 只有在本进程被休眠了才需要切换到其他进程,否则只是从数组中移除
			if (taskctl->now >= taskctl->running) 
				// 如果now值出现异常,则进行修正
				taskctl->now = 0;
			farjmp(0, taskctl->tasks[taskctl->now]->sel);
		}
	}
	return;
}
