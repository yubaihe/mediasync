#pragma once
#include "stdafx.h"
#include "ConnectServerMessage.h"
class CConnectServer
{
public:
    CConnectServer();
    ~CConnectServer();
    BOOL Start(const char* pszDbusName);
private:
    BOOL InitStore();
    static void OnServerMessage(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen);
private:
    void* m_pDbusContext;
    CConnectServerMessage m_ConnectServerMessage;
};


