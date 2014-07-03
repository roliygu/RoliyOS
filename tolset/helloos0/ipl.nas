; hello-os
; TAB=4

		CYLS	EQU		10		;定义CYLS=10,一种定义常量的语句
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

; 本次添加的部分

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面0
		MOV		DH,0			; 磁头0
		MOV		CL,2			; 扇区2
readloop:
		MOV 	SI,0 			; 记录失败次数的寄存器
retry:
		MOV		AH,0x02			; AH=0x02，读入磁盘
		MOV		AL,1			; 1个扇区
		MOV		BX,0
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 调用磁盘BIOS
		JNC		next			; 没出错就跳转到next
		ADD		SI,1			; 往SI加1
		CMP		SI,5			; 比较SI和5
		JAE		error			; SI >= 5 时，跳转到error
		MOV		AH,0x00
		MOV		DL,0x00			; A驱动器
		INT		0x13			; 重置驱动器
		JMP		retry
next:
		MOV		AX,ES			; 把内存地址后移0x200
		ADD		AX,0x0020
		MOV		ES,AX			; 因为没有ADD ES，0x020指令，这里绕个弯
		ADD		CL,1			; 往CL里加1
		CMP		CL,18			; 比较CL与18
		JBE		readloop		; 如果CL<=18,跳转至readloop
		MOV		CL,1
		ADD 	DH,1
		CMP 	DH,2
		JB 		readloop 		; 如果DH<2，则跳转到readloop
		MOV 	DH,0
		ADD 	CH,1
		CMP 	CH,CYLS
		JB 		readloop 		; 如果CH < CYLS,则跳转到readloop

fin:
		HLT						;让CPU停止，等待指令
		JMP		fin				;无限循环

error:
		MOV SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			;显示一个文字
		MOV		BX,16			;指定字符颜色
		INT		0x10			;调用显卡BIOS
		JMP		putloop
msg:
		DB		0x0a, 0x0a		;两个换行
		DB		"hello, Roliy"
		DB		0x0a			
		DB		0

		RESB	0x7dfe-$			;填写0x00，直到0x7dfe

		DB		0x55, 0xaa
