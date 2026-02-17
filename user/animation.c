#include "stm32f10x.h"                  // Device header
#include "lcd12864.h"
#include "math.h"
#include "draw.h"
#include "stdlib.h"
#include "animation.h"
#include "delay.h"

extern volatile uint8_t state;	
extern _Bool endflag;
extern uint8_t uartbuf[];
uint32_t random_seed = 46563;
extern uint16_t disbuf[8][64];

_Bool endflag=0;		//特效收尾标志位，串口中断收到即将结束置1

//字数特效函数列表
typedef void (*func_ptr)(uint8_t num,_Bool color);

//特效放里面			字数|特效数 特效需要在对应字数中，16部分范围有特殊操作（合并，在random16里有）
func_ptr const c64_1[]={bigmiddle64};
func_ptr const c64_2[]={bigmiddle64};

func_ptr const c32_1[]={Normalwrite32};
func_ptr const c32_2[]={Normalwrite32};
func_ptr const c32_3[]={Widen32};
func_ptr const c32_4[]={Widen32,Rectupdown32};
func_ptr const c32_5[]={Compression32};
func_ptr const c32_6[]={Normalwrite32,Rectupdown32};
func_ptr const c32_7[]={Normalwrite32};
func_ptr const c32_8[]={Normalwrite32,Rectupdown32};
func_ptr const c32_9[]={Normalwrite32};
func_ptr const c32_10[]={Normalwrite32};

func_ptr const c16_1[]={Floattext_16,Normalwrite16};
func_ptr const c16_2[]={Floattext_16,Normalwrite16};
func_ptr const c16_9to16[]={Floattext_16,Normalwrite16};
func_ptr const c16_17to24[]={Normalwrite16};
func_ptr const c16_25to32[]={Normalwrite16};

//列表特效数量
uint8_t const c64num[] ={sizeof(c64_1)/sizeof(c64_1[0]),		//64大小 获取特效列表数量
		sizeof(c64_2)/sizeof(c64_2[0])};

uint8_t const c32num[] ={sizeof(c32_1)/sizeof(c32_1[0]),
		sizeof(c32_2)/sizeof(c32_2[0]),
		sizeof(c32_3)/sizeof(c32_3[0]),
		sizeof(c32_4)/sizeof(c32_4[0]),
		sizeof(c32_5)/sizeof(c32_5[0]),
		sizeof(c32_6)/sizeof(c32_6[0]),
		sizeof(c32_7)/sizeof(c32_7[0]),
		sizeof(c32_8)/sizeof(c32_8[0]),
		sizeof(c32_9)/sizeof(c32_9[0]),
		sizeof(c32_10)/sizeof(c32_10[0])};

uint8_t const c16num[]={sizeof(c16_1)/sizeof(c16_1[0]),
		sizeof(c16_2)/sizeof(c16_1[0]),
//		sizeof(c16_3)/sizeof(c16_1[0]),
//		sizeof(c16_4)/sizeof(c16_1[0]),
//		sizeof(c16_5)/sizeof(c16_1[0]),
//		sizeof(c16_6)/sizeof(c16_1[0]),
//		sizeof(c16_7)/sizeof(c16_1[0]),
//		sizeof(c16_8)/sizeof(c16_1[0]),
		sizeof(c16_9to16)/sizeof(c16_1[0]),
		sizeof(c16_17to24)/sizeof(c16_1[0]),
		sizeof(c16_25to32)/sizeof(c16_1[0]),};


//列表
func_ptr const *c64list[] = {c64_1,c64_2};
func_ptr const *c32list[] = {c32_1,c32_2,c32_3,c32_4,c32_5,c32_6,c32_7,c32_8,c32_9,c32_10};
func_ptr const *c16list[]= {c16_1,c16_2,c16_9to16,c16_17to24,c16_25to32};//暂时认为字数这么少的不会用到16大小，吧

//随机选择
void random64(uint8_t num,_Bool color)
{
	bigmiddle64(num,color);
	//(c64list[num-1])[rand_simple()%c64num[num-1]](num,color);
}

void random32(uint8_t num,_Bool color)
{
	(c32list[num-1])[rand_simple()%c32num[num-1]](num,color);
}

void random16(uint8_t num,_Bool color)
{
	if(num<=2)
		(c16list[num-1])[rand_simple()%c16num[num-1]](num,color);
	else if(num<=16)
		(c16list[2])[rand_simple()%c16num[2]](num,color);
	else if(num<=24)
		(c16list[3])[rand_simple()%c16num[3]](num,color);
	else
		(c16list[4])[rand_simple()%c16num[4]](num,color);
}


//简易随机数
uint32_t rand_simple(void) {
    random_seed = (random_seed * 735545 +uartbuf[12]) & 0x7FFFFFFF;
    return random_seed;
}


void Draw_RadialLines(int num,_Bool color)
{
	random_seed+=num*num;
	for(;num>0;num--)
		{
			DrawLine(64,32,rand_simple()%128,rand_simple()%64,color);
		}
}



int location[32][2];										//字临时坐标，特效内部使用，上限32

//————————64——————————————

void bigmiddle64(uint8_t num,_Bool color)			//中间大字，1-2
{
	Clear_disbuf();
	while(!endflag)
	{
	Delay_ms(20);
		if(num==1)
		{
			DrawChar64(32,0,uartbuf,color);
		}
		else if(num==2)
		{
			DrawChar64(0,0,uartbuf,1);
			DrawChar64(64,0,uartbuf+512,1);
		}
		Refresh_disbuf_All();
	}
}

//————————32——————————————

void Normalwrite32(uint8_t num,_Bool color)
{
	//Delay_ms(20);
	while(!endflag)
	{
		Clear_disbuf();
		if(num<9)
		{
			for(uint8_t n=0;n<num;n++)
			{
				DrawChar32(n%4*32,n/4*32,uartbuf+128*n,color);
			}
		}
		else if(num==9)
		{
			for(uint8_t n=0;n<num;n++)
			{
				DrawChar32(n%5*26-4,n/5*32,uartbuf+128*n,color);
			}
		}
		else if(num==10)
		{
			for(uint8_t n=0;n<num;n++)
			{
				DrawChar32(n%5*26-4,n/5*32,uartbuf+128*n,color);
			}
		}
		Refresh_disbuf_All();
	}
}

void Widen32(uint8_t num,_Bool color)		//3，4
{		
	uint8_t a=64/(num+1);
	_Bool change=0;
	for(uint8_t b=0;b<num;b++)		//|  xxxx  |
	{
		location[b][0]=32+b*a;
	}
	while(!endflag)
	{
		Clear_disbuf();
		for(uint8_t n=0;n<num;n++)
		{
			DrawChar32(location[n][0],16,uartbuf+128*n,color);
		}
		if(location[0][0]>-15)
		{
			if(num==3)
			{
				location[0][0]-=1;
				location[2][0]+=1;
			}
			else if(num==4)
			{
				if(change)
				{
					location[1][0]-=1;
					location[2][0]+=1;
				}
				location[0][0]-=1;
				location[3][0]+=1;
				change=!change;
				DrawCircle(64,32,64-location[1][0]+2,1);
				DrawCircle(64,32,64-location[1][0],1);
			}
		}
		DrawCircle(64,32,64-location[0][0],1);
		DrawCircle(64,32,64-location[0][0]+2,1);
		
		Refresh_disbuf_All();
	}
}

void Compression32(uint8_t num,_Bool color)   //5
{
	location[0][0]=-8;location[1][0]=20;location[2][0]=48;location[3][0]=76;location[4][0]=104;
	location[0][1]=location[1][1]=location[2][1]=location[3][1]=location[4][1]=16;
	_Bool change=0,up=rand_simple()%2;
	while(!endflag && location[0][1]<48&& location[4][1]<48)
	{
		Clear_disbuf();
		for(uint8_t n=0;n<num;n++)
		{
			DrawChar32(location[n][0],location[n][1],uartbuf+128*n,color);
		}
		if(up)
		{
			location[0][1]-=1;location[4][1]+=1;
			if(change) 
			{
				location[1][1]-=1;location[3][1]+=1;
			}
		}
		else
		{
			location[0][1]+=1;location[4][1]-=1;
			if(change) 
			{
				location[1][1]+=1;location[3][1]-=1;
			}
		}
		change=!change;
		Refresh_disbuf_All();
	}
	while(!endflag);
}

void Rectupdown32(uint8_t num,_Bool color)//6,8
{
	uint8_t temp=num/2;
	uint8_t rectnum=1;
	while(!endflag)
	{
		Clear_disbuf();
		for(uint8_t n=0;n<num;n++)
			{
			
				DrawChar32(n%temp*32+16*(4-temp),n/temp*32,uartbuf+128*n,color);
			}
		if(rectnum<31)
		{
			//for(uint8_t n=0;n<=rectnum;n++)
				DrawInvertFillRect(64-temp*rectnum/2,32-rectnum,temp*rectnum,2*rectnum);
		}
		else
		{
			DrawInvertFillRect(64-temp*30/2,2,temp*30,60);
			uint8_t n=rectnum-30;
			//for(uint8_t n=10;n>=rectnum-10;n--)
			DrawInvertFillRect(64-temp*n/2,32-n,temp*n,2*n);
		}
		rectnum++;
		if(rectnum>60)
			rectnum=0;
		Refresh_disbuf_All();
		//Delay_ms(1);
	}
}
//————————16——————————————

void Floattext_16(uint8_t num,_Bool color)     //3-10
{
	int cenx=64+rand_simple()%21-11,ceny=32+rand_simple()%21-11;
	int a=128/(num+1);
	for(int b=0,n=0;n<num;b+=a,n++)
	{
		location[n][0]=b+rand_simple()%a;			//x随机
		location[n][1]=16+rand_simple()%32;		//y随机
	}
	Clear_disbuf();
	Refresh_disbuf_All();
	while(!endflag)					//渲染，运算，清除
	{
		Clear_disbuf();
		for(int n=0;n<num;n++)
		{
			//DrawLine(cenx,ceny,location[n][0]+8,location[n][1]+8,color);
			DrawChar16(location[n][0],location[n][1],uartbuf+32*n,color);
			//DrawRect(location[n][0],location[n][1],16,16,color);
			if(num<7)
			{
				DrawLine(cenx,ceny,location[n][0]+8,location[n][1]+8,color);
				DrawCircle(cenx,ceny,sqrt((location[n][0]+8-cenx)*(location[n][0]+8-cenx)+(location[n][1]+8-ceny)*(location[n][1]+8-ceny)),1);//圆环 容易卡
			}
		}
		if(num>6&&num<12)
			{
				DrawCircle(cenx,ceny,sqrt((location[0][0]+8-cenx)*(location[0][0]+8-cenx)+(location[0][1]+8-ceny)*(location[0][1]+8-ceny)),1);
				DrawCircle(cenx,ceny,sqrt((location[3][0]+8-cenx)*(location[3][0]+8-cenx)+(location[3][1]+8-ceny)*(location[3][1]+8-ceny)),1);
				//DrawCircle(cenx,ceny,sqrt((location[num][0]+8-cenx)*(location[num][0]+8-cenx)+(location[num][1]+8-ceny)*(location[num][1]+8-ceny)),1);
				DrawCircle(cenx,ceny,sqrt((location[5][0]+8-cenx)*(location[5][0]+8-cenx)+(location[5][1]+8-ceny)*(location[5][1]+8-ceny)),1);
			}
		Refresh_disbuf_All();
		for(int n=0;n<num;n++)
		{
			location[n][0]+=rand_simple()%3-1;
			location[n][1]+=rand_simple()%3-1;
			if(location[n][0]<2)
				location[n][0]+=3;
			else if(location[n][0]>122)
				location[n][0]-=3;
			if(location[n][1]<2)
				location[n][1]+=3;
			else if(location[n][1]>56)
				location[n][1]-=3;
		}
	}
}

void Normalwrite16(uint8_t num,_Bool color)
{
	//Delay_ms(20);
	while(!endflag)
	{
		Clear_disbuf();
		for(uint8_t n=0;n<num;n++)
		{
			DrawChar16(n%8*16,n/8*16,uartbuf+32*n,color);
		}
		Refresh_disbuf_All();
		
		Clear_disbuf();
		for(uint8_t n=0;n<num;n++)
		{
			DrawChar16(n%8*16,n/8*16,uartbuf+32*n,color);
		}
		Refresh_disbuf_All();
	}
}
