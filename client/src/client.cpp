#include <stdio.h>
#include <winsock2.h>
#include "client.h"
#include "package.h"
#include <fstream>
#include <ctime>

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

    string fileName = file.substr(file.find_last_of('/')+1);
    // cout << fileName << endl;

    /* 发送文件名 */
    if (sendto(cltSocket, fileName.c_str(), strlen(fileName.c_str()), 0, (sockaddr *)&serAddr, addrLen) < 0)
    {
        cout << "Send File Name: " << fileName << " Failed" << endl;
        ::closesocket(cltSocket);
        ::WSACleanup();
        cout << "Socket closed..." << endl;
        exit(1);
    }

    /* 打开文件，准备写入 */
    ifstream readFile(file.c_str(), ios::in | ios::binary); //二进制读方式打开
    if (NULL == readFile)
    {
        cout << "File: " << file << " Can Not Open To Write" << endl;
        ::closesocket(cltSocket);
        ::WSACleanup();
        cout << "Socket closed..." << endl;
        exit(2);
    }

    /* 从服务器接收数据，并写入文件 */
    int sendAck = 0;
    int recAck = 0;
    UDP_PACK pack_info;
    while (true)
    {
        try
        {
            if (recAck == sendAck)
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

                cout << pack_info.ack << " " <<  pack_info.bufferSize << " " << pack_info.FIN << endl;

                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0)
                {
                    perror("Send File Failed: ");
                    break;
                }
                /* 接收确认消息 */
                UDP_PACK rcv;
                if(recvfrom(cltSocket, (char *)&rcv, sizeof(rcv), 0, (sockaddr *)&serAddr, &addrLen) < 0) {
                    perror("Send File Failed: ");
                    break;
                }
                ++sendAck;
                recAck = rcv.ack;
                if(rcv.FIN) {
                    ::closesocket(cltSocket);
                    ::WSACleanup();
                    cout << "Transfer successfully" << endl;
                    cout << "Socket closed..." << endl;
                    exit(0);
                }
            }
            else
            {
                /* 如果接收的id和发送的id不相同,重新发送 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0)
                {
                    perror("Send File Failed:");
                    break;
                }
                /* 接收确认消息 */
                UDP_PACK rcv;
                recvfrom(cltSocket, (char *)&rcv, sizeof(rcv), 0, (sockaddr *)&serAddr, &addrLen);
                recAck = rcv.ack;

                if (rcv.FIN && recAck == sendAck)
                {
                    cout << "Transfer successfully" << endl;
                    ::closesocket(cltSocket);
                    ::WSACleanup();
                    cout << "Socket closed..." << endl;
                    exit(0);
                }
            }
        }
        catch (exception &err)
        {
            cerr << err.what() << endl;
            exit(1);
        }
    }
}

void Client::lget() {

    clock_t start, end;

    /* 发送文件名 */
    if (sendto(cltSocket, file.c_str(), strlen(file.c_str()), 0, (sockaddr *)&serAddr, addrLen) < 0) {
        cout << "Send File Name: " << file <<  " Failed" << endl;
        ::closesocket(cltSocket);
        ::WSACleanup();
        cout << "Socket closed..." << endl;
        exit(1);
    }

    /* 打开文件，准备写入 */
    string filePath = "../data/" + file;
    // cout << filePath << endl;
    ofstream writerFile(filePath.c_str(), ios::out | ios::binary);
    if (NULL == writerFile)
    {
        cout << "File: " << filePath << " Can Not Open To Write" << endl;
        ::closesocket(cltSocket);
        ::WSACleanup();
        cout << "Socket closed..." << endl;
        exit(2);
    }

    /* 从服务器接收数据，并写入文件 */
    int sendAck = 0;
    int recAck = 0;
    UDP_PACK pack_info;
    while (true) {

        end = clock(); //程序结束用时
        double endtime = (double)(end - start) / CLOCKS_PER_SEC;
        if (endtime >= 5.0)
        {
            // 超时重传
            sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen);
        }

        if (recvfrom(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, &addrLen) > 0) {
            cout << "ack: " << pack_info.ack << endl;
            recAck = pack_info.ack;
            if(recAck == -1) {
                cout << "Server no such a file: " + file << endl;
                ::closesocket(cltSocket);
                ::WSACleanup();
                cout << "Socket closed..." << endl;
                exit(0);
            }
            if (sendAck == recAck) {
                sendAck++;
                pack_info.ack = sendAck;

                /* 发送数据包确认信息 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0) {
                    cout << "Send confirm information failed!" << endl;
                }

                start = clock(); //程序开始计时

                writerFile.write((char *)&pack_info.data, pack_info.bufferSize);
                if(pack_info.FIN) {
                    writerFile.close();
                    ::closesocket(cltSocket);
                    ::WSACleanup();
                    cout << "Receive File: " << file << " From Server " << serverIp << " Successful!" << endl;
                    exit(0);
                }
            }
            else  {
                /* 如果是重发的包或者发生部分丢包 */
                pack_info.ack = sendAck;
                /* 重发数据包确认信息 */
                if (sendto(cltSocket, (char *)&pack_info, sizeof(pack_info), 0, (sockaddr *)&serAddr, addrLen) < 0) {
                    cout << "Send confirm information failed!" << endl;
                }
            }
        }
    }
    ::closesocket(cltSocket);
    ::WSACleanup();
    cout << "Socket closed..." << endl;
}