
#include <iostream>
#include <WS2tcpip.h>
#include "server.h"
using namespace std;

#define SERVER_PORT 8888
#define MSG_BUG_SIZE 255


Server::Server() {

}

Server::~Server() {

}

void Server::waitForClient() {

}

void DWORD WINAPI CreateClientThread(LPVOID lpParameter){

}

// int main(int argc, char *argv[]) {
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