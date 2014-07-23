#include "bootpack.h"

int insertmemory(struct MEMMAN *man, int index, unsigned int addr, unsigned int size){
	// 往man->free[index]插入{addr,size}
	if(man->frees < MEMMAN_FREES-4){
		// 插入一个元素后,不超过上界
		int tem = man->frees;
		for(;tem>index;tem--)
			man->free[tem] = man->free[tem-1];
		man->free[index].addr = addr;
		man->free[index].size = size;
		man->frees++;
		return 1;
	}else{
		man->losts++;
		man->lostsize+=size;
		return 0;
	}
}
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
			if(man->free[i].size<=1){
				man->frees--;
				if(i < man->frees-1)
					man->free[i].size += man->free[i+1].size;
				for(i = i+1;i < man->frees;i++)
					man->free[i] = man->free[i + 1];
			}
		}
		return a;
	}
	return 0;
}
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size){
	// 释放
	int i,j;
	for(i = 0;i < man->frees; i++){
		// 找到的i满足:free[i-1].addr<addr<free[i].addr
		if(man->free[i].addr > addr){
			break;
		}
	}
	// 将原形式转换成区间形式
	unsigned int sf,ef,st,et,sn,en;
	st =addr;
	et = addr + size;
	sn = man->free[i].addr;
	en = sn + man->free[i].size;

	if(i == 0){
		if(et == sn){
			man->free[i].size += size;
			man->free[i].addr -= size;
		}else{
			insertmemory(&man, i, addr, size);
		}
		return 0;
	}else if(i == man->frees){
		// 遍历完都没有找到要求的块,所以只考虑是否能和前面的块合并
		sf = man->free[i-1].addr;
		ef = sf + man->free[i-1].size;
		if(ef == st)
			man->free[i-1].size += size;
		else{
			insertmemory(&man, i, addr, size);		
		}
	}else{
		sf = man->free[i-1].addr;
		ef = sf + man->free[i-1].size;
		if(ef == st){
			man->free[i-1].size += size;
			if(et == sn){
				man->free[i-1].size += man->free[i].size;
				for(;i < man->frees-1;i++)
					man->free[i] = man->free[i+1];
				man->frees--;
			}
			// 不能和后面合并时，不管
		}else{
			if(et == sn){
				man->free[i].addr -= size;
				man->free[i].size += size;
			}else{
				insertmemory(&man, i, addr, size);
			}
		}
	}
	return 1;
}