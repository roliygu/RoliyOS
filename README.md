RoliyOS-My small OS
=======
笔记与说明:
--------
###前言
######因为还是未完成状态,所以先按照大概整理一下,全部完成以后再好好整理一下
######貌似Github不能直接上传exe的文件,所以将本project必需的工具打包成了`tolset.zip`
######使用的时候先解压缩`tolset.zip`,然后将`helloos0`整个文件夹复制到`tolset`文件目录下,打开`CMD.bat`,输入`make run`即可看到效果

###Makefile
####命令
>run命令:将制作好的img（即镜像文件）导入虚拟机
>>copy %.img ../z__tools/qemu/fdimage0.bin<br>
>>../z_tools/make.exe -C ../z_tools/qemu

>install命令:将制作好的img镜像文件烧进软盘中
>>../z_tools/imgtol.com w a: %.img

>nask命令:将[.nas,]编译成[.bin,]文件
>>../z_tools/nase.exe %.nas %.bin

>edimg命令:将[.bin,.sys,]文件编译成img文件
>>../z_tools/edimg.exe imgin:../z_tools/fdimg0at.tek wbing src:%.bin len:512
>>from:0 to:0 imgout: %.img

####Makefile格式
* 输出文件 : 输入文件 Makefile <br> ../z_tools/xxx.exe xxxx xxxx
* 使用make命令来生成目标文件时,使用`make -r xxx` 将会递归式的生成xxx
* 使用%来代替文件名,以此写出的命令将按后缀名匹配文件来make
* 命令: 程序路径 -r 目标文件 e.g.  img : ../z_tools/make.exe -r
%.img
* Makefile中可以定义变量,如下的方法:<br> TOOLPATH = ../z_tools/ <br> 以后需
要使用 `../z_tools/` 时,用 `$(TOOLPATH)` 代替
* del 文件名 "删除文件

###开发OS的步骤:
* 在Windows系统上编写源代码
* 用C语言编译器编译源代码,生成机器语言文件
* 对机器语言文件进行加工,生成映像文件
* 将映像文件写入磁盘,做成OS的启动盘

###制作3.5英寸软盘的系统启动盘:
	除了地址000000-000090和0001F0-0014000的内容要修改以外,其他部分内容全为0

###汇编指令
* ORG 0x7c00 :指明了将本程序装载到该内存地址
* HLT :让CPU停止动作,进入待机状态

###寄存器
>16位寄存器
>>AX 累加寄存器,被ADD指令使用时,生成的机器码比其他短<br>
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
>>C语言只能使用到EAX,ECX和EDX三个32位寄存器

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
    返回值:
        FLACS.CF=0 :没有错误
        FLACS.CF=1 :有错误,错误码存进AH内
####设置显卡模式
* AH=0x00;
* AL=模式:
	* 0x03 : 16色字符模式, 80*25
	* 0x12 : VGA图形模式, 640*480*4位彩色模式,独特的4面存储模式
	* 0x13 : VGA图形模式, 320*200*8位彩色模式,调色板模式
	* 0x6a : 拓展VGA图形模式, 800*600*4位彩色模式,独特的4面存储模式
* `320*200*8` 的模式下VRAM的地址是0xa0000-0xaffff的64KB
* 更高分辨率的设置
	* BX设定画面模式号码
		* 0x4101 640 * 480 * 8彩色
		* 0x4103 800 * 600 * 8彩色
		* 0x4105 1024 * 768 * 8彩色
	* AX=0x4f02
	* 显存起始地址设定为0xe0000000
* 给ES赋值0x9000,DI赋值0之后,以下位置的数据比较重要
	* WORD [ES:DI+0x00] : 模式属性,bit7得是1
	* WORD [ES:DI+0x12] : X的分辨率
	* WORD [ES:DI+0x14] : Y的分辨率
	* WORD [ES:DI+0x19] : 颜色数，必须为8
	* WORD [ES:DI+0x1b] : 颜色指定方法，必须为4,4是调色板模式
	* WORD [ES:DI+0x28] : VRAM地址

####关于调色板(虽然不是BIOS的内容,但是跟显卡模式是配套的)
	当前模式下,使用6位16进制数来表示一个颜色
	这6位拆成3个两位数,分别表示RGB值
	设置调色板,就是给一个无符号char数组,数组中从0开始,每连续三个元素表示一种颜色
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
* ipl10.nas    : 启动区,检查软盘柱面是否有错
* asmhead.nas  : 暂时没有说明,完成BIOS阶段的任务和16位到32位模式的转换
* bootpack.c   : OS的主文件
	* 主函数名必须是 `HariMain` 
* graphic.c    : 绘图相关C语言操作
* dsctbl.c 	   : 设置GDT和IDT
* naskfunc.nas : C语言需要(只有汇编能办到)的功能,定义在这里
	* 函数名前加`_`
	* 函数要在`GLOBAL`中声明
	* 例子
		* GLOBAL  _io_hlt
		* _io_hlt:
		* HLT
		* RET
	* C文件中对应的函数参数在汇编中使用寄存器和内存地址来表示
	* 第一个参数在ESP+4的位置,后面的依次加4（当然,参数是int的情况下）
	* 执行RET语句时,EAX中的值被当作函数的返回值
	* 标志寄存器的值不能直接传到其他寄存器,需要借助栈

* hankaku.txt  : 字库文件

####GDT 全局段号记录表
* 内存分段模式下,每个段使用一个数据结构Segment来唯一表示,该数据结构包括:
	* 段大小
	* 段起始地址
	* 段管理属性[禁止写入,禁止执行,系统专用]
	这部分内容,大小一共是8个字节(64bits)
* 段寄存器只有16位,无法装下Segment,所以使用GDT(作为检索表)来使用某个段
	* 段寄存器低3位无法使用,故只能表示8192个段
	* 段寄存器存的是`段号`
	* 系统根据`段号`到GDT中查找实际的`Segment`
* 大致可以认为,GDT是一个表,表的每项包括[段号,对应的Segment]
* GDT的起始地址保存在GDTR寄存器中
* Segment{limit_low, base_low;base_mid, access_right;limit_hign, base_high;}
	* 段地址为32位,分成base_low,base_mid和base_high三段
	* 段(大小)上限为4G,分为limit_low和limit_high两部分,注意单位是页(4KB),limit_high只用高4位
	* 权限一共12位,高位4位放在limit_high的高4位,低位8位放在access_right中
		* 高四位的第一位取0或1对应将段上限解释为以字节和页为单位
		* 低8位
			* 0x00 	未使用
			* 0x92  系统专用,可读写,不可执行
			* 0x9a 	系统专用,可执行,可读不可写
			* 0xf2 	应用程序用,可读写,不可执行
			* 0xfa 	应用程序用,可执行,可读不可写

####IDT 中断记录表
####PIC 可编程中断控制器


###各个文件转换的过程,函数名是调用的程序,参数的最后一个是输出文件,其他都是输入文件
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
* RESB命令将会空出对应数目的字节,并填0
* `RESB 0x1fe-$`中的`$`符号代表了这一行在文件中是第几个字节
* 软盘的第一个扇区称作启动区,计算机会检查该扇区最后两个字节是否是`55AA`来判断该软盘是否是启动盘

###键盘控制电路,键盘和鼠标
* 写入一个中断的流程
	1. 用C语言写中断之后要执行的动作[当然,是C语言支持的功能,修改makefile编译此C文件];
		* void inthandler20(int *esp){return ;}
	2. 使用汇编写保存中断现场和恢复的部分[注意声明],中间调用C语言所写的中断程序;
		* _asm_inthandler20:
		* PUSH	ES
		* PUSH	DS
		* PUSHAD
		* MOV		EAX,ESP
		* PUSH	EAX
		* MOV		AX,SS
		* MOV		DS,AX
		* MOV		ES,AX
		* CALL	_inthandler20
		* POP		EAX
		* POPAD
		* POP		DS
		* POP		ES
		* IRETD
	3. 注册中断向量
		* set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 * 8, AR_INTGATE32);
	4. 主函数中开放对应中断
		* io_out8(PIC0_IMR, 0xf8);
* 鼠标
	* 中断号为`2c`
	* 鼠标触发的中断会传输三个字节,这里采用`MOUSE_DEC`[具体看代码]来表示每一个鼠标动作
	* 鼠标的流程
		1. 声明`鼠标`,[包括一个256字节的鼠标图像]
		2. 激活鼠标[前期各个中断的缓冲区是分开的,到后面就将所有中断缓冲区合并成一个了,用于保存每次中断传过来的数据]:
			* 绑定缓冲区
			* 初始各个数据:data[起点],phase等
			* 给`键盘控制器`发送**鼠标已准备好**的消息
		3. [因为中断共用缓冲区的原因]当缓冲区有数据时,从中取出的第一步是判断是哪个中断传过来的数据,如果是鼠标的信息,进入下一步
		4. 鼠标缓冲区信息的解码
			* 鼠标中断会传来四种数据
				* ACK,即`键盘控制电路`返回的**鼠标可以工作**
				* 第一字节
				* 第二字节
				* 第三字节:理论上值看到第三字节时,才回过头把前两个字节从缓冲区取出来一起做解析
			* 解析成功后就将坐标的更改应用到`鼠标`上
* 键盘
	* 中断号为`21`
	* 某个按键的`按下`和`松开`都会触发一次中断,每次中断传来一个字节
	* 键盘的流程
		1. 因为每次传来一个字节,而且这一个字节很容易解析,所以也没有用什么数据结构
		2. 初始化键盘
			* 绑定缓冲区
			* 初始化数据:data[起点]
			* 给`键盘控制电路`发送**键盘准备好**的消息
		3. 从缓冲区取出信号判断是否是键盘信号
		4. 直接解析出是哪个按键的按下或松开,做下一步动作  

因为共用缓冲区的缘故,需要一种方式来区分某个字节是哪个中断的信号,一种比较巧妙的办法是:
不同中断的信号所属的区间虽然是相交的,但是给它们设定不同的`段号`就可以让其不相交,由此就可以判断该数据是属于哪个中断的。
不同中断调用的中断函数也是不同的,于是每次中断,中断函数往缓冲区存数据的时候都用{中断数据+段号}代替{中断数据},每次从缓冲区取数据的时候看数据属于哪个段,就可以知道是哪个中断,然后执行对应的后续动作。 
例如,中断1的信号区间是[1,10],中断2的信号区间是[5,15],中断函数1收到中断1的数据时直接存进缓冲区[默认段号为0],中断函数2收到中断2的数据时,将[data+10]存进缓冲区[信号区间变成[15,25]了],从缓冲区取出信号看属于哪个区间就可以看出是哪个中断的数据.

###内存管理
* 采用FreeInfo:{块首地址,块大小}来表示一个空闲内存块
* 采用Memman:{FreeInfo数组,空闲块数,..,}表示全部空闲内存块
* 每次需要分配一块内存时,到Memman中找合适的块,将其切割开,返回需要的那块的首地址
* 每次回收时,到Memman中找到合适位置[Memman中每块一定不会相交,且首地址严格递增]
	* 可能会和前块or后块合并
* 提供4k分配和回收,使得每次申请按照固定块大小管理,避免小碎片

###显示相关
* 引入`图层`的概念,每次刷新按图层刷新
	* Sheet:{buf,bxsize,bysize,vx0,vy0,col_inv,hight,flags}
	* 每个图层包括,一个bxsize*bysize大小的图像数据区buf,该图层在画面上的坐标{vx0,vy0},图层高度、透明度、标记;
	* 标记为0表示未使用,1表示已使用
	* 通过设置`图层`的高度[设置成负数表示隐藏]改变贴图的顺序,每次刷新从低图层刷到top图层
* 采用一个长度为256的`图层数组`来管理所有图层
* 引入`map`的数据区,用于存储画面上每个像素点的图层,刷新的时候根据`map`来刷新,可以有效节省低图层被高图层覆盖却又必须刷新的时间
* x轴是水平轴,y是竖直向下的轴
* 显示的流程
	1. 声明`图层数组`和所有需要的`图层指针`
	2. 初始化`图层数组`,分配各个`图层`的内存,将`指针`指过去
	3. 分配每个`图层`的`buf`,`setbuf()`到各个`图层`结构中
	4. 以上都设置好之后,通过`slide()`指定`图层`和`显示坐标`,在画面上显示出来
	5. `updown()`设置好各个`图层`在`图层数组`中的位置
	6. 每次使用`slide()`或`refresh()`刷新图层的显示
* 图层刷新的细节[refresh()]
	* 示意图
		* 黄色的是下面的不动图层,红色的是上面正在的移动图层
		* 每次移动之后需要刷新的只有红色的部分
		![图层刷新示意图](/图示/图层刷新示意.bmp)
	* 左图:以(Vx,Vy)[也就是红色方块的起始坐标]为原点的话,我们只用刷新原点右下方的部分,其他部分是不被覆盖的,故左端点从原点开始
	* 右图:还是以(Vx,Vy)为原点,还是只用刷新原点右下方的内容,但是上限不超过红色边界,因为外面的部分没有覆盖,不需要再刷新
	* 刷新时,通过指定开始刷新的高度,可以避免每次刷新都会'很快'地刷一遍背景,造成闪烁的感觉
		* 例如有时图层移动时,下面图层露出,就要考虑从哪个height还是刷比较合适

###计时器(使用8254)
* 有两个数据结构,TIMER和TIMERCTL,理解的话就像图层和图层控制一样,具体的看代码
* 设置中断周期
	* AL=0x34;OUT(0x43,AL)
	* AL=中断周期低8位;OUT(0x40,AL);
	* AL=中断周期高8位;OUT(0x40,AL);
	* 中断周期设为0,默认为65536
* 产生中断的频率为`主频/设定的数值`;[设定数值为11932,近似10ms一次中断]
* 计时器[timer]可以看作是`中断`和`图层`的综合应用
* 计时器流程
	1. 写中断函数,并注册到中断向量表
	2. 设置中断周期后,每个周期到来时,8254给CPU[被中断函数拦截住了]发来一个中断信号[具体数据不用管,来了就算一个时间点]
	3. 中断函数根据总共的计数和下一个timer的timeout比较,若相等则[通过缓冲区]通知CUP有一个timer时间到了,否则继续等8254的中断
	4. 初始化timer控制器
	5. 声明timer
	6. alloc():从timer控制器的数组中分配地址给timer,
	7. init():初始化timer,绑定缓冲区之类的
	8. settime():设定timeout,将该timer插入到合适的timers链表中
	9. 然后就等着中断函数通知timeout到了,然后执行后续动作
	
###多任务
* Jmp跳转命令中有有near和far跳转之分,far特指段地址也改变的跳转：使用`JMP FAR`就能从指定内存地址读到段地址和偏移地址
* CPU每次执行带有段地址的指令时，都会去确认一下GDT中的设置，以确定要执行的JMP是普通的far-JMP，还是任务切换
* 多任务自动管理的思路[细节参看代码]
	* 使用到TASKCTL的多任务管理结构体:维护了正在运行(理解成持续运行的进程)的所有任务
	* 通过计时器触发中断(时间片轮转的方式)从CPU中的进程切换到运行队列中的下一个进程
* 目前一个Task只有三种状态,通过flags来区别
	* 0->进程的内存块未被使用
	* 1->内存块已经被初始化,等待时间片到来被CPU执行
	* 2->被CPU执行


###内存分布
	0x00000000 - 0x000fffff :BIOS显存之类(1M)
	0x00100000 - 0x00267fff :保存软盘内容（1440K）
	0x00268000 - 0x0026f7ff	:空(30K)
	0x0026f800 - 0x0026ffff :IDT(2K)
	0x00270000 - 0x0027ffff :GDT(64K)
	0x00280000 - 0x002fffff :bootpack.hrb(512K)
	0x00300000 - 0x003fffff :栈和其他(1M)
	0x00400000 - 			:空

版本说明:
--------
###V1.1.3
	用变量的方式修改了Makefile
	增加了检测扇区是否错误的代码
###V1.1.4
	将检测扇区错误扩展到了柱面层次,并加入了定义常量的语句
###V1.1.5
	新增了haribote.nas:将fin函数单独写到了这个文件中
	修改了ipl.nas和Mackfile文件
		ipl.nas 将fin函数的地址设置为haribote中的位置了
		Makefile文件修改了制造img的语句,输入变成了haribote.sys和ipl.bin等
###V1.1.5.1
	修改了haribote.nas文件,使得调用后屏幕全黑
###V1.1.6
	修改了haribote.nas文件,在进入32位状态前收集好BIOS信息,[因为进入32位状态后,再回来收集信息会很麻烦]
###V1.1.7
	修改了haribote.nas文件,将其重命名为asmhead.nas,其包含了原来haribote.nas的汇编功能和将C的机器语言和汇编混合的功能
	新增了bootpack.c文件,作为用C语言开发的基础
	修改了Makfile文件,主要增加了将c文件编译成.hrb,然后和sys混编成img的处理
###V1.1.8
	增加了naskfunc.nas文件,其中包括HLT功能的汇编函数
	修改了bootpack.c文件,直接调用naskfunc.nas里的汇编函数
	修改Makefile,连接的时候,把naskfunc.nas文件连接到bootpack的中间文件里
###V2.1.1
	修改了naskfunc.nas文件,增加了往显存写入数据的函数
	修改了bootpack.c文件,往显存写入数据
###V2.1.2
	使用指针来往内存写内容
###V2.1.3
	修改了bootpack.c文件,增加了调色板设置
	修改了naskfunc.nas文件,增加了往端口读写数据的函数,和读写标志寄存去Eflags的函数
###V2.1.4
	修改了bootpack.c文件,增加了画出矩形的函数
###V2.1.5
	增加了hankaku文件:ASCII字符库
	修改了Makefile文件,在编译时将字符库文件一起编译
	修改了bootpack.c文件,成功输出“Roliy”
###V2.1.6
	修改了bootpack.c文件,使用了sprint()函数
	修改了bootpack.c文件,增加了初始化GDT和IDT的函数
	修改了naskfunc.nas文件,增加了初始化GDT和IDT的汇编实现
###V3.1.1
	将bootpack.c文件按功能分成了bootpack.c,dsctbl.c和graphic.c三个文件
	修改了Makefile文件,支持分割以后的文件的编译
###V3.1.2
	新增int.c文件,初始化PIC
	修改bootpack.c,bootpack.h和Makefile文件来编译
###V3.1.3
	修改了naskfunc.nas,dsctbl.c,bootpack.h,bottpack.c,int.c文件,增加了处理键盘中断的部分
###V4.1
	详细处理了键盘和鼠标中断:完成从按键到屏幕显示的实现
	增加了两个文件:
		equipment.c是鼠标和键盘准备的函数,之后可能会混合仅int.c中
		datastructure.c基础数据结构,目前实现了`队列`,用作鼠标键盘缓冲区
###V5.1
	实现了移动鼠标的功能,源文件分割成其他文件
###V6.1
	内存管理[未完成]
###V6.2
	完成内存释放部分
###V7.1
	内存4K处理
	显示采用多图层贴图来刷新显示
###V7.2.2
	更新显示的说明;
	完善图层的结构和显示流程
###V8.1.*
	完善Timer计时器功能,将鼠标键盘中断缓冲区和Timer中断缓冲区合并
	尝试用链表来管理Timer,目前来看还是不错的
	更新了鼠标键盘中断和Timer的说明
	新增了tolset.zip
###V9.1.1
	修正了内存分配中的一个bug,该bug会导致高分辨画面显示不正常
###V9.2
	增加了键盘按键编码和显示字符的映射,支持在屏幕上显示键盘输入字符
	增加了鼠标控制窗口移动的功能
###V10.1.*
	增加了多进程和多进程切换
	增加了一些多进程的说明
###V10.2
	修补多进程的BUG
