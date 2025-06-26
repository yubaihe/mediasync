#include "MessageHandler.h"
#include "Dbus/libdbus.h"
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include "Common.h"
#define DBUS_NETWORK "Network"
CMessageHandler::CMessageHandler()
{
}

CMessageHandler::~CMessageHandler()
{
}
void CommonSdk_WriteLog(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszLog)
{
    printf("%s\n", pszLog);
}
//curl -X POST -H "Content-Type: application/json" -d "{\"action\":\"deviceinfo\"}" http://192.168.110.8/mediaparse.fcgi
void CMessageHandler::OnMessage(const char* pMsg, char* pRet)
{
    int iDbusLen = MAX_MESSAGE_LEN;
    if(0 == strlen(pMsg))
    {
        char szMessage[] = "{\"action\":\"test\"}";
        LibDbus_SendSync(DBUS_NETWORK, 0x5007, (const char*)szMessage, strlen(szMessage) + 1, pRet, &iDbusLen);
        return;
    }
    int iMsgLen = 0;
    if(NULL != pMsg)
    {
        iMsgLen = strlen(pMsg) + 1;
    }
    BOOL bRet = LibDbus_SendSync(DBUS_NETWORK, 0x5007, (const char*)pMsg, iMsgLen, pRet, &iDbusLen);
    if(FALSE == bRet)
    {
        memset(pRet, 0, strlen(pRet));
        sprintf(pRet, "{\"status\":%d,\"info\":\"Inner error\",\"env\":\"%s\"}", bRet, getenv("DBUS_SESSION_BUS_ADDRESS"));
    }
}