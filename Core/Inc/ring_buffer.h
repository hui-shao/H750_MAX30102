#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "main.h"

#define RINGBUFF_LEN (500) // 定义最大接收数 500
#define RINGBUFF_OK 1
#define RINGBUFF_ERR 0

typedef struct
{
    uint16_t Head;
    uint16_t Tail;
    uint16_t Length;
    uint32_t Ring_data[RINGBUFF_LEN];
} RingBuff_t;

void RingBuff_Clear(RingBuff_t *ringBuff);
uint8_t RingBuff_Write(RingBuff_t *ringBuff, uint32_t data);
uint8_t RingBuff_Read(RingBuff_t *ringBuff, uint32_t *rData);
uint8_t RingBuff_CheckFull(RingBuff_t *ringBuff);

#endif /* __RING_BUFFER_H__ */
