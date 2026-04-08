#ifndef LIBDBUS_H__
#define LIBDBUS_H__
#include <dbus/dbus.h>
typedef void* LPVOID;
typedef struct 
{
    void* pContext;
    void* pCallBack;
    void* pSignalCallBack;
    LPVOID pParam;
}DbusContext;
#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0
#endif
typedef void (*OnDbusMessageHandler)(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen);
typedef void (*OnDbusSignalHandler)(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen);
typedef void (*OnDbusLog)(const char* pszLog);

#ifdef __cplusplus
extern "C" {
#endif
    //不要在回调函数调用这里面的任意一个函数 否则会死循环
    void LibDbus_SetHexLog(OnDbusLog onDbusLog); 
    void* LibDbus_Init(const char* pszDbusName, int iTimeOut, OnDbusMessageHandler handler, LPVOID param);
    BOOL LibDbus_Reply(void* pContext, DBusMessage* pDBusMessage, const int iCommandID, const char* pszMessage, int iMessageLen);
    void LibDbus_UnInit(void* pVoid);
    BOOL LibDbus_SendSync(const char* pszDbusName, const int iCommandID, const char* pszMessage, int iMessageLen, char* pszRet, int* piRetLen);
    BOOL LibDbus_Send(void* pContext, const char* pszToDbusName, const int iCommandID, const char* pszMessage, int iMessageLen);
    void LibDbus_SetSignalCallBack(void* pContext, OnDbusSignalHandler handler);
    BOOL LibDbus_SendSignal(void* pContext, const int iCommandID, const char* pszMessage, int iMessageLen);
    void LibDbus_ServiceList(char* pszServiceList);
#ifdef __cplusplus
};
#endif

#endif
//////////////////////////使用方法//////////////////////////////////
/***
 *char szRetBuffer[100] = {0};
 *int iDbusLen = 100;
 *BOOL bRet = LibDbus_SendSync("OCPP", 0x5007, (const char*)"", 0, szRetBuffer, &iDbusLen);
 * 
 *void* g_pContext = LibDbus_Init("NETCONFIG", 10, NetWork_OnDbusMessageHandler);
 *void NetWork_OnDbusMessageHandler(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen)
 *{
 *  // printf("Recv Msg:%s\n", pszMessage);
 *  // printf("command id:%d\n", iCommandID);
 *  char szBuffer[65535] = {0};
 *  g_NetWorkHandler.OnRecvMessage(pszMessage, szBuffer);
 *  LibDbus_Reply(pContext, pDBusMessage, 1002, (char*)szBuffer, strlen(szBuffer) + 1);
 *}
 *LibDbus_SendSignal(g_pProxyDbusContext, 1000, "hello word", sizeof("hello word"));
 *LibDbus_SetSignalCallBack(g_pProxyDbusContext, NetWork_OnDbusMessageHandler);
 *void CProxyDbusHandler::OnSignal(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen)
 *{
 *  // printf("Recv Msg:%s\n", pszMessage);
 *  // printf("command id:%d\n", iCommandID);
 *}
 *LibDbus_UnInit(g_pContext);
 */