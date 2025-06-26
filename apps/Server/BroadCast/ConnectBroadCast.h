#pragma once
#include "stdafx.h"
#include "MessageDefine.h"
typedef void(*OnConnectBroadCastMessageFun)(BaseMsg* pMsg, SOCKET iSocket, LPVOID param);
class CConnectBroadCast
{
public:
    CConnectBroadCast();
    ~CConnectBroadCast();
    BOOL Start(int iPort, OnConnectBroadCastMessageFun callBack, LPVOID param);
    void Stop();
    BOOL SendMessage(const char* pszSendBuff, DWORD dwLen, struct sockaddr_in* pSocket);
    std::list<std::string> GetBroadCastIps(void);
    std::list<std::string> GetLocalMacs(void);
    std::string BroadCastOne(const char* pszSendBuff, DWORD dwLen, int iPort, int iTimeOut, char* pszRetIp, int* piRetPort);
    void BroadCastNotifyOnLine(int iPort);
private:
    BOOL Init(int iPort);
    void AddToMsgList( char * pszRecvBuff, char* pszAddr);
    static DWORD RecvProc(LPVOID* lpParameter);
    static DWORD MsgProc(LPVOID* lpParameter);
    void BroadCast(const char* pszSendBuff, DWORD dwLen, int iPort);
private:
    SOCKET m_iSocketFd;
    std::list<struct BaseMsg*> m_pMsgList;
    CRITICAL_SECTION m_SocketSection;
    CRITICAL_SECTION m_MsgQueueSection;
    struct RelechQueue* m_pMsgQueue;
    HANDLE m_hRecvProcHandle;
    HANDLE m_hMessageHandle;
    OnConnectBroadCastMessageFun m_OnMessageProc;
    LPVOID m_Param;
};


