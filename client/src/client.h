#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <Winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

#pragma comment(lib, "ws2_32.lib")

class Client
{

  public:
    Client(string _ip = "127.0.0.1", string _file = "../data/filename", int _port = 8888);
    ~Client();
    void lsend();
    void lget();

  private:
    string file; // 文件地址
    int serPort;    // 服务端口
    string serverIp;
    WSADATA wsaData;
    WORD sockVersion;
    SOCKET cltSocket;
    SOCKADDR_IN serAddr;
    int addrLen;
};

#endif