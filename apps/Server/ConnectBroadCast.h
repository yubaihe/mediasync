#ifndef RELECH_CONNECTBROADCAST_H__
#define RELECH_CONNECTBROADCAST_H__
#include "stdafx.h"
#include "RelechQueue.h"
#undef MAX_DATALEN
#define MAX_DATALEN  65535
struct ConnectBroadCast;

typedef void(*OnConnectBroadCastMessageFun)(struct ConnectBroadCast* pConnectBroadCast, BaseMsg* pMsg, SOCKET iSocket);

struct ConnectBroadCast
{
    SOCKET iSocketFd;
    CRITICAL_SECTION SocketSection;
    CRITICAL_SECTION MsgQueueSection;
    struct RelechQueue* pMsgQueue;
    HANDLE hRecvProcHandle;
    HANDLE hMessageHandle;
    OnConnectBroadCastMessageFun OnMessageProc;
};



struct ConnectBroadCast* ConnectBroadCast_Start(int iPort);
void ConnectBroadCast_Stop(struct ConnectBroadCast* pConnectBroadCast);
BOOL ConnectBroadCast_SendMessage(struct ConnectBroadCast* pConnectBroadCast, const char* pSendBuff, DWORD dwLen, struct sockaddr_in* pSocket);
char* ConnectBroadCast_GetBroadCastIps(void);
char* ConnectBroadCast_GetLocalMacs(void);

char* ConnectBroadCast_BroadCastOne(const char* pSendBuff, DWORD dwLen, int iPort, int iTimeOut, char* pRetIp, int* piRetPort);
void ConnectBroadCast_Close(SOCKET iSocket);

void ConnectBroadCast_BroadCastNotifyOnLine(struct ConnectBroadCast* pConnectBroadCast, int iPort);

#endif
