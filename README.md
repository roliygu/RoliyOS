RoliyOS--My small OS
=======
版本说明：
--------
V1.1.3
##
	用变量的方式修改了Makefile<br>
	增加了检测扇区是否错误的代码<br>
V1.1.4
##
	将检测扇区错误扩展到了柱面层次，并加入了定义常量的语句<br>
V1.1.5
##
	新增了haribote.nas：将fin函数单独写到了这个文件中<br>
	修改了ipl.nas和Mackfile文件<br>
		ipl.nas 将fin函数的地址设置为haribote中的位置了<br>
		Makefile文件修改了制造img的语句，输入变成了haribote.sys和ipl.bin等<br>
V1.1.5.1
##
	修改了haribote.nas文件，使得调用后屏幕全黑<br>
V1.1.6
##
	修改了haribote.nas文件，在进入32位状态前收集好BIOS信息，[因为进入32位状态后，再回来收集信息会很麻烦]<br>
V1.1.7
##
	修改了haribote.nas文件，将其重命名为asmhead.nas，其包含了原来haribote.nas的汇编功能和将C的机器语言和汇编混合的功能<br>
	新增了bootpack.c文件，作为用C语言开发的基础<br>
	修改了Makfile文件，主要增加了将c文件编译成.hrb，然后和sys混编成img的处理<br>
V1.1.8
##
	增加了naskfunc.nas文件，其中包括HLT功能的汇编函数<br>
	修改了bootpack.c文件，直接调用naskfunc.nas里的汇编函数<br>
	修改Makefile，连接的时候，把naskfunc.nas文件连接到bootpack的中间文件里<br>
V2.1.1
##
	修改了naskfunc.nas文件，增加了往显存写入数据的函数<br>
	修改了bootpack.c文件，往显存写入数据<br>
V2.1.2
##
	使用指针来往内存写内容<br>
V2.1.3
##
	修改了bootpack.c文件，增加了调色板设置<br>
	修改了naskfunc.nas文件，增加了往端口读写数据的函数，和读写标志寄存去Eflags的函数<br>
V2.1.4
##
	修改了bootpack.c文件，增加了画出矩形的函数<br>
V2.1.5
##
	增加了hankaku文件：ASCII字符库<br>
	修改了Makefile文件，在编译时将字符库文件一起编译<br>
	修改了bootpack.c文件，成功输出“Roliy”<br>
V2.1.6
##
	修改了bootpack.c文件，使用了sprint()函数<br>
	修改了bootpack.c文件，增加了初始化GDT和IDT的函数<br>
	修改了naskfunc.nas文件，增加了初始化GDT和IDT的汇编实现<br>
笔记与说明：
--------
前言
##
	这个projet来源于《30天自制操作系统》一书，也是为了锻炼一下底层编程和管理多个源文件的能力<br>
	这里的笔记先按照章节的顺序整理一下，之后再考虑其他更好的整理方式
Chapter 1
##
	> 开发OS的步骤：
	>> 在Windows（或其他）系统上编写源代码
	>> 用C语言编译器编译源代码，生成机器语言文件
	>> 对机器语言文件进行加工，生成映像文件
	>> 将映像文件写入磁盘，做成OS的启动盘
