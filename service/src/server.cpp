#include <exception>
#include <iostream>
#include <WS2tcpip.h>
#include "server.h"
#include "package.h"
#include <fstream>
#include <ctime>

#define MSG_BUG_SIZE 255

Server::Server(string _dir, int _serPort)
{

    this->dataDir = _dir;
    this->serPort = _serPort;

    cout << "Initializing server...\n";

    //初始化socket资源
    this->sockVersion = MAKEWORD(2, 2);
    if (WSAStartup(sockVersion, &wsaData) != 0)
    {
        exit(1);
    }
    this->addrLen = sizeof(SOCKADDR_IN);

    this->serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serSocket == INVALID_SOCKET)
    {
        cerr << "socket create error!" << endl;
        exit(2);
    }

    //设置传输协议、端口以及目的地址
    this->serAddr.sin_family = AF_INET;
    this->serAddr.sin_port = htons(this->serPort);
    this->serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        cerr << "socket bind error!" << endl;
        closesocket(serSocket);
        exit(3);
    }

    cout << "Server socket created successfully..." << endl;
}

Server::~Server()
{
    ::closesocket(serSocket);
    ::WSACleanup();
    cout << "Socket closed..." << endl;
}

void Server::waitForClient()
{
    dealGet();
    // dealSend();
}

void Server::dealSend()
{

    clock_t start, end;

    /* 接收数据 */
    int FILE_NAME_MAX_SIZE = 100;
    char fileName[FILE_NAME_MAX_SIZE + 1];
    memset(fileName, 0, FILE_NAME_MAX_SIZE);
    if (recvfrom(serSocket, fileName, FILE_NAME_MAX_SIZE, 0, (sockaddr *)&cltAddr, &addrLen) < 0)
    {
        cerr << "Receive Data Failed" << endl;
        exit(1);
    }

    string filePath = "../data/" + string(fileName);

    /* 打开文件，准备写入 */
    // cout << filePath << endl;
    ofstream writerFile(filePath.c_str(), ios::out | ios::binary);
    if (writerFile == NULL)
    {
        cout << "File: " << filePath << " Can Not Open To Write" << endl;
        exit(2);
    }

    // 发送确认包
    UDP_PACK confirm;
    confirm.ack = 0;
    confirm.FIN = false;
    sendto(serSocket, (char *)&confirm, sizeof(confirm), 0, (sockaddr *)&cltAddr, addrLen);

    /* 从服务器接收数据，并写入文件 */
    int sendAck = 0;
    int recAck = 0;
    UDP_PACK pack_info;
    while (true)
    {

        end = clock(); //程序结束用时
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        if(endtime >= 5.0) {
            // 超时重传
            sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen);
        }

        if (recvfrom(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, &addrLen) > 0)
        {
            // cout << "ack: " << pack_info.ack << " " << pack_info.bufferSize << endl;
            recAck = pack_info.ack;
            if (sendAck == recAck)
            {
                sendAck++;
                pack_info.ack = sendAck;

                // 写入信息
                writerFile.write((char *)&pack_info.data, pack_info.bufferSize);

                /* 发送数据包确认信息 */
                if (sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen) < 0)
                {
                    cout << "Send confirm information failed!" << endl;
                }
                start = clock(); //程序开始计时
                if (pack_info.FIN)
                {
                    writerFile.close();
                    ::closesocket(serSocket);
                    ::WSACleanup();
                    cout << "Socket closed..." << endl;
                }
            }
            else
            {
                /* 如果是重发的包或者发生部分丢包 */
                pack_info.ack = sendAck;
                /* 重发数据包确认信息 */
                if (sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen) < 0)
                {
                    cout << "Send confirm information failed!" << endl;
                }
            }
        }
        else
        {
            break;
        }
    }
}

void Server::dealGet()
{
    /* 接收数据 */
    int FILE_NAME_MAX_SIZE = 100;
    char fileName[FILE_NAME_MAX_SIZE + 1];
    memset(fileName, 0, FILE_NAME_MAX_SIZE);
    int ret = recvfrom(serSocket, fileName, FILE_NAME_MAX_SIZE, 0, (sockaddr *)&cltAddr, &addrLen);
    if (ret < 0)
    {
        cerr << "Receive Data Failed" << endl;
        exit(1);
    }

    /* 打开文件 */
    string realFileName = "../data/" + string(fileName);

    ifstream readFile(realFileName.c_str(), ios::in | ios::binary); //二进制读方式打开
    // 没有相应文件
    if (readFile == NULL)
    {
        string err = "No such file: " + string(fileName);
        UDP_PACK confirm;
        confirm.ack = -1;
        confirm.FIN = true;
        sendto(serSocket, (char *)&confirm, sizeof(confirm), 0, (sockaddr *)&cltAddr, addrLen);
        ::closesocket(serSocket);
        ::WSACleanup();
        cout << "Socket closed..." << endl;
        return;
    }

    cout << "start transfer file" << endl;

    int recAck = 0;
    int sendAck = 0;

    /* 每读取一段数据，便将其发给客户端 */
    UDP_PACK pack_info;
    while (true)
    {
        if (recAck == sendAck)
        {
            try
            {
                if (readFile.peek() != EOF)
                {
                    readFile.read((char *)&pack_info.data, sizeof(int) * MSG_BUG_SIZE);
                    pack_info.FIN = false;
                }
                else
                {
                    pack_info.FIN = true;
                    readFile.close();
                }
                pack_info.bufferSize = readFile.gcount();
                /* 发送ack放进包头,用于标记顺序 */
                pack_info.ack = sendAck;
                if (sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen) < 0)
                {
                    perror("Send File Failed: ");
                    break;
                }
                ++sendAck;
                /* 接收确认消息 */
                UDP_PACK rcv;
                recvfrom(serSocket, (char *)&rcv, sizeof(rcv), 0, (sockaddr *)&cltAddr, &addrLen);
                recAck = rcv.ack;
                // 结束
                if (rcv.FIN && recAck == sendAck)
                {
                    ::closesocket(serSocket);
                    ::WSACleanup();
                    cout << "Socket closed..." << endl;
                }
            }
            catch (exception &err)
            {
                cerr << err.what() << endl;
                exit(1);
            }
        }
        else
        {
            /* 如果接收的id和发送的id不相同,重新发送 */
            if (sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen) < 0)
            {
                perror("Send File Failed:");
                break;
            }
            /* 接收确认消息 */
            UDP_PACK rcv;
            recvfrom(serSocket, (char *)&rcv, sizeof(rcv), 0, (sockaddr *)&cltAddr, &addrLen);
            recAck = rcv.ack;
        }
    }
    /* 关闭文件 */
    cout << "File:" << fileName << " Transfer Successful!" << endl;
}

inline DWORD WINAPI CreateClientThread(LPVOID lpParameter)
{
    cout << "CreateClientThread" << endl;
}