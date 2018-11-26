#include <stdio.h>
#include <winsock2.h>
#include "client.h"
#include "package.h"

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
    string filename = file;
    int BUFFER_SIZE = 10;
    string buffer = filename;    

    /* 发送文件名 */
    string name = filename.substr(filename.find_last_of('/'));
    const char *data = filename.data();
    cout << buffer.c_str() << endl;
    if (sendto(cltSocket, data, strlen(data), 0, (sockaddr *)&serAddr, addrLen) < 0) {
        cout << "Send File Name: " << filename <<  " Failed" << endl;
        exit(1);
    }
    /* 打开文件，准备写入 */
    FILE *fp = fopen(file.c_str(), "w");
    if (NULL == fp) {
        printf("File:\t%s Can Not Open To Write\n", filename.c_str());
        exit(1);
    }

    /* 从服务器接收数据，并写入文件 */
    int id = 0;
    int len = 0;
    while (1) {
        Package pack_info;
        Package data;
        if ((len = recvfrom(cltSocket, (char *)&data, sizeof(data), 0, (struct sockaddr *)&serAddr, &addrLen)) > 0) {
            if (data.ack== id) {
                ++id;
                /* 发送数据包确认信息 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (struct sockaddr *)&serAddr, addrLen) < 0)
                {
                    printf("Send confirm information failed!");
                }
                /* 写入文件 */
                if (fwrite(data.data, sizeof(char), 100, fp) < 100)
                {
                    // printf("File:\t%s Write Failed\n", filename);
                    cout << "file: " << filename << " write failed!" << endl;
                    break;
                }
            }
            else if (data.ack < id) /* 如果是重发的包 */
            {
                pack_info.ack = data.ack;
                /* 重发数据包确认信息 */
                if (sendto(cltSocket, (char *)&data, sizeof(data), 0, (struct sockaddr *)&serAddr, addrLen) < 0)
                {
                    printf("Send confirm information failed!");
                }
            }
        }
        else    break;
    }

    cout << "Receive File:\t" + filename + " From Server IP Successful!" << endl;
    fclose(fp);
}

// int test(int argc, char *argv[]) {
//     WORD socketVersion = MAKEWORD(2, 2);
//     WSADATA wsaData;
//     if (WSAStartup(socketVersion, &wsaData) != 0)
//         return 0;
        
//     SOCKET sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

//     sockaddr_in sin;
//     sin.sin_family = AF_INET;
//     sin.sin_port = htons(8888);
//     sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
//     int len = sizeof(sin);

//     const char *sendData = "asaaaaaaaaaaaaaaak\n";
//     sendto(sclient, sendData, strlen(sendData), 0, (sockaddr *)&sin, len);

//     char recvData[255];
//     int ret = recvfrom(sclient, recvData, 255, 0, (sockaddr *)&sin, &len);
//     if (ret > 0) {
//         recvData[ret] = 0x00;
//         printf(recvData);
//     }

//     closesocket(sclient);
//     WSACleanup();
//     return 0;
// }