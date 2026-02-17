#include "stm32f10x.h"
#include "animation.h"
#define uartbufsize 2048

extern uint8_t uartbuf[2048];
extern uint8_t endflag;
volatile uint8_t state=0;										//0等待 1已经收到 2传输数据 3+特殊字段处理
volatile uint8_t textsize=0,charcount=0;		//字大小与数量
volatile uint16_t length=0;									//本次字占字节数
/*
开头：0xC9

指令：CA接收字符			CB字大小（特殊）			CC当前歌词进入结束（非必要）		CD特效选择

**一般只需要C9 CA 长度 内容

指令 长度（开启接收，需要放在最后）			√
0xCA 0x01（例，字数）

指令 大小（特殊）默认1-2 64/3-10 32/11-32 16
0xCB 1 16大小 2 32大小 3 64大小

指令(需要放在最后)
0xCC

指令 时长
0xCD  0x01（例如）
*/


void USART1_SendChar(char c) {
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // 等待发送缓冲区空
  USART_SendData(USART1, c);           // 写入发送寄存器
}
 
void USART1_SendString(char *str) {
  while(*str) {
    USART1_SendChar(*str++);           // 逐字符发送
  }
}

void DMA1_Channel5_IRQHandler(void)				//DMA结束，回串口接收中断模式，修改标志位
{
    if(DMA_GetITStatus(DMA1_IT_TC5) != RESET)
    {
      DMA_ClearITPendingBit(DMA1_IT_TC5);							//关DMA
			DMA_Cmd(DMA1_Channel5, DISABLE);
			USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
			
			state=0;
			endflag=1;
			
      USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	//开串口
    }
}


void Setup_DMA_Receive(uint16_t length)						//设置DMA
{
    // 确保DMA已停止
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_InitTypeDef DMA_InitStructure;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)uartbuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = length;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_HT5 | DMA1_FLAG_TE5);
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

void USART1_IRQHandler(void)			//串口接收指令，识别
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		uint8_t rec_byte = USART_ReceiveData(USART1);
		USART1_SendChar (rec_byte);
		switch(state){
			case 0: 
				if(rec_byte==0xC9)
					state=1;
				length=0;		//清理上次
				textsize=0;
				charcount=0;
				break;
			case 1:
				if(rec_byte==0xCA)
					state=2;
				else if (rec_byte==0xCB)
					state=3;
				break;
			case 2:													//0xCA 收到长度，准备接收数据
				charcount=rec_byte;
				if(rec_byte==0)
				{
					state=0;										//无效0字数
					break;
				}
				if(textsize==0)									//默认大小
				{
					if(rec_byte>=11)
						length=32*(charcount);			//16*16像素占用字节
					else if(rec_byte>2)
						length=128*(charcount);			//32*32
					else if(rec_byte>0)
						length=512*(charcount);			//64*64
				}
				else if(textsize==1)				//16大小
				{
					length=32*(charcount);
				}
				else if(textsize==2)				//32大小
				{
					length=128*(charcount);
				}
				else if(textsize==3)				//64大小
				{
					length=512*(charcount);
				}
				else
				{
					state=0;										//无效大小
					break;
				}
				
				USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
				Setup_DMA_Receive(length);		//关闭本中断,打开DMA搬运到uartbuf
				break;
			case 3:													//0xCB 自定义文本大小
				textsize=rec_byte;
				state=1;
			break;
			default:												//还没做 没有判定
				state=1;
		}
	}
}

void Uart1_Init(void) 								//UART1,DMA初始化
{
  //---------------- 1. 时钟配置 -----------------
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1| RCC_APB2Periph_AFIO, ENABLE);
  // 使能GPIOA和USART1时钟（USART1挂载在APB2总线，最高72MHz）
	
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

  //---------------- 2. GPIO初始化 -----------------
  GPIO_InitTypeDef GPIO_InitStruct;
  // TX引脚配置（PB6：复用推挽输出）
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出模式
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  // RX引脚配置（PB7：浮空输入）
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 浮空输入模式
  GPIO_Init(GPIOB, &GPIO_InitStruct);
 
  //---------------- 3. USART参数配置 -----------------
  USART_InitTypeDef USART_InitStruct;
  USART_InitStruct.USART_BaudRate = 115200;       // 波特率设置（常用9600/115200）
  USART_InitStruct.USART_WordLength = USART_WordLength_8b; // 数据位：8位
  USART_InitStruct.USART_StopBits = USART_StopBits_1;      // 停止位：1位
  USART_InitStruct.USART_Parity = USART_Parity_No;         // 校验位：无校验
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // 启用收发模式
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
  USART_Init(USART1, &USART_InitStruct);          // 应用配置
 
  //---------------- 4. 中断配置（可选）-----------------
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // 使能接收中断
  NVIC_InitTypeDef NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;  // 中断通道
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  // 子优先级
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;     // 使能中断
  NVIC_Init(&NVIC_InitStruct);
 
  //---------------- 5. 启动USART -----------------
  USART_Cmd(USART1, ENABLE);                       // 使能USART1
	//---------------- 6. DMA配置 -----------------
	// 使能DMA1时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// 配置DMA中断
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
