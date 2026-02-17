#ifndef __DRAW_H__
#define __DRAW_H__

#include "stm32f10x.h"   // Device header

void Clear_disbuf(void);		//清楚屏幕缓存-白
void Fill_disbuf(void);			//填充屏幕缓存-黑
void Refresh_disbuf(uint16_t buf);		//刷新本位置 buf：x前三 y后六 -----xxx --yyyyyy
void Refresh_disbuf_All(void);				//全屏刷新
void Refresh_disbuf_List(void);				//刷新部分（列表内地址）
void Test_Refresh_disbuf(void);				//列表满了就刷新，防内存爆
void Write_disbuf(uint16_t buf,uint16_t dat,_Bool color);		//写入缓存，添加至列表 color 0=关（底色），1=开（黑色）
void Write_disbuf_r(uint16_t buf,uint16_t dat,_Bool color); //立刻刷新，只刷新当前位置，不改变列表
void Write_disbuf_n(uint16_t buf,uint16_t dat,_Bool color);	//不改变列表，不刷新
void SetPixel(uint16_t x, uint16_t y, _Bool color);							//像素设置
void DrawLine(int x0, int y0, int x1, int y1, _Bool color);			//随意线
void DrawFastHLine(int x, int y, unsigned char w,_Bool color);		//快速横线
void DrawFastVLine(int x, int y, unsigned char h,_Bool color);		//快速竖线
void DrawTriangle(int x1,int y1,int x2,int y2,int x3,int y3,_Bool color);					//画三角
void DrawFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2,_Bool color);//画实心三角
void DrawRect(int left,int top,int width,int height, _Bool color);								//画矩形
void DrawFillRect(int left,int top,int width,int height, _Bool color);						//画实心矩形
void DrawCircle ( int usX_Center, int usY_Center, int usRadius, _Bool color);			//画圆

void Write_Invertdisbuf(uint16_t buf,uint16_t dat);		//反转系列，一样
void DrawInvertLine(int x1,int y1,int x2,int y2);
void DrawInvertHLine(int x, int y, unsigned char w);
void DrawInvertVLine(int x, int y, unsigned char h);
void DrawInvertRect(int left,int top,int width,int height);
void DrawInvertFillRect(int left,int top,int width,int height);
void DrawInvertTriangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void DrawInvertFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
void DrawInvertCircle ( int usX_Center, int usY_Center, int usRadius);

void DrawChar16(int x,int y,uint8_t *text,_Bool color);			//16*16(uint8_t[32]储存方式)字体/图案
void DrawChar32(int x,int y,uint8_t *text,_Bool color);			//32*32:
void DrawChar64(int x,int y,uint8_t *text,_Bool color);			//64*64:




#endif
