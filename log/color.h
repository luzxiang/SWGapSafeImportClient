﻿/*
 * color.h
 *
 *  Created on: 2018年1月6日
 *      Author: root
 */

#ifndef COLOR_H_
#define COLOR_H_
/***********************************************************************
 例如：记得在打印完之后，把颜色恢复成 NONE()，不然再后面的打印都会跟着变色。
 printf(ANSI("1;31")"####----->> "ANSI("0;32")"hello\n" ANSI("0"));
 printf(HLIGHT()FGROUND(CYAN)"current function is %s "FGROUND(GREEN)" file line is %d\n"NONE(),
 __FUNCTION__, __LINE__ );
 fprintf(stderr, HLIGHT()FGROUND(RED) "current function is %s " FGROUND(BLUE) " file line is %d\n" NONE(),
 __FUNCTION__, __LINE__ );
 ***********************************************************************/

/*********************************************
 * 颜色序号，只能配合FGROUND(x) BGROUND(x)使用
 * *******************************************/
#define DARY     	"0" 			//黑色：前景30、背景40
#define RED       	"1" 			//红色：前景31、背景41
#define GREEN     	"2" 			//绿色：前景32、背景42
#define YELLOW     	"3" 			//黄色：前景33、背景43
#define BLUE       	"4" 			//蓝色：前景34、背景44
#define PURPLE     	"5" 			//紫色：前景35、背景45
#define CYAN       	"6" 			//深绿：前景36、背景46
#define WHITE      	"7" 			//白色：前景37、背景47
#define FGROUND(x)	"\033[3" x "m"	//设置前景色，x:颜色序号
#define BGROUND(x)	"\033[4" x "m"	//设置背景色，x:颜色序号

/*********************************************
 * ANSI控制函数
 * *******************************************/
#define ANSI(x)		"\033[" x "m"		//设置ANSI属性 x:可以是控制码的组合，每个参数用;隔开。例如ANSI(1;31)表示设置为高亮，前景红色
#define NONE()    	"\033[m"		//关闭所有属性,恢复成默认值
#define HLIGHT()   	ANSI("1")    		//设置高亮
#define UNLINE()	ANSI("4")    		//设置下划线
#define FLASH()   	ANSI("5")    		//设置闪烁
#define RSHOW()		ANSI("7")    		//设置反显

/***********************************************************************
 ANSI控制码，颜色只是以下控制码中的一种：
 \033[0m 	关闭所有属性
 \033[1m 	设置高亮度
 \033[4m   	下划线
 \033[5m   	闪烁
 \033[7m   	反显
 \033[8m   	消隐
 \033[30m -- \033[37m 设置前景色
 \033[40m -- \033[47m 设置背景色
 \033[nA 	光标上移n行
 \033[nB 	光标下移n行
 \033[nC 	光标右移n行
 \033[nD 	光标左移n行
 \033[y;xH	设置光标位置
 \033[2J 	清屏
 \033[K 		清除从光标到行尾的内容
 \033[s 		保存光标位置
 \033[u 		恢复光标位置
 \033[?25l 	隐藏光标
 \033[?25h 	显示光标
 ***********************************************************************/

#endif /* COLOR_H_ */
