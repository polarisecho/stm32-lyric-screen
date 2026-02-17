#include "stm32f10x.h"                  // Device header
#include "delay.h"
//#include "LCD12864.H"


// 引脚定义
/*
A0-A7
RS  = A12  // 数据/指令选择
RW  = A11  // 读/写控制
E   = A10  // 使能信号
PSB = A9  // 并口模式选择
RST = A8  // 复位信号
*/
#define PSB GPIOA,GPIO_Pin_9
#define RST GPIOA,GPIO_Pin_8
#define RS GPIOA,GPIO_Pin_12
#define RW GPIOA,GPIO_Pin_11
#define E GPIOA,GPIO_Pin_10

/*
void txs0108e_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_15;
	GPIO_InitStructure1.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure1);
	GPIO_SetBits(GPIOC,GPIO_Pin_15);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure2;
	GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure2.GPIO_Pin=GPIO_Pin_All;
	GPIO_InitStructure2.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure2);
}
*/


static void LCD_out(uint8_t dat)
{
    GPIOA->ODR = (GPIOA->ODR & 0xFF00) | dat;
}

// 写命令函数
void Write_Command(uint8_t cmd) {
	GPIO_ResetBits(RS);
	GPIO_ResetBits(RW);
	//写
	LCD_out(cmd);
	GPIO_SetBits(E);
	Delay_us(30);
	GPIO_ResetBits(E);
	Delay_us(30);
	/*
    RS = 0;       // 指令模式
    RW = 0;       // 写模式
    P0 = cmd;     // 数据总线赋值
    E = 1;        // 使能信号上升沿
    Delay1ms();
    E = 0;        // 下降沿锁存数据
    Delay1ms();  // 等待指令执行
	*/
}

// 写数据函数
void Write_Data(uint8_t dat) {
	GPIO_SetBits(RS);
	GPIO_ResetBits(RW);
	//XIE
	LCD_out(dat);
	GPIO_SetBits(E);
	Delay_us(5);//5
	GPIO_ResetBits(E);
	Delay_us(6);//6
	
	/*
    RS = 1;       // 数据模式
    RW = 0;       // 写模式
    P0 = dat;
    E = 1;
    Delay1ms();
    E = 0;
    Delay1ms();
	*/
}

void LCD_Init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitStructure1.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_15;
	GPIO_InitStructure1.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure1);
	GPIO_SetBits(GPIOC,GPIO_Pin_15);
	//电平转换器使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure2;
	GPIO_InitStructure2.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure2.GPIO_Pin=GPIO_Pin_All;
	GPIO_InitStructure2.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure2);
	GPIO_SetBits(PSB);
	GPIO_ResetBits(RST);
	Delay_ms(2);
	GPIO_SetBits(RST);
	Delay_ms(4);
	Write_Command(0x30);   // 基本指令集
  Delay_ms(1);
  Write_Command(0x0C);   // 显示开，无光标
  Delay_ms(1);
  Write_Command(0x01);   // 清屏
  Delay_ms(10);
}


void Set_Graphic_Mode() {
	Write_Command(0x34); // 进入扩展指令集
	Delay_ms(1);
  Write_Command(0x36); // 开启图形显示
	Delay_ms(10);
}

void Clear_Screen() {
    Write_Command(0x01); // 清屏指令
    Delay_ms(10);        // 必须等待10ms以上
}

void Clear_GDRAM(void)
{
  uint8_t i, x;
	for(x=0; x<32; x++)
	{
    for(i=0; i<8; i++) { // 16行
        Write_Command(0x80 + (x)); // 设置垂直地址
        Write_Command(0x80 + (i)); // 设置水平地址
        Write_Data(0x00);               // 高 8 位
        Write_Data(0x00);
    }
	}
	for(x=0; x<32; x++)
	{
    for(i=8; i<16; i++) { // 16行
        Write_Command(0x80 + (x)); // 设置垂直地址
        Write_Command(0x80 + (i)); // 设置水平地址
        Write_Data(0x00);               // 高 8 位
        Write_Data(0x00);
    }
	}
}

void Fill_GDRAM(void)
{
  uint8_t i, x;
	for(x=0; x<32; x++)
	{
    for(i=0; i<8; i++) { // 16行
        Write_Command(0x80 + (x)); // 设置垂直地址
        Write_Command(0x80 + (i)); // 设置水平地址
        Write_Data(0xFF);               // 高 8 位
        Write_Data(0xFF);
    }
	}
	for(x=0; x<32; x++)
	{
    for(i=8; i<16; i++) { // 16行
        Write_Command(0x80 + (x)); // 设置垂直地址
        Write_Command(0x80 + (i)); // 设置水平地址
        Write_Data(0xFF);               // 高 8 位
        Write_Data(0xFF);
    }
	}
}

void Display_Chinese(uint8_t x, uint8_t y, uint8_t const *dat) {
    uint8_t i, j;
    for(i=0; i<16; i++) { // 16行
        Write_Command(0x80 + (y + i)); // 设置垂直地址
        Write_Command(0x80 + (x / 16)); // 设置水平地址
        
        // 写入两个字节（每个地址对应16像素）
        for(j=0; j<2; j++) {
            Write_Data(dat[i*2 + j]);
        }
    }
}
