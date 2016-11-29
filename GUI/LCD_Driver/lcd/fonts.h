/*										
*********************************************************************************************************
*	                                  
*	模块名称 : 字库模块
*	文件名称 : fonts.h
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef __FONTS_H_
#define __FONTS_H_

extern unsigned char const g_Ascii16[];

extern unsigned char const g_Hz16[];

extern unsigned char const g_Ascii12[];

extern unsigned char const g_Hz12[];

#define USE_SMALL_FONT	        /* 定义此行表示使用小字库 */

//#define HZK16_ADDR 0x803EA00		/* 汉字库地址， 这个地址在CPU内部Flash，位于主程序区的后面一部分 */

#endif

