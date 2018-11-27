#ifndef PACKAGE_H
#define PACKAGE_H

#define BUFFER_SIZE 255

typedef struct
{
    bool ACK;               // ACK标志
    int ack;                // 确认字段的值
    bool FIN;               // 结束标志
    int winSize;            // 拥塞控制
    int rwnd;               // 流量控制
    int data[BUFFER_SIZE]; // 传输数据
    int bufferSize;
} UDP_PACK;

#endif