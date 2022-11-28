#pragma once
#ifndef FILESENDSERVER_H_
#define FILESENDSERVER_H_
#include "stdafx.h"
#include "RelechMap.h"
// #define  _WINSOCK_DEPRECATED_NO_WARNINGS
// #define _CRT_SECURE_NO_WARNINGS
// #include <stdio.h>
// #include <stdint.h>
// #include <winsock2.h>
// #include <windows.h>
// #include <WS2tcpip.h>
// #pragma comment(lib,"ws2_32.lib")
struct HttpFileClient
{
    BOOL bConnect;
    SOCKET iClientSocket;
    struct HttpFileServer* pServer;
};
struct HttpFileServer
{
    SOCKET iServerSocket;
    BOOL bQuit;
    HANDLE hAccept;
};

struct HttpFileServer* HttpFileServer_Start(int iPort);
void HttpFileServer_Stop(struct HttpFileServer* pServer);
#endif