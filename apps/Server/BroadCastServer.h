#pragma once
#include "stdafx.h"
#include "ConnectBroadCast.h"
class CBroadCastServer
{
public:
    CBroadCastServer();
    ~CBroadCastServer();
    BOOL Start(int iPort);
    void BroadCastNotifyOnLine(int iPort);
private:
    static void OnConnectBroadCastMessage(BaseMsg* pMsg, SOCKET iSocket, LPVOID param);
    void OnGetIdentifyIp(char* pszBuffer, struct sockaddr_in* pSocket);
    void OnGetIdentifyIpAck(char* pszBuffer, struct sockaddr_in* pSocket);
private:
    CConnectBroadCast m_ConnectBroadCast;
};

