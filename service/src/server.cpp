
#include <iostream>
#include <WS2tcpip.h>
#include "server.h"
#include "package.h"

#define MSG_BUG_SIZE 255


Server::Server(string _dir, int _serPort) {

    this->dataDir = _dir;
    this->serPort = _serPort;

    cout << "Initializing server...\n";

    //初始化socket资源
    this->sockVersion = MAKEWORD(2, 2);
    if (WSAStartup(sockVersion, &wsaData) != 0) {
        exit(1);
    }
    this->addrLen = sizeof(SOCKADDR_IN);

    this->serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serSocket == INVALID_SOCKET) {
        cerr << "socket create error!" << endl;
        exit(2);
    }

    //设置传输协议、端口以及目的地址
    this->serAddr.sin_family = AF_INET;
    this->serAddr.sin_port = htons(this->serPort);
    this->serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR) {
        cerr << "socket bind error!" << endl;
        closesocket(serSocket);
        exit(3);
    }

    cout << "Server socket created successfully..." << endl;
}

Server::~Server() {
    ::closesocket(serSocket);
    ::WSACleanup();
    cout << "Socket closed..." << endl;
}

void Server::waitForClient() {
    // while (true) {
    //     char recvData[MSG_BUG_SIZE];
    //     int ret = recvfrom(serSocket, recvData, 1024, 0, (sockaddr *)&cltAddr, &addrLen);
    //     if (ret > 0) {
    //         recvData[ret] = 0x00;
    //         cout << "Accept:" << inet_ntoa(cltAddr.sin_addr) << endl;
    //         cout << recvData;
    //     }

    //     const char *sendData = "date udp\n";
    //     // sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&remoteAddr, addrLen);
    // }

    // lget demo

    while(true) {

        /* 接收数据 */
        char buffer[MSG_BUG_SIZE];
        int FILE_NAME_MAX_SIZE = 100;
        memset(buffer, 0, MSG_BUG_SIZE);
        int ret = recvfrom(serSocket, buffer, MSG_BUG_SIZE, 0, (sockaddr *)&cltAddr, &addrLen);
        if(ret < 0) {
            cerr << "Receive Data Failed" << endl;
            exit(1);
        }
        buffer[ret] = 0x00;
        /* 从buffer中拷贝出file_name */
        char file_name[FILE_NAME_MAX_SIZE + 1];
        memset(file_name, 0, FILE_NAME_MAX_SIZE);
        strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));

        string realFileName = "../data/" + string(file_name);

        /* 打开文件 */
        FILE *fp = fopen(realFileName.c_str(), "r");
        if (NULL == fp) {
            printf("File:%s Not Found.\n", realFileName.c_str());
        }
        else {
            cout << "start transfer file" << endl;
            int len = 0;
            int receive_id = 0;
            int send_id = 0;
            /* 每读取一段数据，便将其发给客户端 */
            // while (true) {
                UDP_PACK pack_info;
                // const char *sendData = "的说法是p\n";
                // sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&cltAddr, addrLen);
                
                if ((len = fread(pack_info.data, sizeof(char), MSG_BUG_SIZE, fp)) > 0)
                {
                    cout << pack_info.data << endl;
                    pack_info.ack = send_id;   /* 发送id放进包头,用于标记顺序 */
                    if (sendto(serSocket, (char*)&pack_info, sizeof(pack_info), 0, (sockaddr *)&cltAddr, addrLen) < 0) {
                        perror("Send File Failed: ");
                        break;
                    }
                    /* 接收确认消息 */
                    // UDP_PACK rcv;
                    // recvfrom(serSocket, (char *)&rcv, sizeof(pack_info), 0, (struct sockaddr *)&cltAddr, &addrLen);
                    // receive_id = pack_info.ack;
                }

                    // if (receive_id == send_id) {
                    //     ++send_id;
                    //     if ((len = fread(pack_info.data, sizeof(char), MSG_BUG_SIZE, fp)) > 0) {
                    //         cout << pack_info.data << endl;
                    //         pack_info.ack = send_id;   /* 发送id放进包头,用于标记顺序 */
                    //         if (sendto(serSocket, (char *)&pack_info, sizeof(pack_info), 0, (struct sockaddr *)&cltAddr, addrLen) < 0) {
                    //             perror("Send File Failed:");
                    //             break;
                    //         }
                    //         /* 接收确认消息 */
                    //         UDP_PACK rcv;
                    //         recvfrom(serSocket, (char *)&rcv, sizeof(pack_info), 0, (struct sockaddr *)&cltAddr, &addrLen);
                    //         receive_id = pack_info.ack;
                    //     }
                    //     else
                    //     {
                    //         break;
                    //     }
                    // }
                    // else
                    // {
                    //     /* 如果接收的id和发送的id不相同,重新发送 */
                    //     if (sendto(serSocket, (char *)& pack_info, sizeof(pack_info), 0, (struct sockaddr *)&cltAddr, addrLen) < 0)
                    //     {
                    //         perror("Send File Failed:");
                    //         break;
                    //     }
                    //     /* 接收确认消息 */
                    //     UDP_PACK rcv;
                    //     recvfrom(serSocket, (char *)&rcv, sizeof(rcv), 0, (struct sockaddr *)&cltAddr, &addrLen);
                    //     receive_id = pack_info.ack;
                    // }
                    // }
                    /* 关闭文件 */
                    fclose(fp);
                    printf("File:%s Transfer Successful!\n", file_name);
        }
    }
}

inline DWORD WINAPI CreateClientThread(LPVOID lpParameter){
    cout << "CreateClientThread" << endl;
}

// int test()
// {
//     WSADATA wsaData;
//     WORD sockVersion = MAKEWORD(2, 2);
//     if (WSAStartup(sockVersion, &wsaData) != 0)
//         return 0;
    
//     SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if (serSocket == INVALID_SOCKET) {
//         cerr << "socket error!" << endl;
//         return 0;
//     }

//     sockaddr_in serAddr;
//     serAddr.sin_family = AF_INET;
//     serAddr.sin_port = htons(8888);
//     serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
//     if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR) {
//         cerr << "bind error!" << endl;
//         closesocket(serSocket);
//         return 0;
//     }

//     sockaddr_in remoteAddr;
//     int nAddrLen = sizeof(remoteAddr);
//     while (true) {
//         char recvData[255];
//         int ret = recvfrom(serSocket, recvData, 255, 0, (sockaddr *)&remoteAddr, &nAddrLen);
//         if (ret > 0) {
//             recvData[ret] = 0x00;
//             printf("Accept：%s \r\n", inet_ntoa(remoteAddr.sin_addr));
//             printf(recvData);
//         }

//         const char *sendData = "date udp\n";
//         sendto(serSocket, sendData, strlen(sendData), 0, (sockaddr *)&remoteAddr, nAddrLen);
//     }
//     closesocket(serSocket);
//     WSACleanup();
//     return 0;
// }