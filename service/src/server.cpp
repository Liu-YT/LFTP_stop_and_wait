#include <exception>
#include <iostream>
#include <WS2tcpip.h>
#include "server.h"
#include "package.h"
#include <fstream>

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

   
}

void Server::dealGet() {
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
    string realFileName = "../data/" + string(fileName);

    /* 打开文件 */
    ifstream readFile(realFileName.c_str(), ios::in | ios::binary); //二进制读方式打开

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
                    readFile.close();
                }
                else
                {
                    pack_info.FIN = true;
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
    readFile.close();
    printf("File:%s Transfer Successful!\n", fileName);
}

inline DWORD WINAPI CreateClientThread(LPVOID lpParameter)
{
    cout << "CreateClientThread" << endl;
}