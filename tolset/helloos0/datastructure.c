#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void que_init(struct Queue *Q, int size, Type *buf){
	Q->size = size;
	Q->buf = buf;
	Q->free = size; 
	Q->flags = 0;
	Q->end = 0; 
	Q->start = 0; 
	return;
}

int que_push(struct Queue *Q, Type data){
	if (Q->free == 0) {
		Q->flags |= FLAGS_OVERRUN;
		return -1;
	}
	Q->buf[Q->end] = data;
	Q->end = (Q->end+1) % Q->size;
	Q->free--;
	return 0;
}

int que_pop(struct Queue *Q){
	int data;
	if (Q->free == Q->size) {
		return -1;
	}
	data = Q->buf[Q->start];
	Q->start = (Q->start+1) % Q->size;
	Q->free++;
	return data;
}

int que_status(struct Queue *Q){
	return Q->size - Q->free;
}
