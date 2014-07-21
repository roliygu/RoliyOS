#include "bootpack.h"

bool init_que(struct Queue *Q){
	int i;
	for(i=0;i<QUELEN;i++)
		Q->data[i]=0;
	Q->start=0;
	Q->end=0;
	return true;
}
bool push_que(struct Queue *Q, Type ele){
	//有点缺陷，最大长度为QUELEN的队列只能存QUELEN-1个元素
	if((Q->end+1)%QUELEN==Q->start)
		return false;
	else{
		Q->data[Q->end] = ele;
		Q->end = (Q->end+1)%QUELEN;
	}
	return true;
}
bool pop_que(struct Queue *Q, Type *ele){
	if(Q->end==Q->start)
		return false;
	else{
		*ele = Q->data[Q->start];
		Q->start = (Q->start+1)%QUELEN;
	}		
	return true;
}