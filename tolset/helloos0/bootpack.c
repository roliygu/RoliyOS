void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void){
	int i;
	for(i = 0xa0000; i<=0xaffff; i+=2){
		write_mem8(i, 15);
		if(i%4==0)
			write_mem8(i, 7);
	}
	for(;;){
		io_hlt();
	}
}