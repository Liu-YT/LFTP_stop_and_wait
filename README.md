# LFTP

* [LFTP基于流水线实现](https://github.com/Liu-YT/LFTP_pipeline)



## 介绍

LFTP支持传输两台电脑之间传输大文件



## 特点

可靠传输 - **停等实现**



## 运行命令

* 服务端
    ```shell
    g++ server.cpp main.cpp -o lftp -lwsock32   ## 编译
    lftp.exe                                    ## 启动服务器
    ```
* 客户端
    * 说明
        ```shell
        g++ client.cpp main.cpp -o lftp -lwsock32   ## 编译
        lftp.exe lget server filename               ## 使用legt
        lftp.exe lsend server filename              ## 使用lsend
        ```
    * 例子
        ```shell
        lftp.exe lget 127.0.0.1:8888 server.txt
        lftp.exe lsend 127.0.0.1:8888 ../data/client.txt
        ```





