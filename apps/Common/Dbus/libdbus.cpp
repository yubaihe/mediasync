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
    if (pszServiceList == NULL)
    {
        return;
    }
    pszServiceList[0] = '\0';

    DBusError error;
    dbus_error_init(&error);
    // 1. 获取 D-Bus 连接
    DBusConnection* pDBusConnection = dbus_bus_get_private(DBUS_BUS_SESSION, &error);
    if (pDBusConnection == NULL || dbus_error_is_set(&error))
    {
        printf("D-Bus connection error: %s\n", error.message ? error.message : "unknown");
        dbus_error_free(&error);
        return;
    }
    dbus_error_free(&error);
    // 2. 创建方法调用消息
    DBusMessage* pDBusMessage = dbus_message_new_method_call(
        "org.freedesktop.DBus",
        "/org/freedesktop/DBus",
        "org.freedesktop.DBus",
        "ListNames"
    );
    if (pDBusMessage == NULL)
    {
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }

    // 3. 发送调用并等待回复
    DBusPendingCall* pDBusPendingCall = NULL;
    if (!dbus_connection_send_with_reply(pDBusConnection, pDBusMessage, &pDBusPendingCall, -1) ||
        pDBusPendingCall == NULL)
    {
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }

    dbus_pending_call_block(pDBusPendingCall);
    DBusMessage* pDBusMessageReply = dbus_pending_call_steal_reply(pDBusPendingCall);
    dbus_pending_call_unref(pDBusPendingCall);
    if (pDBusMessageReply == NULL)
    {
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }

    // 4. 检查回复是否为方法返回（而非错误）
    if (dbus_message_get_type(pDBusMessageReply) != DBUS_MESSAGE_TYPE_METHOD_RETURN)
    {
        printf("D-Bus method call failed: %s\n", dbus_message_get_error_name(pDBusMessageReply));
        dbus_message_unref(pDBusMessageReply);
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }

    // 5. 解析回复：外层应该是数组类型
    DBusMessageIter iter;
    dbus_message_iter_init(pDBusMessageReply, &iter);
    if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
    {
        printf("Unexpected D-Bus reply signature (expected array)\n");
        dbus_message_unref(pDBusMessageReply);
        dbus_message_unref(pDBusMessage);
        dbus_connection_close(pDBusConnection);
        dbus_connection_unref(pDBusConnection);
        return;
    }

    DBusMessageIter array_iter;
    dbus_message_iter_recurse(&iter, &array_iter);

    // 6. 遍历字符串数组
    while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_STRING)
    {
        const char* pszServiceName = NULL;
        dbus_message_iter_get_basic(&array_iter, &pszServiceName);

        char szName[100] = {0};
        // 根据 BUSNAME 宏提取名字，若格式不匹配则跳过
        if (sscanf(pszServiceName, BUSNAME, szName) != 1)
        {
            dbus_message_iter_next(&array_iter);
            continue;
        }

        // 跳过空名字或以 "Module_dbus_" 开头的名字
        if (strlen(szName) == 0 || strncmp(szName, "Module_dbus_", strlen("Module_dbus_")) == 0)
        {
            dbus_message_iter_next(&array_iter);
            continue;
        }

        // 将名字和逗号追加到局部缓冲区，检查空间
        
        strcat(pszServiceList, szName);
        strcat(pszServiceList, ",");
        dbus_message_iter_next(&array_iter);
    }
    // 清理资源
    dbus_message_unref(pDBusMessageReply);
    dbus_message_unref(pDBusMessage);
    dbus_connection_close(pDBusConnection);
    dbus_connection_unref(pDBusConnection);
}