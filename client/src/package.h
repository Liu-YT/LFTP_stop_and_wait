#ifndef PACKAGE_H
#define PACKAGE_H

#define BUFFER_SIZE 1024

typedef struct
{
    bool ACK;               // ACK标志
    int ack;                // 确认字段的值
    bool FIN;               // 结束标志
    int winSize;            // 拥塞控制
    int rwnd;               // 流量控制
    char data[BUFFER_SIZE]; // 传输数据
} UDP_PACK;

#endif