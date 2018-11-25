#ifndef SERVER_HPP
#define SERVER_HPP

#include <Winsock2.h>
#include <windows.h>

#define IP_BUF_SIZE 129

#pragma comment(lib, "ws2_32.lib")


class Server {

public:
    Server();
    ~Server();
    void waitForClient();

private:
    WSADATA wsaData;
    WORD sockVersion;
    SOCKET sockSvr;
    SOCKET sockClt;
    HANDLE hThread;
    SOCKADDR_IN addrSvr;
    SOCKADDR_IN addrClt;
    char bufIp[IP_BUF_SIZE];
};

#endif