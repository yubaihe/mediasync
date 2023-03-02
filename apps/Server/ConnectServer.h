#include "stdafx.h"
#include "RelechMap.h"
#include "RelechQueue.h"
#ifndef RELECH_CONNSERVER_H
#define RELECH_CONNSERVER_H

#define MAX_DATALEN  65535
#pragma pack (1)
struct ConnectConn;
struct ConnectServer;

typedef	void(*OnMessageFun)(struct ConnectServer* pServer, BaseMsg* pMsg, SOCKET iSocket);
typedef	void(*UnContectedConnFun)(struct ConnectServer* pServer, SOCKET iSocket);
typedef	void(*ContectedConnFun)(struct ConnectServer* pServer, SOCKET iSocket);
struct ConnectConn ConnectServer_GetConn(struct ConnectServer* pServer, SOCKET iSocket);
void ConnectServer_UpdateAccessTime(struct ConnectServer* pServer, SOCKET iSocket);
struct ConnectServer
{
	SOCKET iServerSocket;
	HANDLE hServerHandle;
	HANDLE hTimerHandle;
	HANDLE hMessageHandle;
	CRITICAL_SECTION section;
	RelechMap* pConnMap;
	CRITICAL_SECTION MsgQueueSection;
	struct RelechQueue* pMsgQueue;
	CRITICAL_SECTION UnConnectQueueSection;
	struct RelechQueue* pUnConnectQueue;
	OnMessageFun OnMessageProc;
	UnContectedConnFun UnContectedConnProc;
	ContectedConnFun ContectedConnProc;
};

struct ConnectConn
{
	SOCKET iConnSocket;
	int iLastAccessTime;
	HANDLE hConnRecvHandle;
	CRITICAL_SECTION section;
	struct ConnectServer* pServer;
	struct ConnectConn* pNext;
	char* pExtra;
};


struct SocketLeftBuffer
{
	DWORD dwDataLen;
	char* pszBuf;
};

#pragma pack ()

struct ConnectServer* ConnectServer_Start(int iPort);
void ConnectServer_Stop(struct ConnectServer* pServer);
void ConnectServer_CloseConn(struct ConnectConn* pConn);
BOOL ConnectServer_SendMessage(struct ConnectServer* pServer, SOCKET iSocketFd, const char* pSendBuff, DWORD dwLen);



#endif