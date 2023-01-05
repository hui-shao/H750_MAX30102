/************************************************************************************
 *  Copyright (c), 2014, HelTec Automatic Technology co.,LTD.
 *            All rights reserved.
 *
 * Http:    www.heltec.cn
 * Email:   cn.heltec@gmail.com
 * WebShop: heltec.taobao.com
 *
 * File name: OLED_I2C.c
 * Project  : HelTec.uvprij
 * Processor: STM32F103C8T6
 * Compiler : MDK fo ARM
 *
 * Author : 小林
 * Version: 1.01
 * Date   : 2014.4.8
 * Email  : hello14blog@gmail.com
 * Modification: 改造为HAL版本 by Hui-Shao 2022.04.05
 *
 * Description:128*64点阵的OLED显示屏驱动文件，仅适用于惠特自动化(heltec.taobao.com)的SD1306驱动IIC通信方式显示屏
 *
 * Others: none;
 *
 * Function List:
 *	1. void I2C_Configuration(void) -- 配置CPU的硬件I2C
 * 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
 * 3. void WriteCmd(unsigned char I2C_Command) -- 写命令
 * 4. void WriteDat(unsigned char I2C_Data) -- 写数据
 * 5. void OLED_Init(void) -- OLED屏初始化
 * 6. void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
 * 7. void OLED_Fill(unsigned char fill_Data) -- 全屏填充
 * 8. void OLED_CLS(void) -- 清屏
 * 9. void OLED_ON(void) -- 唤醒
 * 10. void OLED_OFF(void) -- 睡眠
 * 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
 * 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char index) -- 显示中文(中文需要先取模，然后放到codetab.h中)
 * 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
 *
 * History: none;
 *
 *************************************************************************************/
#include "oled_i2c.h"
#include "codetab.h"
#include "stdio.h"
#include "string.h"

uint8_t Bef[3]; // 保存前一个数据的几个参数1.要写在第几页2.0x01要移动几位3.写什么数据
uint8_t Cur[3]; // 当前前一个数据1.要写在第几页2.0x01要移动几位3.写什么数据
// const float accur = 0.015295f; // accur=18*3.3/4096（3.3/4096就是ADC采样精度，18是为了让波形转化一下能够显示在适当位子）
// uint8_t DataBuffer[8][128];
void I2C_WriteByte(uint8_t mem_addr, uint8_t data)
{
	uint8_t *pData;
	pData = &data;
	HAL_I2C_Mem_Write(&OLED_I2C_HANDLE, OLED_ADDRESS, mem_addr, I2C_MEMADD_SIZE_8BIT, pData, 1, 100);
}

void WriteCmd(unsigned char I2C_Command) // 写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data) // 写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

void OLED_Init(void)
{
	HAL_Delay(100); // 这里的延时很重要

	WriteCmd(0xAE); // display off
	WriteCmd(0x20); // Set Memory Addressing Mode
	WriteCmd(0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0); // Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8); // Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); // 亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); // 0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	OLED_CLS();
}

void OLED_SetPos(unsigned char x, unsigned char y) // 设置起始点坐标
{
	WriteCmd(0xb0 + y);
	WriteCmd(((x & 0xf0) >> 4) | 0x10);
	WriteCmd((x & 0x0f) | 0x01);
}

/*波形显示专用*/
/*与上面的Setpos，x、y是相反的*/
void OLED_Set(unsigned char x, unsigned char y) // 设置起始点坐标
{
	WriteCmd(0xb0 + x);
	WriteCmd((y & 0x0f) | 0x00);		// LOW
	WriteCmd(((y & 0xf0) >> 4) | 0x10); // HIGHT
}

void OLED_Fill(unsigned char fill_Data) // 全屏填充
{
	unsigned char m, n;
	for (m = 0; m < 8; m++)
	{
		WriteCmd(0xb0 + m); // page0-page1
		WriteCmd(0x00);		// low column start address
		WriteCmd(0x10);		// high column start address
		for (n = 0; n < 128; n++)
		{
			WriteDat(fill_Data);
		}
	}
}

void OLED_CLS(void) // 清屏
{
	OLED_Fill(0x00);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D); // 设置电荷泵
	WriteCmd(0X14); // 开启电荷泵
	WriteCmd(0XAF); // OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D); // 设置电荷泵
	WriteCmd(0X10); // 关闭电荷泵
	WriteCmd(0XAE); // OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowChar(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0, i = 0, j = 0;
	switch (TextSize)
	{
	case 1:
	{
		while (ch[j] != '\0')
		{
			c = ch[j] - 32;
			if (x > 126)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x, y);
			for (i = 0; i < 6; i++)
				WriteDat(F6x8[c][i]);
			x += 6;
			j++;
		}
	}
	break;
	case 2:
	{
		while (ch[j] != '\0')
		{
			c = ch[j] - 32;
			if (x > 120)
			{
				x = 0;
				y++;
			}
			OLED_SetPos(x, y);
			for (i = 0; i < 8; i++)
				WriteDat(F8X16[c * 16 + i]);
			OLED_SetPos(x, y + 1);
			for (i = 0; i < 8; i++)
				WriteDat(F8X16[c * 16 + i + 8]);
			x += 8;
			j++;
		}
	}
	break;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char index)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); index:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char *src_string, unsigned char index)
{
	unsigned char wm = 0;
	unsigned int adder = 32 * index;
	OLED_SetPos(x, y);
	for (wm = 0; wm < 16; wm++)
	{
		WriteDat(src_string[adder]);
		adder += 1;
	}
	OLED_SetPos(x, y + 1);
	for (wm = 0; wm < 16; wm++)
	{
		WriteDat(src_string[adder]);
		adder += 1;
	}
}

//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          :
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
//--------------------------------------------------------------
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

	if (y1 % 8 == 0)
		y = y1 / 8;
	else
		y = y1 / 8 + 1;
	for (y = y0; y < y1; y++)
	{
		OLED_SetPos(x0, y);
		for (x = x0; x < x1; x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}

void Before_State_Update(uint8_t y) // 根据y的值，求出前一个数据的有关参数
{
	Bef[0] = 7 - y / 8;
	Bef[1] = 7 - y % 8;
	Bef[2] = 1 << Bef[1];
}
void Current_State_Update(uint8_t y) // 根据Y值，求出当前数据的有关参数
{
	Cur[0] = 7 - y / 8;	  // 数据写在第几页
	Cur[1] = 7 - y % 8;	  // 0x01要移动的位数
	Cur[2] = 1 << Cur[1]; // 要写什么数据
}

// 显示打点
/*0<=x<=127,0<=y<=63屏幕看作一个坐标轴，左下角是原点*/
void OLED_DrawPoint(uint8_t x, uint8_t y) /*这里x是横坐标，y是纵坐标，在（x,y）处画一个点*/
{
	if (x > 127 || y > 63)
		return;
	uint8_t page, move, data;

	page = 7 - y / 8;	 // 数据写在第几页
	move = 7 - y % 8;	 // 0x01要移动的位数
	data = 0x01 << move; // 要写什么数据
	OLED_Set(page, x);
	WriteDat(data);
}

// 显示波形
void OLED_DrawWave(uint8_t x, uint8_t y)
{
	int8_t page_sub;
	uint8_t page_buff, i, j;
	Current_State_Update(y);	// 根据Y值，求出当前数据的有关参数
	page_sub = Bef[0] - Cur[0]; // 当前值与前一个值的页数相比较
	// 确定当前列，每一页应该写什么数据
	if (page_sub > 0)
	{
		page_buff = Bef[0];
		OLED_Set(page_buff, x);
		WriteDat(Bef[2] - 0x01);
		page_buff--;
		for (i = 0; i < page_sub - 1; i++)
		{
			OLED_Set(page_buff, x);
			WriteDat(0xff);
			page_buff--;
		}
		OLED_Set(page_buff, x);
		WriteDat(0xff << Cur[1]);
	}
	else if (page_sub == 0)
	{
		if (Cur[1] == Bef[1])
		{
			OLED_Set(Cur[0], x);
			WriteDat(Cur[2]);
		}
		else if (Cur[1] > Bef[1])
		{
			OLED_Set(Cur[0], x);
			WriteDat((Cur[2] - Bef[2]) | Cur[2]);
		}
		else if (Cur[1] < Bef[1])
		{
			OLED_Set(Cur[0], x);
			WriteDat(Bef[2] - Cur[2]);
		}
	}
	else if (page_sub < 0)
	{
		page_buff = Cur[0];
		OLED_Set(page_buff, x);
		WriteDat((Cur[2] << 1) - 0x01);
		page_buff--;
		for (i = 0; i < 0 - page_sub - 1; i++)
		{
			OLED_Set(page_buff, x);
			WriteDat(0xff);
			page_buff--;
		}
		OLED_Set(page_buff, x);
		WriteDat(0xff << (Bef[1] + 1));
	}
	Before_State_Update(y);
	// 把下一列，每一页的数据清除掉
	for (i = 0; i < 8; i++)
	{
		OLED_Set(i, x + 1);
		for (j = 0; j < 1; j++)
			WriteDat(0x00);
	}
}

void OLED_Customized_Show(calculated_data_t *data, uint8_t screen)
{
	char text[16];

	switch (screen)
	{
	case 1:
		OLED_ShowStr(1, 0, (unsigned char *)"SpO2: --.---%", 2);  // 8*16字符
		OLED_ShowStr(1, 2, (unsigned char *)"HeartRate: ---", 2); // 8*16字符
		break;
	case 2:
		snprintf(text, 16, "SpO2: %4.3f%%", data->spo2);
		OLED_ShowStr(1, 0, (unsigned char *)text, 2); // 8*16字符

		snprintf(text, 16, "HeartRate: %-3d", data->heart_rate);
		OLED_ShowStr(1, 2, (unsigned char *)text, 2); // 8*16字符
		break;
	default:
		break;
	}
	snprintf(text, 16, "Ratio: %2.2f", data->ratio);
	OLED_ShowStr(1, 4, (unsigned char *)text, 2); // 8*16字符

	snprintf(text, 16, "Correl: %2.2f", data->correl);
	OLED_ShowStr(1, 6, (unsigned char *)text, 2); // 8*16字符
}
