#include "libdbus.h"
#include "DbusTools.h"
#include "Package.h"
OnDbusLog g_OnDbusLog = NULL;
void LibDbus_PrintfBinHex(unsigned char* pszBin, int iBinLen)
{
    if(0 == iBinLen || NULL == pszBin)
    {
        return;
    }
    char* pszBuffer = (char*)malloc(iBinLen * 3 + 1);
    ASSERT(NULL != pszBuffer);
    memset(pszBuffer, 0, iBinLen * 3 + 1);
    for (int i = 0; i<iBinLen; ++i)
    {
        sprintf(pszBuffer + i * 3, "%02X ", pszBin[i]);
        // pszBuffer[i * 3 + 1] = 0x20;
    }
    if(NULL != g_OnDbusLog)
    {
        // printf("%s\n", pszBuffer);
        g_OnDbusLog(pszBuffer);
    }
    
    free(pszBuffer);
    pszBuffer = NULL;
}

void OnDbusMessageCallBack(DBusMessage* pDBusMessage, uint8_t* pszMessage, uint32_t iMessageLen, void* pParam)
{
    DbusContext* pDbusContext = (DbusContext*)pParam;
    OnDbusMessageHandler pHandle = (OnDbusMessageHandler)pDbusContext->pCallBack;
    LibDbus_PrintfBinHex(pszMessage, iMessageLen);
    CPackage package;
    int iOutLen = 0;
    int iCmd = 0;
    char* pszOut = (char*)package.ParsePackage(pszMessage, iMessageLen, &iOutLen, &iCmd);
    if(NULL == pszOut)
    {
        return ;
    }
    pHandle(pDbusContext, pDBusMessage, iCmd, pszOut, iOutLen);
}
void OnDbusSignalCallBack(DBusMessage* pDBusMessage, uint8_t* pszMessage, uint32_t iMessageLen, void* pParam)
{
    DbusContext* pDbusContext = (DbusContext*)pParam;
    OnDbusMessageHandler pHandle = (OnDbusMessageHandler)pDbusContext->pSignalCallBack;
    LibDbus_PrintfBinHex(pszMessage, iMessageLen);
    CPackage package;
    int iOutLen = 0;
    int iCmd = 0;
    char* pszOut = (char*)package.ParsePackage(pszMessage, iMessageLen, &iOutLen, &iCmd);
    if(NULL == pszOut)
    {
        return ;
    }
    pHandle(pDbusContext, pDBusMessage, iCmd, pszOut, iOutLen);
}
void* LibDbus_Init(const char* pszDbusName, int iTimeOut, OnDbusMessageHandler handler, LPVOID param)
{
    printf("LibDbus_Init\n");
    CDbusTools* pDbusTools = new CDbusTools();
    pDbusTools->SetTimeOut(iTimeOut);

    char* pszDbusContext = (char*)malloc(sizeof(DbusContext));
    ASSERT(NULL != pszDbusContext);
    DbusContext* pDbusContext = (DbusContext*)pszDbusContext;
    pDbusContext->pContext = pDbusTools;
    pDbusContext->pCallBack = (void*)handler;
    pDbusContext->pParam = param;
    BOOL bRet = pDbusTools->InitConn(pszDbusName, pDbusContext, OnDbusMessageCallBack);
    if(FALSE == bRet)
    {
        delete pDbusTools;
        pDbusTools = NULL;
        free(pszDbusContext);
        pszDbusContext = NULL;
        return NULL;
    }
    return pDbusContext;
}
void LibDbus_UnInit(void* pVoid)
{
    printf("LibDbus_UnInit\n");
    DbusContext* pDbusContext = (DbusContext*)pVoid;
    CDbusTools* pDbusTools = (CDbusTools*)pDbusContext->pContext;
    pDbusTools->Release();
    delete pDbusTools;
    pDbusTools = NULL;
    free((char*)pVoid);
}
BOOL LibDbus_Reply(void* pContext, DBusMessage* pDBusMessage, const int iCommandID, const char* pszMessage, int iMessageLen)
{
    DbusContext* pDbusContext = (DbusContext*)pContext;
    CDbusTools* pDbusTools = (CDbusTools*)pDbusContext->pContext;
    CPackage package;
    uint16_t iPackageLen = 0;
	uint8_t* pPackage = package.GetPackage(iCommandID, (uint8_t*)pszMessage, iMessageLen, &iPackageLen);
    LibDbus_PrintfBinHex(pPackage, iPackageLen);
	return pDbusTools->SendReplay(pDBusMessage, (const char*)pPackage, iPackageLen);
}
BOOL LibDbus_SendSync(const char* pszDbusName, const int iCommandID, const char* pszMessage, int iMessageLen, char* pszRet, int* piRetLen)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    char szDbusName[255] = {0};
    sprintf(szDbusName, "Module_dbus_%ld_%ld", now.tv_sec, now.tv_nsec);
    CDbusTools DbusTools;
    DbusTools.SetTimeOut(10);
    BOOL bRet = DbusTools.InitConn(szDbusName, NULL, NULL);
    if(FALSE == bRet)
    {
        return bRet;
    }
    
    uint16_t iPackageLen = 0;
    CPackage package;
    uint8_t* pPackage = package.GetPackage(iCommandID, (uint8_t*)pszMessage, iMessageLen, &iPackageLen);
    LibDbus_PrintfBinHex(pPackage, iPackageLen);
    uint8_t* pszRecv = NULL;
	uint32_t iRecvLen = 0;
    bRet = DbusTools.SendDataSync(pszDbusName, (const char*)pPackage, iPackageLen, &pszRecv, &iRecvLen);
    if(0 == iRecvLen || NULL == pszRecv)
    {
        if(NULL != pszRecv)
        {
            free(pszRecv);
            pszRecv = NULL;
        }
        return FALSE;
    }
    int iOutLen = 0;
    int iCmd = 0;
    LibDbus_PrintfBinHex(pszRecv, iRecvLen);
    char* pszOut = (char*)package.ParsePackage(pszRecv, iRecvLen, &iOutLen, &iCmd);
    ASSERT(NULL != pszOut);
    if(NULL != pszRecv)
    {
        free(pszRecv);
        pszRecv = NULL;
    }

    if(iOutLen > *piRetLen)
    {
        return FALSE;
    }
    *piRetLen = iOutLen;
    memcpy(pszRet, pszOut, *piRetLen);
    return TRUE;
}

BOOL LibDbus_Send(void* pContext, const char* pszToDbusName, const int iCommandID, const char* pszMessage, int iMessageLen)
{
    DbusContext* pDbusContext = (DbusContext*)pContext;
    CDbusTools* pDbusTools = (CDbusTools*)pDbusContext->pContext;
    
    uint16_t iPackageLen = 0;
    CPackage package;
    uint8_t* pPackage = package.GetPackage(iCommandID, (uint8_t*)pszMessage, iMessageLen, &iPackageLen);
    LibDbus_PrintfBinHex(pPackage, iPackageLen);
    BOOL bRet = pDbusTools->SendData(pszToDbusName, (const char*)pPackage, iPackageLen);
    
    return bRet;
}
void LibDbus_SetHexLog(OnDbusLog onDbusLog)
{
    g_OnDbusLog = onDbusLog;
}
BOOL LibDbus_SendSignal(void* pContext, const int iCommandID, const char* pszMessage, int iMessageLen)
{
    DbusContext* pDbusContext = (DbusContext*)pContext;
    CDbusTools* pDbusTools = (CDbusTools*)pDbusContext->pContext;
    
    uint16_t iPackageLen = 0;
    CPackage package;
    uint8_t* pPackage = package.GetPackage(iCommandID, (uint8_t*)pszMessage, iMessageLen, &iPackageLen);
    LibDbus_PrintfBinHex(pPackage, iPackageLen);
   
    BOOL bRet = pDbusTools->SendSignal((const char*)pPackage, iPackageLen);
    return bRet;
}
void LibDbus_SetSignalCallBack(void* pContext, OnDbusSignalHandler handler)
{
    DbusContext* pDbusContext = (DbusContext*)pContext;
    pDbusContext->pSignalCallBack = (void*)handler;
    CDbusTools* pDbusTools = (CDbusTools*)pDbusContext->pContext;
    pDbusTools->SetSignalCallBack(OnDbusSignalCallBack);
}
void LibDbus_ServiceList(char* pszServiceList)
{
    DBusError error;
    dbus_error_init(&error);
    DBusConnection* pDBusConnection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
    if (dbus_error_is_set(&error)) 
    {
        printf("Connection Error (%s)\n", error.message); 
        fprintf(stderr, "Connection Error (%s)\n", error.message); 
        dbus_error_free(&error);
        return ;
    }
    dbus_error_free(&error);
    DBusMessage* pDBusMessage = dbus_message_new_method_call(
        "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "ListNames"
    );
    if(NULL == pDBusMessage)
    {
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }
    DBusPendingCall* pDBusPendingCall = NULL;
    dbus_connection_send_with_reply(pDBusConnection, pDBusMessage, &pDBusPendingCall, -1);
    if(NULL == pDBusPendingCall)
    {
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }
    dbus_pending_call_block(pDBusPendingCall);
    DBusMessage* pDBusMessageReply = dbus_pending_call_steal_reply(pDBusPendingCall);
    dbus_pending_call_unref(pDBusPendingCall);
    if(NULL == pDBusMessageReply)
    {
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }
    DBusMessageIter iter;
    dbus_message_iter_init(pDBusMessageReply, &iter);
    DBusMessageIter array_iter;
    dbus_message_iter_recurse(&iter, &array_iter);
    
    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING) 
    {
        const char* pszServiceName = NULL;
        dbus_message_iter_get_basic(&array_iter, &pszServiceName);
        char szName[100] = {0};
        sscanf(pszServiceName, BUSNAME, szName);
        if(0 == strlen(szName) || 0 == strncmp(szName, "Module_dbus_", strlen("Module_dbus_")))
        {
            dbus_message_iter_next(&array_iter);
            continue;
        }
        strcat(pszServiceList, szName);
        strcat(pszServiceList, ",");
        dbus_message_iter_next(&array_iter);
    }
    dbus_message_unref(pDBusMessage);
    dbus_message_unref(pDBusMessageReply);
    dbus_connection_close(pDBusConnection);
    dbus_connection_unref(pDBusConnection);
    if(strlen(pszServiceList) > 0)
    {
        pszServiceList[strlen(pszServiceList) - 1] = '\0'; 
    }
}