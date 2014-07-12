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
