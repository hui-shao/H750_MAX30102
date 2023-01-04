#include "ring_buffer.h"
#include <string.h>

/**
 * @description: 初始化缓冲区参数
 * @param {RingBuff_t} *ringBuff 缓冲区指针
 * @return {*}
 */
void RingBuff_Clear(RingBuff_t *ringBuff)
{
    // 初始化相关信息
    ringBuff->Head = 0;
    ringBuff->Tail = 0;
    ringBuff->Length = 0;
    memset(ringBuff, 0, sizeof(uint32_t) * RINGBUFF_LEN);
}

/**
 * @description:
 * @param {RingBuff_t} *ringBuff 缓冲区指针
 * @param {uint32_t} data 待写入的数据
 * @return {*} 成功则返回1
 */
uint8_t RingBuff_Write(RingBuff_t *ringBuff, uint32_t data)
{
    if (ringBuff->Length == RINGBUFF_LEN) // 判断缓冲区是否已满
    {
        return RINGBUFF_ERR;
    }
    ringBuff->Ring_data[ringBuff->Tail] = data;
    ringBuff->Tail = (ringBuff->Tail + 1) % RINGBUFF_LEN; // 防止越界非法访问
    ringBuff->Length++;
    return RINGBUFF_OK;
}

/**
 * @description:
 * @param {RingBuff_t} *ringBuff 缓冲区指针
 * @param {uint32_t} *rData 用于存放读出的数据的变量地址
 * @return {*}
 */
uint8_t RingBuff_Read(RingBuff_t *ringBuff, uint32_t *rData)
{
    if (ringBuff->Length == 0) // 判断非空
    {
        return RINGBUFF_ERR;
    }
    *rData = ringBuff->Ring_data[ringBuff->Head];         // 先进先出FIFO，从缓冲区头出
    ringBuff->Head = (ringBuff->Head + 1) % RINGBUFF_LEN; // 防止越界非法访问
    ringBuff->Length--;
    return RINGBUFF_OK;
}

/**
 * @description: 检查环形缓冲区是否已满
 * @param {RingBuff_t} *ringBuff 缓冲区指针
 * @return {*} 满则返回1
 */
uint8_t RingBuff_CheckFull(RingBuff_t *ringBuff)
{
    return (ringBuff->Length == RINGBUFF_LEN);
}
