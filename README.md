RoliyOS-My small OS
=======
版本说明：
--------
###V1.1.3
	用变量的方式修改了Makefile
	增加了检测扇区是否错误的代码
###V1.1.4
	将检测扇区错误扩展到了柱面层次，并加入了定义常量的语句
###V1.1.5
	新增了haribote.nas：将fin函数单独写到了这个文件中
	修改了ipl.nas和Mackfile文件
		ipl.nas 将fin函数的地址设置为haribote中的位置了
		Makefile文件修改了制造img的语句，输入变成了haribote.sys和ipl.bin等
###V1.1.5.1
	修改了haribote.nas文件，使得调用后屏幕全黑
###V1.1.6
	修改了haribote.nas文件，在进入32位状态前收集好BIOS信息，[因为进入32位状态后，再回来收集信息会很麻烦]
###V1.1.7
	修改了haribote.nas文件，将其重命名为asmhead.nas，其包含了原来haribote.nas的汇编功能和将C的机器语言和汇编混合的功能
	新增了bootpack.c文件，作为用C语言开发的基础
	修改了Makfile文件，主要增加了将c文件编译成.hrb，然后和sys混编成img的处理
###V1.1.8
	增加了naskfunc.nas文件，其中包括HLT功能的汇编函数
	修改了bootpack.c文件，直接调用naskfunc.nas里的汇编函数
	修改Makefile，连接的时候，把naskfunc.nas文件连接到bootpack的中间文件里
###V2.1.1
	修改了naskfunc.nas文件，增加了往显存写入数据的函数
	修改了bootpack.c文件，往显存写入数据
###V2.1.2
	使用指针来往内存写内容
###V2.1.3
	修改了bootpack.c文件，增加了调色板设置
	修改了naskfunc.nas文件，增加了往端口读写数据的函数，和读写标志寄存去Eflags的函数
###V2.1.4
	修改了bootpack.c文件，增加了画出矩形的函数
###V2.1.5
	增加了hankaku文件：ASCII字符库
	修改了Makefile文件，在编译时将字符库文件一起编译
	修改了bootpack.c文件，成功输出“Roliy”
###V2.1.6
	修改了bootpack.c文件，使用了sprint()函数
	修改了bootpack.c文件，增加了初始化GDT和IDT的函数
	修改了naskfunc.nas文件，增加了初始化GDT和IDT的汇编实现
笔记与说明：
--------
###前言
	这个projet来源于《30天自制操作系统》一书，也是为了锻炼一下底层编程和管理多个源文件的能力
	这里的笔记先按照章节的顺序整理一下，之后再考虑其他更好的整理方式

###Makefile
####命令
>run命令：将制作好的img（即镜像文件）导入虚拟机
>>copy %.img ../z__tools/qemu/fdimage0.bin<br>
>>../z_tools/make.exe -C ../z_tools/qemu

>install命令：将制作好的img镜像文件烧进软盘中
>>../z_tools/imgtol.com w a: %.img

>nask命令：将[.nas,]编译成[.bin,]文件
>>../z_tools/nase.exe %.nas %.bin

>edimg命令：将[.bin,.sys,]文件编译成img文件
>>../z_tools/edimg.exe imgin:../z_tools/fdimg0at.tek wbing src:%.bin len:512
>>from:0 to:0 imgout: %.img

####Makefile格式
* 输出文件 : 输入文件 Makefile <br> ../z_tools/xxx.exe xxxx xxxx
* 使用make命令来生成目标文件时，使用`make -r xxx` 将会递归式的生成xxx
* 使用%来代替文件名，以此写出的命令将按后缀名匹配文件来make
* 命令: 程序路径 -r 目标文件 e.g.  img : ../z_tools/make.exe -r
%.img
* Makefile中可以定义变量，如下的方法：<br> TOOLPATH = ../z_tools/ <br> 以后需
要使用 `../z_tools/` 时，用 `$(TOOLPATH)` 代替
* del 文件名 "删除文件

###开发OS的步骤：
* 在Windows系统上编写源代码
* 用C语言编译器编译源代码，生成机器语言文件
* 对机器语言文件进行加工，生成映像文件
* 将映像文件写入磁盘，做成OS的启动盘

###制作3.5英寸软盘的系统启动盘：
	除了地址000000-000090和0001F0-0014000的内容要修改以外，其他部分内容全为0

###使用汇编语言制作系统启动盘：
####Version_1.0
####代码
	DB	0xeb, 0x4e, 0x90, 0x48, 0x45, 0x4c, 0x4c, 0x4f
	DB	0x49, 0x50, 0x4c, 0x00, 0x02, 0x01, 0x01, 0x00
	DB	0x02, 0xe0, 0x00, 0x40, 0x0b, 0xf0, 0x09, 0x00
	DB	0x12, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
	DB	0x40, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x29, 0xff
	DB	0xff, 0xff, 0xff, 0x48, 0x45, 0x4c, 0x4c, 0x4f
	DB	0x2d, 0x4f, 0x53, 0x20, 0x20, 0x20, 0x46, 0x41
	DB	0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00
	RESB	16
	DB	0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
	DB	0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
	DB	0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
	DB	0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
	DB	0xee, 0xf4, 0xeb, 0xfd, 0x0a, 0x0a, 0x68, 0x65
	DB	0x6c, 0x6c, 0x6f, 0x2c, 0x20, 0x77, 0x6f, 0x72
	DB	0x6c, 0x64, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB	368
	DB	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xaa
	DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB	4600
	DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB	1469432

####说明
	RESB命令将会空出对应数目的字节，并填0

####Version_1.1
####代码
	DB		0xeb, 0x4e, 0x90
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
	;程序主体
	DB		0xb8, 0x00, 0x00, 0x8e, 0xd0, 0xbc, 0x00, 0x7c
	DB		0x8e, 0xd8, 0x8e, 0xc0, 0xbe, 0x74, 0x7c, 0x8a
	DB		0x04, 0x83, 0xc6, 0x01, 0x3c, 0x00, 0x74, 0x09
	DB		0xb4, 0x0e, 0xbb, 0x0f, 0x00, 0xcd, 0x10, 0xeb
	DB		0xee, 0xf4, 0xeb, 0xfd
	;信息显示部分
	DB		0x0a, 0x0a		;两个换行
	DB		"hello, Roliy"
	DB		0x0a			;换行
	DB		0
	RESB	0x1fe-$			;填写0x00，直到0x001fe
	DB		0x55, 0xaa
	;以下是启动区以外部分的输出
	DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB	4600
	DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	RESB	1469432

####说明
	`RESB 0x1fe-$`中的`$`符号代表了这一行在文件中是第几个字节
	软盘的第一个扇区称作启动区，计算机会检查该扇区最后两个字节是否是`55AA`来判断该软盘是否是启动盘

###汇编指令
* ORG 0x7c00 ：指明了将本程序装载到该内存地址
* HLT ：让CPU停止动作，进入待机状态

###寄存器
>16位寄存器
>>AX 累加寄存器,被ADD指令使用时，生成的机器码比其他短<br>
>>CX 计数寄存器<br>
>>DX 数据寄存器<br>
>>BX 基址寄存器<br>
>>SP 栈指针寄存器<br>
>>BP 基址指针寄存器<br>
>>SI 源变址寄存器<br>
>>DI 目的变址寄存器<br>
>>ES 附加段寄存器<br>
>>CS 代码段寄存器<br>
>>SS 栈段寄存器<br>
>>DS 数据段寄存器<br>

>AX,CX,DX,BX可以被拆成高低8位寄存器

>EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI为32位寄存器

###BIOS的命令,必须在16位下
####显示一个字符
	AH=0x0e
	AL=character code
	BH=0
	BL=color code
	INT 0x10
####检验磁盘
    AH=0x02 ;读盘
    AH=0x03 ;写盘
    AH=0x04 ;校验
    AH=0x0c ;寻道
    AL=处理对象的扇区数
    CH=柱面号&0xff
    CL=扇区号（0-5位）|（柱面号&0x300）>>2;
    DH=磁头号
    DL=驱动器号
    ES:BX=缓冲地址
    返回值：
        FLACS.CF=0 :没有错误
        FLACS.CF=1 :有错误，错误码存进AH内
####设置显卡模式
* AH=0x00;
* AL=模式:
	* 0x03 : 16色字符模式, 80*25
	* 0x12 : VGA图形模式, 640*480*4位彩色模式，独特的4面存储模式
	* 0x13 : VGA图形模式, 320*200*8位彩色模式，调色板模式
	* 0x6a : 拓展VGA图形模式, 800*600*4位彩色模式，独特的4面存储模式
* `320*200*8` 的模式下VRAM的地址是0xa0000-0xaffff的64KB

####读取键盘LED灯状态
* MOV	AH,0x02
* INT 	0x16
* MOV 	[LEDS],AL

###初始各个文件介绍
* ipl10.nas    : 启动区，检查软盘柱面是否有错
* asmhead.nas  : 暂时没有说明，完成BIOS阶段的任务和16位到32位模式的转换
* bootpack.c   : OS的主要实现
	* 主函数名必须是 `HariMain` 

* naskfunc.nas : C语言需要(只有汇编能办到)的功能，定义在这里
	* 

* hankaku.txt  : 字库文件

###各个文件转换的过程,函数名是调用的程序，参数的最后一个是输出文件，其他都是输入文件
* ccl(%.c, %gas)
* gas2nask(%.gas, %.nas)
* nask(%.nas, %.obj)
* obi2bim(%.obj, %.bim)
* bim2hrb(%.bim, %.hrb)
* copy(%.hrb, asmhead.bim, %.sys)
* makefont(%.txt, %.bin)
* bin2obj(%.bin, %.obj)
* edimg(%.bin, [%.sys,] %.img)
* 具体的转换过程参考[此图](.\图示\文件编译图.bmp)

###注意
* 磁盘上的内容从内存的0x8000号地址开始装载
* 32位模式不能调用BIOS功能
* 