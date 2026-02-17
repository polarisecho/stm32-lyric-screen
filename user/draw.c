#include "stm32f10x.h"                  // Device header
#include "lcd12864.h"
#include "math.h"
#include "stdlib.h"

#define SWAP(A,B) {temp=A;A=B;B=temp;}
#define refreshlistmax 1024

/*
   X0-128(x0-8)原[0-16]————————>
YO
|
64	原[Y0-32]
|
|
|
\/   缓冲区位置：高八位x低八位y（原）
*/

uint16_t disbuf[8][64]={0x00};			//缓存
uint16_t refreshlist[refreshlistmax]={0x00};		//部分刷新位置列表
uint16_t refreshlistlen=0;					//部分刷新标志位

void Clear_disbuf(void)
{
	uint8_t i,j;
	for(i=0;i<8;i++)
	{
		for(j=0;j<64;j++)
		{
			disbuf[i][j]=0x0000;
		}
	}
}

void Fill_disbuf(void)
{
	uint8_t i,j;
	for(i=0;i<8;i++)
	{
		for(j=0;j<64;j++)
		{
			disbuf[i][j]=0x0000;
		}
	}
}

void Refresh_disbuf(uint16_t buf)		//地址刷新
{
	uint8_t x,y;
	x=buf/256;
	y=buf%64;
	if(y>31)
	{
		y-=32;
		x+=8;
	}
	Write_Command(0x80+y);
	Write_Command(0x80+x);
	Write_Data(disbuf[buf/256][buf%64]/256);
	Write_Data(disbuf[buf/256][buf%64]%256);
}


void Refresh_disbuf_All(void)				//全屏刷新
{
  uint8_t i,j;
	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j++)
		{
			Write_Command(0x80+j);
			Write_Command(0x80+i);
			Write_Data(disbuf[i][j]/256);
			Write_Data(disbuf[i][j]%256);
		}
	}
	for(i=0;i<8;i++)
	{
		for(j=0;j<32;j++)
		{
			Write_Command(0x80+j);
			Write_Command(0x88+i);
			Write_Data(disbuf[i][j+32]/256);
			Write_Data(disbuf[i][j+32]%256);
		}
	}
	refreshlistlen=0;
}

void Refresh_disbuf_List(void)		//刷新部分（列表内地址）
{
	uint16_t n=0;
	while(n<refreshlistlen)
	{
		Refresh_disbuf(refreshlist[n]);
		n++;
	}
	refreshlistlen=0;
}	

void Test_Refresh_disbuf(void)		//满了就刷新
{
	if(refreshlistlen==refreshlistmax-1)
	{
		Refresh_disbuf_List();
	}
}

void Write_disbuf(uint16_t buf,uint16_t dat,_Bool color)//color 0=关（底色），1=开（黑色）
{
	void Test_Refresh_disbuf(void);
	uint16_t i;
	uint8_t x,y;
	_Bool in=0;
	x=buf/256;
	y=buf%64;
	if(color)
	{
		disbuf[x][y]|=dat;
	}
	else
	{
		dat^=dat;
		disbuf[x][y]&=dat;
	}
	for(i=refreshlistlen;i>0&&i>refreshlistlen-5;i--)				//5次内写入的，只是为了快才写5
	{
		if(refreshlist[i]==buf)
		{
			in=1;
			break;
		}
	}	
	if(in==0)
	{
		refreshlist[refreshlistlen]=buf;
		refreshlistlen++;
	}
}

void Write_disbuf_r(uint16_t buf,uint16_t dat,_Bool color)//立刻刷新，只刷新当前位置，不改变列表
{
	uint8_t x,y;
	x=buf/256;
	y=buf%64;
	if(color)
	{
		disbuf[x][y]|=dat;
	}
	else
	{
		dat^=dat;
		disbuf[x][y]&=dat;
	}
	Refresh_disbuf(buf);
}

void Write_disbuf_n(uint16_t buf,uint16_t dat,_Bool color)//不改变列表，不刷新
{
	uint8_t x,y;
	x=buf/256;
	y=buf%64;
	if(color)
	{
		disbuf[x][y]|=dat;
	}
	else
	{
		dat^=dat;
		disbuf[x][y]&=dat;
	}
}

void SetPixel(uint16_t x, uint16_t y, _Bool color)
{
	if(x<128 && y<64)									//防炸
	{
		uint8_t a,b;
		a=x/16;
		b=x%16;
		Write_disbuf(a*256+y,0x8000>>b,color);
	}
}

void SetPixel_n(uint16_t x, uint16_t y, _Bool color)						//像素 不更新列表
{
	uint8_t a,b;
	a=x/16;
	b=x%16;
	if(color)
	{
		disbuf[a][y]|=0x8000>>b;
	}
	else
	{
		disbuf[a][y]=disbuf[a][y]&(uint16_t)(0x8000>>b);
	}
}

void DrawLine(int x1,int y1,int x2,int y2,_Bool color)					//画线			以及画圆和填充来自【单片机图形库开源分享 32 51 oled】 https://www.bilibili.com/video/BV1EC4y1872W/?share_source=copy_web&vd_source=5e17989c487ce7173c18acfa7a34fa30
{
	unsigned short us; 
	unsigned short usX_Current, usY_Current;
	
	
	int lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int lIncrease_X, lIncrease_Y; 	

	lDelta_X = x2 - x1; //计算坐标增量 
	lDelta_Y = y2 - y1; 

	usX_Current = x1; 
	usY_Current = y1; 

	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; 			//设置单步正方向 
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;			//垂直线 
	else 
	{ 
		lIncrease_X = -1;			//设置单步反方向 
		lDelta_X = - lDelta_X;
	} 

	//Y轴的处理方式与上图X轴的处理方式同理
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;			//水平线 
	else 
	{
		lIncrease_Y = -1;
		lDelta_Y = - lDelta_Y;
	} 

	//选取不那么陡的方向依次画点
	if ( lDelta_X > lDelta_Y )
		lDistance = lDelta_X;
	else 
		lDistance = lDelta_Y; 

	//依次画点 进入缓存区 画好后再刷新缓冲区就好啦
	for ( us = 0; us <= lDistance + 1; us ++ )					//画线输出 
	{
		SetPixel(usX_Current,usY_Current,color);	//画点 
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 

		if ( lError_X > lDistance ) 
		{
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  

		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 		
	}  
}

void DrawFastHLine(int x, int y, unsigned char w,_Bool color)			//快速横线
{
	int end = x+w,a,b;
	a=x/16;
	b=end/16;
	x=x%16;
	end=end%16;
	if(a==b)
	{
		Write_disbuf(a*256+y,((1U << (end - x )) - 1) << (15 - end),color);
	}
	else
	{
		Write_disbuf(a*256+y,0xFFFF >> x,color);
		Write_disbuf(b*256+y,0xFFFF <<(15-end+1),color);
		for(uint8_t t=a+1;t<b;t++)
		{
			Write_disbuf(t*256+y,0xFFFF,color);
		}
	}
}

void DrawFastVLine(int x, int y, unsigned char h,_Bool color)						//快速竖线
{
	for(uint8_t a=y;a<y+h;a++)
		SetPixel(x,a,color);
}

//画三角形
void DrawTriangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2,_Bool color)
{
	//很简单  就是画3条任意线
	DrawLine(x0, y0, x1, y1,color);
	DrawLine(x1, y1, x2, y2,color);
	DrawLine(x2, y2, x0, y0,color);
}

//填充三角形
void DrawFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2,_Bool color)
{
	int a, b, y, last,temp;
	int dx01, dy01,dx02,dy02,dx12,dy12,sa = 0,sb = 0;
	
	if (y0 > y1)
	{
		SWAP(y0, y1); SWAP(x0, x1);
	}
	if (y1 > y2)
	{
		SWAP(y2, y1); SWAP(x2, x1);
	}
	if (y0 > y1)
	{
		SWAP(y0, y1); SWAP(x0, x1);
	}
	if(y0 == y2)
	{
		a = b = x0;
		if(x1 < a)
		{
			a = x1;
		}
		else if(x1 > b)
		{
			b = x1;
		}
		if(x2 < a)
		{
			a = x2;
		}
		else if(x2 > b)
		{
			b = x2;
		}
		DrawFastHLine(a, y0, b-a+1,color);
		return;
	}
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1,
	sa = 0,
	sb = 0;
	if (y1 == y2)
	{
		last = y1;   // Include y1 scanline
	}
	else
	{
		last = y1-1; // Skip it
	}

	for(y = y0; y <= last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;

		if(a > b)
		{
			SWAP(a,b);
		}
		DrawFastHLine(a, y, b-a+1,color);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y <= y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b)
		{
			SWAP(a,b);
		}
		DrawFastHLine(a, y, b-a+1,color);
	}
}

void DrawRect(int left,int top,int width,int height, _Bool color)
{
	DrawFastHLine(left,top,width,color);
	DrawFastHLine(left,top+height-1,width,color);
	DrawFastVLine(left,top,height,color);
	DrawFastVLine(left+width-1,top,height,color);
}

void DrawFillRect(int left,int top,int width,int height, _Bool color)
{
	for(uint8_t a=top;a<top+height;a++)
	DrawFastHLine(left,a,width,color);
}

																																									//画圆
void DrawCircle ( int usX_Center, int usY_Center, int usRadius, _Bool color)
{
	short sCurrentX, sCurrentY;
	short sError;
	sCurrentX = 0; sCurrentY = usRadius;	  
	sError = 3 - ( usRadius << 1 );     //判断下个点位置的标志
	
	while ( sCurrentX <= sCurrentY )
	{
		//此处画圆打点的方法和画圆角矩形的四分之一圆弧的函数有点像
		SetPixel( usX_Center + sCurrentX, usY_Center + sCurrentY ,color);             //1，研究对象
		SetPixel( usX_Center - sCurrentX, usY_Center + sCurrentY ,color);             //2      
		SetPixel( usX_Center - sCurrentY, usY_Center + sCurrentX ,color);             //3
		SetPixel( usX_Center - sCurrentY, usY_Center - sCurrentX ,color);             //4
		SetPixel( usX_Center - sCurrentX, usY_Center - sCurrentY ,color);             //5       
		SetPixel( usX_Center + sCurrentX, usY_Center - sCurrentY ,color);             //6
		SetPixel( usX_Center + sCurrentY, usY_Center - sCurrentX ,color);             //7 
		SetPixel( usX_Center + sCurrentY, usY_Center + sCurrentX ,color);             //0
		sCurrentX ++;		
		if ( sError < 0 ) 
			sError += 4 * sCurrentX + 6;	  
		else
		{
			sError += 10 + 4 * ( sCurrentX - sCurrentY );   
			sCurrentY --;
		} 		
	}
}

//————————反转————————
void Write_Invertdisbuf(uint16_t buf,uint16_t dat)//color 0=关（底色），1=开（黑色）
{
	void Test_Refresh_disbuf(void);
	uint16_t i;
	uint8_t x,y;
	_Bool in=0;
	x=buf/256;
	y=buf%64;
	
	//disbuf[x][y]=disbuf[x][y]&(~dat)+~(disbuf[x][y]&dat);
	disbuf[x][y] ^= dat;
	for(i=refreshlistlen;i>0&&i>refreshlistlen-5;i--)				//5次内写入的，只是为了快才写5
	{
		if(refreshlist[i]==buf)
		{
			in=1;
			break;
		}
	}	
	if(in==0)
	{
		refreshlist[refreshlistlen]=buf;
		refreshlistlen++;
	}
}

void InvertPixel(uint16_t x, uint16_t y)
{
	if(x<128 && y<64)									//防炸
	{
		uint8_t a,b;
		a=x/16;
		b=x%16;
		Write_Invertdisbuf(a*256+y,0x8000>>b);
	}
}
void DrawInvertLine(int x1,int y1,int x2,int y2)					//画线			以及画圆和填充来自【单片机图形库开源分享 32 51 oled】 https://www.bilibili.com/video/BV1EC4y1872W/?share_source=copy_web&vd_source=5e17989c487ce7173c18acfa7a34fa30
{
	unsigned short us; 
	unsigned short usX_Current, usY_Current;
	
	
	int lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int lIncrease_X, lIncrease_Y; 	

	lDelta_X = x2 - x1; //计算坐标增量 
	lDelta_Y = y2 - y1; 

	usX_Current = x1; 
	usY_Current = y1; 

	if ( lDelta_X > 0 ) 
		lIncrease_X = 1; 			//设置单步正方向 
	else if ( lDelta_X == 0 ) 
		lIncrease_X = 0;			//垂直线 
	else 
	{ 
		lIncrease_X = -1;			//设置单步反方向 
		lDelta_X = - lDelta_X;
	} 

	//Y轴的处理方式与上图X轴的处理方式同理
	if ( lDelta_Y > 0 )
		lIncrease_Y = 1; 
	else if ( lDelta_Y == 0 )
		lIncrease_Y = 0;			//水平线 
	else 
	{
		lIncrease_Y = -1;
		lDelta_Y = - lDelta_Y;
	} 

	//选取不那么陡的方向依次画点
	if ( lDelta_X > lDelta_Y )
		lDistance = lDelta_X;
	else 
		lDistance = lDelta_Y; 

	//依次画点 进入缓存区 画好后再刷新缓冲区就好啦
	for ( us = 0; us <= lDistance + 1; us ++ )					//画线输出 
	{
		InvertPixel(usX_Current,usY_Current);	//画点 
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 

		if ( lError_X > lDistance ) 
		{
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  

		if ( lError_Y > lDistance ) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 		
	}  
}

void DrawInvertHLine(int x, int y, unsigned char w)			//快速横线
{
	int end = x+w,a,b;
	a=x/16;
	b=end/16;
	x=x%16;
	end=end%16;
	if(a==b)
	{
		Write_Invertdisbuf(a*256+y,((1U << (end - x )) - 1) << (15 - end));
	}
	else
	{
		Write_Invertdisbuf(a*256+y,0xFFFF >> x);
		Write_Invertdisbuf(b*256+y,0xFFFF <<(15-end+1));
		for(uint8_t t=a+1;t<b;t++)
		{
			Write_Invertdisbuf(t*256+y,0xFFFF);
		}
	}
}
void DrawInvertVLine(int x, int y, unsigned char h)						//快速竖线
{
	for(uint8_t a=y;a<y+h;a++)
		InvertPixel(x,a);
}

void DrawInvertRect(int left,int top,int width,int height)
{
	DrawInvertHLine(left,top,width);
	DrawInvertHLine(left,top+height-1,width);
	DrawInvertVLine(left,top,height);
	DrawInvertVLine(left+width-1,top,height);
}

void DrawInvertFillRect(int left,int top,int width,int height)
{
	for(uint8_t a=top;a<top+height;a++)
	DrawInvertHLine(left,a,width);
}

void DrawInvertTriangle(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2)
{
	//很简单  就是画3条任意线
	DrawInvertLine(x0, y0, x1, y1);
	DrawInvertLine(x1, y1, x2, y2);
	DrawInvertLine(x2, y2, x0, y0);
}

void DrawInvertFillTriangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
	int a, b, y, last,temp;
	int dx01, dy01,dx02,dy02,dx12,dy12,sa = 0,sb = 0;
	
	if (y0 > y1)
	{
		SWAP(y0, y1); SWAP(x0, x1);
	}
	if (y1 > y2)
	{
		SWAP(y2, y1); SWAP(x2, x1);
	}
	if (y0 > y1)
	{
		SWAP(y0, y1); SWAP(x0, x1);
	}
	if(y0 == y2)
	{
		a = b = x0;
		if(x1 < a)
		{
			a = x1;
		}
		else if(x1 > b)
		{
			b = x1;
		}
		if(x2 < a)
		{
			a = x2;
		}
		else if(x2 > b)
		{
			b = x2;
		}
		DrawInvertHLine(a, y0, b-a+1);
		return;
	}
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1,
	sa = 0,
	sb = 0;
	if (y1 == y2)
	{
		last = y1;   // Include y1 scanline
	}
	else
	{
		last = y1-1; // Skip it
	}

	for(y = y0; y <= last; y++)
	{
		a   = x0 + sa / dy01;
		b   = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;

		if(a > b)
		{
			SWAP(a,b);
		}
		DrawInvertHLine(a, y, b-a+1);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y <= y2; y++)
	{
		a   = x1 + sa / dy12;
		b   = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b)
		{
			SWAP(a,b);
		}
		DrawInvertHLine(a, y, b-a+1);
	}
}


void DrawInvertCircle ( int usX_Center, int usY_Center, int usRadius)
{
	short sCurrentX, sCurrentY;
	short sError;
	sCurrentX = 0; sCurrentY = usRadius;	  
	sError = 3 - ( usRadius << 1 );     //判断下个点位置的标志
	
	while ( sCurrentX <= sCurrentY )
	{
		//此处画圆打点的方法和画圆角矩形的四分之一圆弧的函数有点像
		InvertPixel( usX_Center + sCurrentX, usY_Center + sCurrentY);             //1，研究对象
		InvertPixel( usX_Center - sCurrentX, usY_Center + sCurrentY);             //2      
		InvertPixel( usX_Center - sCurrentY, usY_Center + sCurrentX);             //3
		InvertPixel( usX_Center - sCurrentY, usY_Center - sCurrentX);             //4
		InvertPixel( usX_Center - sCurrentX, usY_Center - sCurrentY);             //5       
		InvertPixel( usX_Center + sCurrentX, usY_Center - sCurrentY);             //6
		InvertPixel( usX_Center + sCurrentY, usY_Center - sCurrentX);             //7 
		InvertPixel( usX_Center + sCurrentY, usY_Center + sCurrentX);             //0
		sCurrentX ++;		
		if ( sError < 0 ) 
			sError += 4 * sCurrentX + 6;	  
		else
		{
			sError += 10 + 4 * ( sCurrentX - sCurrentY );   
			sCurrentY --;
		} 		
	}
}
//————————文字————————
void DrawChar16(int x,int y,uint8_t *text,_Bool color)					//16*16字
{
	int a=x/16;
	x=x%16;
	if(x &&a<7 &&a>-1)				//不在完整区域
	{
		for(uint8_t b=0;b<16 &&y+b<64;b++)
		{
			
			Write_disbuf(a*256+y+b,((text[2*b]<<8)+text[2*b+1])>>x,color);
			
			Write_disbuf(a*256+256+y+b,((text[2*b]<<8)+text[2*b+1])<<(16-x),color);
		}
	}
	else if(a<8 &&a>-1)
	{
		for(uint8_t b=0;b<16 &&y+b<64;b++)
		{
			Write_disbuf(a*256+y+b,((text[2*b]<<8)+text[2*b+1]),color);
		}
	}
}

//void DrawInvertChar16(int x,int y,uint8_t *text)					//16*16字
//{
//	uint8_t a=x/16;
//	x=x%16;
//	if(x)
//	{
//		for(uint8_t b=0;b<16;b++)
//		{
//			disbuf[a][y+b]=(((text[2*b]<<8)+text[2*b+1])>>x)^disbuf[a][y+b];
//			disbuf[a+1][y+b]=(((text[2*b]<<8)+text[2*b+1])<<(16-x))^disbuf[a+1][y+b];
//		}
//	}
//	else
//	{
//		for(uint8_t b=0;b<16;b++)
//		{
//			disbuf[a+1][y+b]=(((text[2*b]<<8)+text[2*b+1]))^disbuf[a+1][y+b];
//		}
//	}
//}
void DrawChar32(int x,int y,uint8_t *text,_Bool color)			//32*32字
{
	int a=0;
	if(x>0)
	{
		a=x/16;
		x=x%16;
	}
	if(x && a<8 && a>-2)				//不在完整区域
	{
		for(uint8_t b=0;b<32 &&y+b<64;b++)
		{
			if(y+b>-1)
			{
				uint32_t line_data = (text[4*b] << 24) | (text[4*b+1] << 16) | (text[4*b+2] << 8) | text[4*b+3];
				if(a>-1)
					Write_disbuf(a*256+y+b,(line_data >> (16 + x)) & 0xFFFF,color);
				Write_disbuf(a*256+256+y+b,(line_data >> x) & 0xFFFF,color);
				if(a<6)
					Write_disbuf(a*256+512+y+b,(line_data << (16 - x)) & 0xFFFF,color);
				if(x<0)
					Write_disbuf(256+y+b,(line_data<<-x & 0xFFFF),color);
			}
		}
	}
	else if(a<8)
	{ 
		for(uint8_t b=0;b<32 &&y+b<64;b++)
		{
			Write_disbuf(a*256+y+b,((text[4*b]<<8)+text[4*b+1]),color);
			if(a<7)
				Write_disbuf(a*256+256+y+b,((text[4*b+2]<<8)+text[4*b+3]),color);
		}
	}
}

void DrawChar64(int x, int y, uint8_t *text, _Bool color)
{
    int a = x / 16;
    x = x % 16;
    
    uint16_t base_addr = a * 256 + y;
    
    if(x && a < 4 && a > -1) // 不在完整区域
    {
        for(uint8_t b = 0; b < 64 && y + b < 64 && y + b > 0; b++)
        {
            uint16_t current_addr = base_addr + b;
            
            // 将8个字节组合成一个64位数据（左高顺序）
            uint64_t line_data = 
                ((uint64_t)text[8*b] << 56) | 
                ((uint64_t)text[8*b+1] << 48) | 
                ((uint64_t)text[8*b+2] << 40) | 
                ((uint64_t)text[8*b+3] << 32) | 
                ((uint64_t)text[8*b+4] << 24) | 
                ((uint64_t)text[8*b+5] << 16) | 
                ((uint64_t)text[8*b+6] << 8) | 
                (uint64_t)text[8*b+7];
            
            // 对64位数据进行左移x位
            
            
            // 提取5个16位段
            uint16_t seg1 = (line_data >> (48+x)) & 0xFFFF;  // 高16位
            uint16_t seg2 = (line_data >> (32+x)) & 0xFFFF;  // 次高16位
            uint16_t seg3 = (line_data >> (16+x)) & 0xFFFF;  // 中间16位
            uint16_t seg4 = (line_data >> x) & 0xFFFF;       // 次低16位
            uint16_t seg5 = (line_data << (16-x))&0xFFFF;   // 低16位（通过移位提取）
            
            // 跳过零数据部分
            if(seg1 != 0) Write_disbuf(current_addr, seg1, color);
            if(seg2 != 0) Write_disbuf(current_addr + 256, seg2, color);
            if(seg3 != 0) Write_disbuf(current_addr + 512, seg3, color);
            if(seg4 != 0) Write_disbuf(current_addr + 768, seg4, color);
            if(seg5 != 0) Write_disbuf(current_addr + 1024, seg5, color);
        }
    }
    else if(a < 5 && a > -1) // 完整区域
    {
        for(uint8_t b = 0; b < 64 && y + b < 64; b++)
        {
            uint32_t current_addr = base_addr + b;
            
            // 写入四个16位段（左高顺序）
            uint16_t part1 = (text[8*b] << 8) | text[8*b+1];
            uint16_t part2 = (text[8*b+2] << 8) | text[8*b+3];
            uint16_t part3 = (text[8*b+4] << 8) | text[8*b+5];
            uint16_t part4 = (text[8*b+6] << 8) | text[8*b+7];
            
            if(part1 != 0) Write_disbuf(current_addr, part1, color);
            if(part2 != 0) Write_disbuf(current_addr + 256, part2, color);
            if(part3 != 0) Write_disbuf(current_addr + 512, part3, color);
            if(part4 != 0) Write_disbuf(current_addr + 768, part4, color);
        }
    }
}
