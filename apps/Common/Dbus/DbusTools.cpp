#include "DbusTools.h"
#include "Package.h"
#include <sys/sysinfo.h>
#include <unistd.h>
BOOL g_bInitDbus = FALSE;
CDbusTools::CDbusTools()
{
    m_pParam = NULL;
    m_bExit = TRUE;
    m_CallBack = NULL;
    m_SignalCallBack = NULL;
    m_iTimeOut = 5;
    m_pDBusConnection = NULL;
    m_hDbusRecv = NULL;
    memset(m_szMessageInterfaceName, 0, sizeof(m_szMessageInterfaceName));
    sprintf(m_szMessageInterfaceName, INTERFACENAME, MESSAGE);
    memset(m_szSignalInterfaceName, 0, sizeof(m_szSignalInterfaceName));
    sprintf(m_szSignalInterfaceName, INTERFACENAME, SIGNAL);
    memset(m_szDbusName, 0, sizeof(m_szDbusName));
    InitializeCriticalSection(&m_DbusMessageMapSection);
    
    // static int dbus_threads_initialized = 0;
    // if (!dbus_threads_initialized) 
    // {
    //     dbus_threads_init_default();
    //     dbus_threads_initialized = 1;
    // }
}

CDbusTools::~CDbusTools()
{
    Release();
    //dbus_shutdown();
}
void CDbusTools::Release()
{
    m_bExit = TRUE;
    if(NULL != m_hDbusRecv)
    {
        WaitForSingleObject(m_hDbusRecv, INFINITE);
        CloseHandle(m_hDbusRecv);
        m_hDbusRecv = NULL;
    }

    EnterCriticalSection(&m_DbusMessageMapSection);
    std::map<uint32_t, DbusMessageItem*>::iterator itor;
    for(itor = m_pDbusMessageMap.begin(); itor != m_pDbusMessageMap.end(); ++itor)
    {
        SignalNotify(&itor->second->sem);
    }
    LeaveCriticalSection(&m_DbusMessageMapSection);
    DeleteCriticalSection(&m_DbusMessageMapSection);
    Sleep(100);
    if(NULL != m_pDBusConnection)
    {
        printf("CDbusTools::Release\n");
        dbus_connection_close(m_pDBusConnection);
        dbus_connection_unref(m_pDBusConnection);
        m_pDBusConnection = NULL;
    }
}

DWORD CDbusTools::DbusMessageProc(void* lpParameter)
{
    CDbusTools* pTool = (CDbusTools*)lpParameter;
    time_t iTime = pTool->GetCurRunTime();
    while (!pTool->m_bExit)
    {
        dbus_connection_read_write(pTool->m_pDBusConnection, 2);
        DBusMessage* pDBusMessage = dbus_connection_pop_message(pTool->m_pDBusConnection);
        if(NULL != pDBusMessage)
        {
            uint8_t* pszBuffer = NULL;
            uint32_t iBufferLen = 0;
            pTool->DoMessageRecv(pDBusMessage, &pszBuffer, &iBufferLen);
            // printf("bRet:%d  iBufferLen:%d\n", bRet, iBufferLen);
            if(iBufferLen > 0)
            {
                int iMsgType = dbus_message_get_type(pDBusMessage);
                switch (iMsgType)
                {
                    case DBUS_MESSAGE_TYPE_SIGNAL:
                    {
                        if(NULL != pTool->m_SignalCallBack)
                        {
                            pTool->m_SignalCallBack(pDBusMessage, pszBuffer, iBufferLen, pTool->m_pParam);
                        }
                        break;
                    }
                    case DBUS_MESSAGE_TYPE_METHOD_CALL:
                    {
                        if(NULL != pTool->m_CallBack)
                        {
                            pTool->m_CallBack(pDBusMessage, pszBuffer, iBufferLen, pTool->m_pParam);
                        }
                        break;
                    }
                }
            }
            if(NULL != pszBuffer)
            {
                free(pszBuffer);
                pszBuffer = NULL;
            }
            dbus_message_unref(pDBusMessage);
            pDBusMessage = NULL;
        }
        else
        {
            usleep(10*1000);
        }
        uint64_t iCurTime = pTool->GetCurRunTime();
        int iDiff = iCurTime - iTime;

        if(iDiff > pTool->m_iTimeOut)
        {
            EnterCriticalSection(&pTool->m_DbusMessageMapSection);
            std::map<uint32_t, DbusMessageItem*>::iterator itor;
            for(itor = pTool->m_pDbusMessageMap.begin(); itor != pTool->m_pDbusMessageMap.end(); ++itor)
            {
                if((int)(iCurTime - itor->second->iStartTime) > pTool->m_iTimeOut)
                {
                    SignalNotify(&itor->second->sem);
                }
            }
            LeaveCriticalSection(&pTool->m_DbusMessageMapSection);
            iTime = pTool->GetCurRunTime();
        }
    }
    return 1;
}
BOOL CDbusTools::InitConn(const char* pszDbusName, void* pParam, OnDbusMessage callBack)
{
    m_pParam = pParam;
    m_CallBack = callBack;
    DBusError err;
    dbus_error_init(&err);
    m_pDBusConnection = dbus_bus_get_private(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) 
    {
        printf("Connection Error (%s)\n", err.message); 
        fprintf(stderr, "Connection Error (%s)\n", err.message); 
        dbus_error_free(&err);
        return FALSE;
    }
    if(NULL == m_pDBusConnection)
    {
        printf("m_pDBusConnection null\n");
        return FALSE;
    }
    char szDbusName[MAX_PATH] = {0};
    sprintf(szDbusName, BUSNAME, pszDbusName);
    memset(m_szDbusName, 0, sizeof(m_szDbusName));
    strcpy(m_szDbusName, pszDbusName);
    dbus_bus_request_name(m_pDBusConnection, szDbusName, DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (dbus_error_is_set(&err)) 
    { 
        printf("Name Error (%s)\n", err.message); 
        fprintf(stderr, "Name Error (%s)\n", err.message); 
        dbus_error_free(&err); 
        dbus_connection_unref(m_pDBusConnection);
        m_pDBusConnection = NULL;
        return FALSE;
    }
    dbus_error_free(&err);
    dbus_connection_set_exit_on_disconnect(m_pDBusConnection, FALSE);
    
    m_bExit = FALSE;
    m_hDbusRecv = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DbusMessageProc, this, 0, NULL);
    return TRUE;
}
void CDbusTools::SetSignalCallBack(OnDbusSignal callBack)
{
    m_SignalCallBack = callBack;

    char szBuffer[MAX_PATH * 2] = {0};
    sprintf(szBuffer, "type='signal',interface='%s'", m_szSignalInterfaceName);
    DBusError err;
    dbus_error_init(&err);
    dbus_bus_add_match(m_pDBusConnection, szBuffer, &err);
    dbus_error_free(&err);
}
BOOL CDbusTools::SendSignal(const char* pszSendData, uint32_t iDataLen)
{
    char szSignalPath[MAX_PATH * 2] = {0};
    sprintf(szSignalPath, PATHNAME, m_szDbusName);
    DBusMessage* pDBusMessage = dbus_message_new_signal(szSignalPath, m_szSignalInterfaceName, SIGNAL);
    if(NULL == pDBusMessage)
    {
        return FALSE;
    }
    dbus_message_set_sender(pDBusMessage, m_szDbusName);
    DBusMessageIter msg;
    DBusMessageIter msgSub;
    dbus_message_iter_init_append(pDBusMessage, &msg);
    if (!dbus_message_iter_append_basic(&msg, DBUS_TYPE_INT32, &iDataLen))
    {
        dbus_message_unref(pDBusMessage);
    	return FALSE;
    }
    if (!dbus_message_iter_open_container(&msg, DBUS_TYPE_ARRAY, "y", &msgSub))
	{
        dbus_message_unref(pDBusMessage);
   		return FALSE;
	}
    if (!dbus_message_iter_append_fixed_array(&msgSub, DBUS_TYPE_BYTE, &pszSendData, iDataLen))
	{
        dbus_message_unref(pDBusMessage);
        return FALSE;
	}
    dbus_message_iter_close_container(&msg, &msgSub);
    dbus_message_set_no_reply(pDBusMessage, TRUE);
    dbus_uint32_t serial = 0;
    BOOL bRet = dbus_connection_send(m_pDBusConnection, pDBusMessage, &serial);
    dbus_message_unref(pDBusMessage);
    return bRet;
}
BOOL CDbusTools::SendDataSync(const char* pszDbusName, const char* pszSendData, uint32_t iDataLen, uint8_t** pszRetBuffer, uint32_t* piRetBufferLen)
{
    DBusMessage* pDBusMessage = GetDBusMessage(pszDbusName);
    if(NULL == pDBusMessage)
    {
        return FALSE;
    }
    DBusMessageIter msg;
    DBusMessageIter msgSub;
    dbus_message_iter_init_append(pDBusMessage, &msg);
    if (!dbus_message_iter_append_basic(&msg, DBUS_TYPE_INT32, &iDataLen))
    {
    	return FALSE;
    }
    if (!dbus_message_iter_open_container(&msg, DBUS_TYPE_ARRAY, "y", &msgSub))
	{
   		return FALSE;
	}
    if (!dbus_message_iter_append_fixed_array(&msgSub, DBUS_TYPE_BYTE, &pszSendData, iDataLen))
	{
        return FALSE;
	}
    dbus_message_iter_close_container(&msg, &msgSub);
    // dbus_message_set_no_reply(pDBusMessage, TRUE);
    
    return WaitForRecive(pDBusMessage, pszRetBuffer, piRetBufferLen);
}      

BOOL CDbusTools::WaitForRecive(DBusMessage* pDBusMessage, uint8_t** pszRetBuffer, uint32_t* piRetBufferLen)
{
    EnterCriticalSection(&m_DbusMessageMapSection);
    dbus_uint32_t iSerialID;
    if (!dbus_connection_send(m_pDBusConnection, pDBusMessage, &iSerialID))
    {
        LeaveCriticalSection(&m_DbusMessageMapSection);
        return FALSE;
    }

    int iBufferLen = sizeof(DbusMessageItem);
    char* pszBuffer = (char*)malloc(iBufferLen);
    ASSERT(NULL != pszBuffer);
    memset(pszBuffer, 0, iBufferLen);
    DbusMessageItem* pDbusMessageItem = (DbusMessageItem*)pszBuffer;
    pDbusMessageItem->iRetBufferLen = 0;
    pDbusMessageItem->iSerialID = iSerialID;
    pDbusMessageItem->pszRetBuffer = NULL;
    pDbusMessageItem->iStartTime = GetCurRunTime();
    SignalCreate(&pDbusMessageItem->sem, 0, 0);

    m_pDbusMessageMap.insert(std::pair<uint32_t, DbusMessageItem*>(iSerialID, pDbusMessageItem));
    dbus_connection_flush(m_pDBusConnection);
    LeaveCriticalSection(&m_DbusMessageMapSection);
    SignalWait(&pDbusMessageItem->sem);
    SignalDestroy(&pDbusMessageItem->sem);
    *piRetBufferLen = pDbusMessageItem->iRetBufferLen; 
    *pszRetBuffer = pDbusMessageItem->pszRetBuffer;
    free(pszBuffer);
    pszBuffer = NULL;
    EnterCriticalSection(&m_DbusMessageMapSection);
    m_pDbusMessageMap.erase(iSerialID);
    LeaveCriticalSection(&m_DbusMessageMapSection);
    dbus_message_unref(pDBusMessage);
    return TRUE;
}  

DBusMessage* CDbusTools::GetDBusMessage(const char* pszDbusName)
{
    char szBusName[255] = {0};
    char szPathName[255] = {0};
    sprintf(szBusName, BUSNAME, pszDbusName);
    sprintf(szPathName, PATHNAME, pszDbusName);
    DBusMessage* pDBusMessage = dbus_message_new_method_call(szBusName, szPathName, m_szMessageInterfaceName, MESSAGE);
    return pDBusMessage;
}

BOOL CDbusTools::SendData(const char* pszDbusName, const char* pszSendData, uint32_t iDataLen)
{
    DBusMessage* pDBusMessage = GetDBusMessage(pszDbusName);
    if(NULL == pDBusMessage)
    {
        return FALSE;
    }
    DBusMessageIter msg;
    DBusMessageIter msgSub;
    dbus_message_iter_init_append(pDBusMessage, &msg);
    if (!dbus_message_iter_append_basic(&msg, DBUS_TYPE_INT32, &iDataLen))
    {
    	return FALSE;
    }
    if (!dbus_message_iter_open_container(&msg, DBUS_TYPE_ARRAY, "y", &msgSub))
	{
   		return FALSE;
	}
    if (!dbus_message_iter_append_fixed_array(&msgSub, DBUS_TYPE_BYTE, &pszSendData, iDataLen))
	{
        return FALSE;
	}
    dbus_message_iter_close_container(&msg, &msgSub);
    // dbus_message_set_no_reply(pDBusMessage, TRUE);
    dbus_uint32_t iSerialID;
    if (!dbus_connection_send(m_pDBusConnection, pDBusMessage, &iSerialID))
    {
        return FALSE;
    }
    // dbus_connection_flush(m_pDBusConnection);
    dbus_message_unref(pDBusMessage);
    return TRUE;
}

BOOL CDbusTools::DoMessageRecv(DBusMessage* pDBusMessage, uint8_t** pszRetBuffer, uint32_t* piRetBufferLen)
{
    int iMsgType = dbus_message_get_type(pDBusMessage);
    // dzlog_info("iMsgType:%d\n", iMsgType);
    // printf("iMsgType:%d\n", iMsgType);
    // BOOL bReply = FALSE;
    switch (iMsgType)
    {
        case DBUS_MESSAGE_TYPE_SIGNAL:
        {
            if(FALSE == dbus_message_is_signal(pDBusMessage, m_szSignalInterfaceName, SIGNAL))
            {
                break;
            }
            DBusMessageIter msg;
            DBusMessageIter subMsg;
            uint32_t iSerialID;
            uint8_t* pszRet = OnMessageReturn(pDBusMessage, &msg, &subMsg, piRetBufferLen, &iSerialID);
            if(NULL != pszRet)
            {
                *pszRetBuffer = (uint8_t*)malloc(*piRetBufferLen);
                ASSERT(NULL != *pszRetBuffer);
                memcpy(*pszRetBuffer, pszRet, *piRetBufferLen);
            }
            else
            {
                // printf("=========CALL============^__^\n");
            }
            break;
        }
        case DBUS_MESSAGE_TYPE_METHOD_CALL:
        {
            if(FALSE == dbus_message_is_method_call(pDBusMessage, m_szMessageInterfaceName, MESSAGE))
            {
                break;
            }
            DBusMessageIter msg;
            DBusMessageIter subMsg;
            uint32_t iSerialID;
            uint8_t* pszRet = OnMessageReturn(pDBusMessage, &msg, &subMsg, piRetBufferLen, &iSerialID);
            if(NULL != pszRet)
            {
                *pszRetBuffer = (uint8_t*)malloc(*piRetBufferLen);
                ASSERT(NULL != *pszRetBuffer);
                memcpy(*pszRetBuffer, pszRet, *piRetBufferLen);
            }
            break;
        }
        case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        {
            // if(FALSE == dbus_message_is_method_call(pDBusMessage, m_szMessageInterfaceName, MESSAGE))
            // {
            //     break;
            // }
            const char* pszInterface = dbus_message_get_interface(pDBusMessage);
            if(0 != strcmp(pszInterface, m_szMessageInterfaceName))
            {
                printf("=pszInterface===%s==\n", pszInterface);
                break;
            }            
            DBusMessageIter msg;
            DBusMessageIter subMsg;
            uint32_t iSerialID;
            uint8_t* pszRet = OnMessageReturn(pDBusMessage, &msg, &subMsg, piRetBufferLen, &iSerialID);
            EnterCriticalSection(&m_DbusMessageMapSection);
            std::map<uint32_t, DbusMessageItem *>::iterator itor = m_pDbusMessageMap.find(iSerialID);
            if(itor != m_pDbusMessageMap.end())
            {
                //是响应
                // bReply = TRUE;
                if(NULL != pszRet)
                {
                    //reply一个非空值
                    if(*piRetBufferLen > 0)
                    {
                        itor->second->pszRetBuffer = (uint8_t*)malloc(*piRetBufferLen);
                        ASSERT(NULL != itor->second->pszRetBuffer);
                        memcpy(itor->second->pszRetBuffer, pszRet, *piRetBufferLen);
                        itor->second->iRetBufferLen = *piRetBufferLen;
                    }
                    else
                    {
                        //reply一个非空值 长度为0
                        // printf("=========RETURN=============^__^\n");
                        itor->second->pszRetBuffer = (uint8_t*)malloc(1);
                        ASSERT(NULL != itor->second->pszRetBuffer);
                        memset(itor->second->pszRetBuffer, 0, 1);
                        itor->second->iRetBufferLen = 1;
                    }
                }
                else
                {
                    //reply一个空值 
                    // printf("=========Fuck=============^__^\n");
                    itor->second->pszRetBuffer = (uint8_t*)malloc(1);
                    ASSERT(NULL != itor->second->pszRetBuffer);
                    memset(itor->second->pszRetBuffer, 0, 1);
                    itor->second->iRetBufferLen = 1;
                }
                SignalNotify(&itor->second->sem);
            }
            else
            {
                //reply一个不是我发的消息
                printf("=========Double Fuck=============^__^\n");
                // *pszRetBuffer = (uint8_t*)malloc(*piRetBufferLen);
                // ASSERT(NULL != *pszRetBuffer);
                // memcpy(*pszRetBuffer, pszRet, *piRetBufferLen);
            }
            LeaveCriticalSection(&m_DbusMessageMapSection);
            break;
        }
        case DBUS_MESSAGE_TYPE_ERROR:
        {
            break;
        }
    }
    return TRUE;
}

uint8_t* CDbusTools::OnMessageReturn(DBusMessage* pDBusMessage, DBusMessageIter* pMsg, DBusMessageIter* pSubMsg, uint32_t* piRetBufferLen, uint32_t* piSerialID)
{
    *piSerialID = dbus_message_get_serial(pDBusMessage);
   // printf("OnMessageReturn:%d\n", *piSerialID);
    if (!dbus_message_iter_init(pDBusMessage, pMsg))
    {
        return NULL;
    }
    dbus_message_iter_get_basic(pMsg, piRetBufferLen);
    if(*piRetBufferLen == 0)
    {
        return NULL;
    }
    if (!dbus_message_iter_next(pMsg))
	{
		return NULL;
	}
    uint32_t iActureLen = 0;
    dbus_message_iter_recurse(pMsg, pSubMsg);
    uint8_t* pszTmp;
	dbus_message_iter_get_fixed_array(pSubMsg, &pszTmp, (int*)&iActureLen);
    // printf("iActureLen:%d iRetBufferLen:%d\n", iActureLen, *piRetBufferLen);
    if(iActureLen == *piRetBufferLen)
    {
       return pszTmp;
    }
    else
    {
        printf("error\n");
        *piRetBufferLen = 0;
        return NULL;
    }
}

BOOL CDbusTools::SendReplay(DBusMessage* pDBusMessage, const char* pszSendData, uint32_t iDataLen)
{
    DBusMessage* pDBusMessageReplay = dbus_message_new_method_return(pDBusMessage);
    if(NULL == pDBusMessageReplay)
    {
        return FALSE;
    }
    dbus_message_set_member(pDBusMessageReplay, MESSAGE);
    dbus_message_set_interface(pDBusMessageReplay, m_szMessageInterfaceName);
    dbus_message_set_serial(pDBusMessageReplay, dbus_message_get_serial(pDBusMessage));
    DBusMessageIter msg;
    DBusMessageIter msgSub;
    dbus_message_iter_init_append(pDBusMessageReplay, &msg);
    if (!dbus_message_iter_append_basic(&msg, DBUS_TYPE_INT32, &iDataLen))
    {
    	return FALSE;
    }
    if (!dbus_message_iter_open_container(&msg, DBUS_TYPE_ARRAY, "y", &msgSub))
	{
   		return FALSE;
	}
    if (!dbus_message_iter_append_fixed_array(&msgSub, DBUS_TYPE_BYTE, &pszSendData, iDataLen))
	{
        return FALSE;
	}
    dbus_message_iter_close_container(&msg, &msgSub);
    dbus_message_set_no_reply(pDBusMessageReplay, TRUE);
    dbus_uint32_t iSerialID;
    if (!dbus_connection_send(m_pDBusConnection, pDBusMessageReplay, &iSerialID))
    {
        return FALSE;
    }
    // dbus_connection_flush(m_pDBusConnection);
    dbus_message_unref(pDBusMessageReplay);
    return TRUE;
}

void CDbusTools::SetTimeOut(int iTimeOutSec)
{
    m_iTimeOut = iTimeOutSec;
}

time_t CDbusTools::GetCurRunTime()
{
    struct sysinfo info;
    sysinfo(&info);
    return info.uptime;
}