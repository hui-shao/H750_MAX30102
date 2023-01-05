#ifndef __OLED_I2C_H__
#define __OLED_I2C_H__

#include "main.h"
#include "i2c.h"
#include "algorithm_by_RF.h"

#define OLED_ADDRESS 0x78 
#define OLED_I2C_HANDLE (hi2c2)

void I2C_WriteByte(uint8_t mem_addr, uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char* src_string, unsigned char index);
void OLED_DrawBMP(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char BMP[]);
void OLED_DrawWave(uint8_t x, uint8_t y);
void OLED_DrawPoint(uint8_t x, uint8_t y);
void Before_State_Update(uint8_t y);

void OLED_Customized_Show(calculated_data_t *data, uint8_t screen);

#endif
