void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void){
	int i;
	for(i = 0xa0000; i<=0xaffff; i+=2){
		char *p= (char *)i;
		*p = i%4;
	}
	for(;;){
		io_hlt();
	}
}