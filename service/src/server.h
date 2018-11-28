#ifndef SERVER_HPP
#define SERVER_HPP

#include <Winsock2.h>
#include <windows.h>
#include <string>
using namespace std;

#define IP_BUF_SIZE 129

#pragma comment(lib, "ws2_32.lib")


class Server {

public:
    Server(string _dir = "../data/", int port = 8888);
    ~Server();
    void waitForClient();
    void dealGet();
    void dealSend();
private:
    string dataDir; // 文件地址
    int serPort;    // 服务端口    
    WSADATA wsaData;
    WORD sockVersion;
    SOCKET serSocket;
    SOCKADDR_IN cltAddr;
    SOCKADDR_IN serAddr;
    int addrLen;
    char bufIp[IP_BUF_SIZE];
};

#endif