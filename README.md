RoliyOS
=======

My small OS
=======
版本说明：
V1.1.3
	用变量的方式修改了Makefile
	增加了检测扇区是否错误的代码
V1.1.4
	将检测扇区错误扩展到了柱面层次，并加入了定义常量的语句
V1.1.5
	新增了haribote.nas：将fin函数单独写到了这个文件中
	修改了ipl.nas和Mackfile文件
		ipl.nas 将fin函数的地址设置为haribote中的位置了
		Makefile文件修改了制造img的语句，输入变成了haribote.sys和ipl.bin等
V1.1.5.1
	修改了haribote.nas文件，使得调用后屏幕全黑
V1.1.6
	修改了haribote.nas文件，在进入32位状态前收集好BIOS信息，[因为进入32位状态后，再回来收集信息会很麻烦]
V1.1.7
	修改了haribote.nas文件，将其重命名为asmhead.nas，其包含了原来haribote.nas的汇编功能和将C的机器语言和汇编混合的功能
	新增了bootpack.c文件，作为用C语言开发的基础
	修改了Makfile文件，主要增加了将c文件编译成.hrb，然后和sys混编成img的处理
V1.1.8
	增加了naskfunc.nas文件，其中包括HLT功能的汇编函数
	修改了bootpack.c文件，直接调用naskfunc.nas里的汇编函数
	修改Makefile，连接的时候，把naskfunc.nas文件连接到bootpack的中间文件里