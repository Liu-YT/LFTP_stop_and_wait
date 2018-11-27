#include <stdio.h>
#include <winsock2.h>
#include "client.h"
#include "package.h"
#include <fstream>
#pragma comment(lib, "ws2_32.lib")

#define MSG_BUG_SIZE 255

Client::Client(string _ip, string _file, int _port) {

    cout << "Initializing client...\n";
    this->serverIp = _ip;
    this->file = _file;
    this->serPort = _port;

    this->sockVersion = MAKEWORD(2, 2);
    if (WSAStartup(sockVersion, &wsaData) != 0)
        exit(1);

    this->cltSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (cltSocket == INVALID_SOCKET) {
        cerr << "socket create error!" << endl;
        exit(2);
    }

    this->serAddr.sin_family = AF_INET;;
    this->serAddr.sin_port = htons(this->serPort);
    this->serAddr.sin_addr.S_un.S_addr = inet_addr(this->serverIp.c_str());
    

    this->addrLen = sizeof(serAddr);
    cout << "Server socket created successfully..." << endl;
}

Client::~Client() {
    ::closesocket(cltSocket);
    ::WSACleanup();
    cout << "Socket closed..." << endl;
}

void Client::lsend() {

    // cout << "lftp lsend " << serverIp << " " << dataDir << "/" << filename << endl;

    // const char *sendData = "asaaaaaaaaaaaaaaak\n";
    // sendto(cltSocket, sendData, strlen(sendData), 0, (sockaddr *)&serAddr, addrLen);

    // char recvData[255];
    // int ret = recvfrom(cltSocket, recvData, 255, 0, (sockaddr *)&serAddr, &addrLen);
    // if (ret > 0) {
    //     recvData[ret] = 0x00;
    //     printf(recvData);
    // }
}

void Client::lget() {
    
    /* 发送文件名 */
    if (sendto(cltSocket, file.c_str(), strlen(file.c_str()), 0, (sockaddr *)&serAddr, addrLen) < 0) {
        cout << "Send File Name: " << file <<  " Failed" << endl;
        exit(1);
    }

    /* 打开文件，准备写入 */
    string filePath = "../data/" + file;
    // cout << filePath << endl;
    ofstream writerFile(filePath.c_str(), ios::out | ios::binary);
    if (NULL == writerFile)
    {
        cout << "File: " << filePath << " Can Not Open To Write" << endl;
        exit(2);
    }

    /* 从服务器接收数据，并写入文件 */
    int sendAck = 0;
    int recAck = 0;
    UDP_PACK pack_info;
    while (true) {
        if (recvfrom(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, &addrLen) > 0) {
            cout << "ack: " << pack_info.ack << endl;
            recAck = pack_info.ack;
            if (sendAck == recAck) {
                sendAck++;
                pack_info.ack = sendAck;

                /* 发送数据包确认信息 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0) {
                    printf("Send confirm information failed!");
                }
                writerFile.write((char *)&pack_info.data, pack_info.bufferSize);
                if(pack_info.FIN) {
                    writerFile.close();
                    ::closesocket(cltSocket);
                    ::WSACleanup();
                    cout << "Receive File:\t" << file << " From Server IP Successful!" << endl;
                    writerFile.close();
                    exit(0);
                }
            }
            else  {
                /* 如果是重发的包或者发生部分丢包 */
                pack_info.ack = sendAck;
                /* 重发数据包确认信息 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0) {
                    printf("Send confirm information failed!");
                }
            }
        }
        else {
            cerr << "Have a error!" << endl;
            exit(3);
        }
    }
}