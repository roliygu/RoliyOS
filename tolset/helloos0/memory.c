#include "bootpack.h"

void memman_init(struct MEMMAN *man){
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return ;
}
unsigned int memman_total(struct MEMMAN *man){
	// 剩余可用内存大小
	unsigned int i,t = 0;
	for(i = 0;i < man->frees;i++){
		t += man->free[i].size;
	}
	return t;
}
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size){
	// 分配
	unsigned int i, a;
	for(i=0;i<man->frees;i++){
		if(man->free[i].size >= size){
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			man->frees--;
			if(i < man->frees-1)
				man->free[i].size += man->free[i+1].size;
			for(i = i+1;i < man->frees;i++)
				man->free[i] = man->free[i + 1];	
		}
		return a;
	}
	return 0;
}
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size){
	// 释放
	int i,j;
	for(i = 0;i < man->frees; i++){
		if(man->free[i].addr > addr){
			break;
		}
	}
	//未完
	return 1;
}