RoliyOS-My small OS
=======
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
>>C语言只能使用到EAX，ECX和EDX三个32位寄存器

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

####关于调色板(虽然不是BIOS的内容，但是跟显卡模式是配套的)
	当前模式下，使用6位16进制数来表示一个颜色
	这6位拆成3个两位数，分别表示RGB值
	设置调色板，就是给一个无符号char数组，数组中从0开始，每连续三个元素表示一种颜色
	显色的过程就是往显存对应位置载入table[i],table[i+1],table[i+2]且i%3==0//其实就是往显存装6个字节的内容
* 使用的时候
	+ 记录中断许可标志值
	+ 中断许可标志值设为0,禁止中断<br>
	+ 将0存进0x03c8[显卡某个端口] //相当于设置<br>
	+ 将table中连续三个三个字节存进0x03c9[显卡另一个端口]//设定颜色<br>
	+ 恢复中断许可标志<br>
	+ 往显存填颜色按照 `0xa0000+x+y*320`来计算地址<br>

####读取键盘LED灯状态
* MOV	AH,0x02
* INT 	0x16
* MOV 	[LEDS],AL

###初始各个文件介绍
* ipl10.nas    : 启动区，检查软盘柱面是否有错
* asmhead.nas  : 暂时没有说明，完成BIOS阶段的任务和16位到32位模式的转换
* bootpack.c   : OS的主文件
	* 主函数名必须是 `HariMain` 
* graphic.c    : 绘图相关C语言操作
* dsctbl.c 	   : 设置GDT和IDT
* naskfunc.nas : C语言需要(只有汇编能办到)的功能，定义在这里
	* 函数名前加`_`
	* 函数要在`GLOBAL`中声明
	* 例子
		* GLOBAL  _io_hlt
		* _io_hlt:
		* HLT
		* RET
	* C文件中对应的函数参数在汇编中使用寄存器和内存地址来表示
	* 第一个参数在ESP+4的位置,后面的依次加4（当然，参数是int的情况下）
	* 执行RET语句时，EAX中的值被当作函数的返回值
	* 标志寄存器的值不能直接传到其他寄存器，需要借助栈

* hankaku.txt  : 字库文件

####GDT 全局段号记录表
* 内存分段模式下，每个段使用一个数据结构Segment来唯一表示，该数据结构包括：
	* 段大小
	* 段起始地址
	* 段管理属性[禁止写入，禁止执行，系统专用]
	这部分内容，大小一共是8个字节(64bits)
* 段寄存器只有16位,无法装下Segment,所以使用GDT(作为检索表)来使用某个段
	* 段寄存器低3位无法使用，故只能表示8192个段
	* 段寄存器存的是`段号`
	* 系统根据`段号`到GDT中查找实际的`Segment`
* 大致可以认为，GDT是一个表，表的每项包括[段号,对应的Segment]
* GDT的起始地址保存在GDTR寄存器中
* Segment{limit_low, base_low;base_mid, access_right;limit_hign, base_high;}
	* 段地址为32位，分成base_low,base_mid和base_high三段
	* 段(大小)上限为4G，分为limit_low和limit_high两部分，注意单位是页(4KB)，limit_high只用高4位
	* 权限一共12位，高位4位放在limit_high的高4位，低位8位放在access_right中
		* 高四位的第一位取0或1对应将段上限解释为以字节和页为单位
		* 低8位
			* 0x00 	未使用
			* 0x92  系统专用，可读写，不可执行
			* 0x9a 	系统专用，可执行，可读不可写
			* 0xf2 	应用程序用，可读写，不可执行
			* 0xfa 	应用程序用，可执行，可读不可写

####IDT 中断记录表
####PIC 可编程中断控制器


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
* 具体的转换过程参考[此图](/图示/文件编译图.bmp)

###注意
* 磁盘上的内容从内存的0x8000号地址开始装载
* 32位模式不能调用BIOS功能
* RESB命令将会空出对应数目的字节，并填0
* `RESB 0x1fe-$`中的`$`符号代表了这一行在文件中是第几个字节
* 软盘的第一个扇区称作启动区，计算机会检查该扇区最后两个字节是否是`55AA`来判断该软盘是否是启动盘

###键盘控制电路，键盘和鼠标


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
###V3.1.1
	将bootpack.c文件按功能分成了bootpack.c,dsctbl.c和graphic.c三个文件
	修改了Makefile文件,支持分割以后的文件的编译
###V3.1.2
	新增int.c文件，初始化PIC
	修改bootpack.c,bootpack.h和Makefile文件来编译
###V3.1.3
	修改了naskfunc.nas,dsctbl.c,bootpack.h,bottpack.c,int.c文件，增加了处理键盘中断的部分
###V4.1
	详细处理了键盘和鼠标中断:完成从按键到屏幕显示的实现
	增加了两个文件：
		equipment.c是鼠标和键盘准备的函数，之后可能会混合仅int.c中
		datastructure.c基础数据结构，目前实现了`队列`，用作鼠标键盘缓冲区