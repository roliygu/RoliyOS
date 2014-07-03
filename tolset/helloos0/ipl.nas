; hello-os
; TAB=4

		ORG		0x7c00			;指明程序装载的地址

; 以下这段是标准FAT12格式软盘专用代码

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		; 启动区的名称可以是任意8字节字符串
		DW		512				;每个扇区的大小必须为512字节
		DB		1				;簇的大小必须是一个扇区
		DW		1				;FAT的起始位置从第一个扇区开始
		DB		2				;FAT的个数必须为2
		DW		224				;根目录大小设为224项
		DW		2880			;磁盘大小必须为2880扇区
		DB		0xf0			;磁盘种类
		DW		9				;FAT的长度为9个扇区
		DW		18				;一个磁道必须有18个扇区
		DW		2				;磁头数必须为2
		DD		0				;不使用分区，必须是0
		DD		2880			;重写一次磁盘大小
		DB		0,0,0x29		;意义不明，固定
		DD		0xffffffff		;可能是卷标号码
		DB		"HELLO-OS   "	;磁盘名称，11字节
		DB		"FAT12   "		;磁盘格式名称，8字节
		RESB	18				;空出18字节

;程序核心
entry:
		MOV		AX,0			
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg		
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			;显示一个文字
		MOV		BX,16			;指定字符颜色
		INT		0x10			;调用显卡BIOS
		JMP		putloop
fin:
		HLT						;让CPU停止，等待指令
		JMP		fin				;无限循环


msg:
		DB		0x0a, 0x0a		;两个换行
		DB		"hello, Roliy"
		DB		0x0a			;换行
		DB		0

		RESB	0x7dfe-$			;填写0x00，直到0x7dfe

		DB		0x55, 0xaa
