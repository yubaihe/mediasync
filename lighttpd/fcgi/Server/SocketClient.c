//
//  SocketClient.c
//  MediaSyncNetWork
//
//  Created by 俞力奇 on 2020/3/14.
//  Copyright © 2020 俞力奇. All rights reserved.
//

#include "SocketClient.h"

BOOL SocketClient_SetBlockOpt(SOCKET socketFd, BOOL bBlocked)
{
    int  iFlags;
    iFlags = fcntl(socketFd, F_GETFL, 0);
    if(iFlags < 0)
    {
        return FALSE;
    }
    if(bBlocked)
    {
        iFlags &= ~O_NONBLOCK;
    }
    else
    {
        iFlags |= O_NONBLOCK;
    }
    if(fcntl(socketFd, F_SETFL, iFlags) < 0)
    {
        return FALSE;
    }
    
    return TRUE;
}

void SocketClient_SignalHandler(int nSigno)
{
    printf("SignalHandler\n");
}

SOCKET SocketClient_OpenClient(const char* pIpAddr, int iPort)
{
    signal(SIGPIPE,  &SocketClient_SignalHandler);
    SOCKET iSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    int iValue = 1;
    setsockopt(iSocketFd, SOL_SOCKET,SO_REUSEADDR, &iValue, sizeof(iValue));
    struct sockaddr_in addrServer;
    addrServer.sin_addr.s_addr = inet_addr(pIpAddr);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(iPort);
    SocketClient_SetBlockOpt(iSocketFd, FALSE);
    int iError = connect((SOCKET)iSocketFd, (struct sockaddr *)&addrServer, sizeof(addrServer));
    if(iError < 0)
    {
        struct timeval tm;
        tm.tv_sec = 10;
        tm.tv_usec = 0;
        fd_set rest, west;
        FD_ZERO(&rest);
        FD_ZERO(&west);
        FD_SET(iSocketFd, &rest);
        FD_SET(iSocketFd, &west);
        if( (iError = select(iSocketFd + 1, &rest, &west, NULL, &tm)) <= 0)
        {
            
            closesocket(iSocketFd);
            iSocketFd = 0;
            return 0;
        }

        //如果套接口及可写也可读，需要进一步判断
        if(FD_ISSET(iSocketFd, &rest) && FD_ISSET(iSocketFd, &west))
        {
            int iError = 0, iLen = 0;
            if(getsockopt(iSocketFd, SOL_SOCKET, SO_ERROR, &iError, (socklen_t *)&iLen) < 0)
            {
                printf("getsockopt error:%d", errno);
                closesocket(iSocketFd);
                iSocketFd = 0;
                return 0;
            }

            /*if(iError != 0)
            {
                closesocket(pSocketClientSync->iSocketFd);
                pSocketClientSync->iSocketFd = 0;
                free(pBuffer);
                pBuffer = NULL;
                return NULL;
            }*/
        }
        //如果套接口可写不可读,则链接完成
        else if(!FD_ISSET(iSocketFd, &west))
        {
            closesocket(iSocketFd);
            iSocketFd = 0;
            return 0;
        }
    }

    SocketClient_SetBlockOpt(iSocketFd, TRUE);
    return iSocketFd;
}

void SocketClient_Close(SOCKET socket)
{
    shutdown(socket, SD_BOTH)
    close(socket);
    socket = 0;
}

BOOL SocketClient_SendFile(long iTag, const char* pIp, int iPort, const char* pFileName, OnProgressCallBack callBack)
{
    FILE* pFile = fopen(pFileName,"rb");
    if(NULL == pFile)
    {
        callBack(iTag, 0, PROGRESS_FAILED);
        return FALSE;
    }
    SOCKET socket = SocketClient_OpenClient(pIp, iPort);
    if(socket == 0)
    {
        fclose(pFile);
        callBack(iTag, 0, PROGRESS_FAILED);
        return FALSE;
    }
    
    int iSendLen = 0;
    char szBuffer[2048] = {0};
    uint64_t iTotalReadLen = 0;
    while(1)
    {
        uint64_t iReadLen = fread(szBuffer, sizeof(char), sizeof(szBuffer), pFile);
        iTotalReadLen += iReadLen;
        //printf("iTotalReadLen: %llu\n", iTotalReadLen);
        if(iReadLen == 0)
        {
            callBack(iTag, iTotalReadLen, PROGRESS_SUCCESS);
            break;
        }
        else
        {
            iSendLen = (int)send((int)socket, szBuffer, (size_t)iReadLen, 0);
            if(iSendLen != iReadLen)
            {
                printf("%s\n", "send failed");
                fclose(pFile);
                SocketClient_Close(socket);
                callBack(iTag, iTotalReadLen, PROGRESS_FAILED);
                return FALSE;
            }
            callBack(iTag, iTotalReadLen, PROGRESS_UPLOADING);
        }
    }
    fclose(pFile);
    //printf("%s\n", "send over");
    char szRecvBuffer[100] = {0};
    int iRecvBuffer = 100;
    int iRecvLen = (int)recv((int)socket, szRecvBuffer, iRecvBuffer, 0);
    //printf("recv len:%d\n", iRecvLen);
    if(iRecvLen <= 0)
    {
        SocketClient_Close(socket);
        callBack(iTag, iTotalReadLen, PROGRESS_FAILED);
        return FALSE;
    }
    
    uint64_t iTotalRecvLen = strtoul(szRecvBuffer, NULL, 10);
    //printf("iTotalRecvLen:%llu iTotalReadLen:%llu\n", iTotalRecvLen, iTotalReadLen);
    if(iTotalRecvLen != iTotalReadLen)
    {
        SocketClient_Close(socket);
        callBack(iTag, iTotalReadLen, PROGRESS_FAILED);
        return FALSE;
    }
    SocketClient_Close(socket);
    return TRUE;
}
////返回值需要手动释放
char* SocketClient_SendMessage(const char* pIp, int iPort, const char* pstrMessage)
{
    SOCKET socket = SocketClient_OpenClient(pIp, iPort);
    if(socket == 0)
    {
        return NULL;
    }
    int iLen = (int)(strlen(pstrMessage) + sizeof(InnerBase));
    char* pSendBuffer = (char*)malloc((size_t)iLen);
    memset((void*)pSendBuffer, 0, iLen);
    InnerBase* pMsg = (InnerBase*)pSendBuffer;
    pMsg->iMsgType = MESSAGETYPE_CMD;
    pMsg->iMsgLen = (int)(strlen(pstrMessage) + 1);

    memcpy(pMsg->szBuf, pstrMessage, strlen(pstrMessage));
    
    int iMsgLen = (int)(sizeof(struct BaseMessage) - 1 + iLen);
    char* pBuffer = malloc(iMsgLen);
    memset(pBuffer, 0, iMsgLen);
    struct BaseMessage* pBaseMsg = (struct BaseMessage*)pBuffer;
    pBaseMsg->iDataLen = (int)iLen;
    memcpy(pBaseMsg->szBuf, pSendBuffer, iLen);
    
    free(pSendBuffer);
    pSendBuffer = NULL;
    
    iLen = (int)send(socket, pBuffer, iMsgLen, 0);
    free(pBuffer);
    pBuffer = NULL;
    if(iLen <= 0)
    {
        SocketClient_Close(socket);
        return NULL;
    }
    char szRecvBuffer[65535] = {0};
    int iBufferLen = 65535;
    iLen = (int)recv(socket, szRecvBuffer ,iBufferLen, 0);
    if(iLen <= 0)
    {
        SocketClient_Close(socket);
        return NULL;
    }
    SocketClient_Close(socket);
    struct InnerBase* pInnerMsg = (struct InnerBase*)((struct BaseMessage*)szRecvBuffer)->szBuf;
    char* pRetBuffer = (char*)malloc((size_t)iLen);
    memset(pRetBuffer, 0, iLen);
    memcpy(pRetBuffer, pInnerMsg->szBuf, pInnerMsg->iMsgLen);
    return pRetBuffer;
}



SOCKET SocketClient_OpenServer(int iPort)
{
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(iPort);

    SOCKET iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(iServerSocket <= 0)
    {
        printf("%s\n", "Create Socket Failed");
        return 0;
    }
    int iValue = 1;
    setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, &iValue, sizeof(iValue));
    int iOpt = 1;
    setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt));

    if(bind(iServerSocket,(const struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)))
    {
        printf("Server Bind Port : Failed %d==>%d", iPort, errno);
        closesocket(iServerSocket);
        iServerSocket = 0;
        return 0;
    }

    if (listen(iServerSocket, 10) )
    {
        printf("Server Listen Failed!");
        closesocket(iServerSocket);
        iServerSocket = -1;
        return 0;
    }
    
    return iServerSocket;
}

