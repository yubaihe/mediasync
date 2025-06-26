#ifndef DBUSTOOLS_H__
#define DBUSTOOLS_H__
#include "stdafx.h"
#include <dbus/dbus.h>
#define BUSNAME                 "com.relech.%s"
#define PATHNAME                "/com/relech/%s"
#define INTERFACENAME           "com.relech.%s"
#define MESSAGE                 "Message"
#define SIGNAL                  "Signal"
typedef struct 
{
    uint32_t iSerialID;
    uint8_t* pszRetBuffer;
    uint32_t iRetBufferLen;
    sem_t sem;
    uint64_t iStartTime;
}DbusMessageItem;
typedef	void(*OnDbusMessage)(DBusMessage* pDBusMessage, uint8_t* pszMessage, uint32_t iMessageLen, void* pParam);
typedef	void(*OnDbusSignal)(DBusMessage* pDBusMessage, uint8_t* pszMessage, uint32_t iMessageLen, void* pParam);
class CDbusTools
{
public:
    CDbusTools();
    ~CDbusTools();
    BOOL InitConn(const char* pszDbusName, void* pParam, OnDbusMessage callBack);
    void Release();
    BOOL SendDataSync(const char* pszDbusName, const char* pszSendData, uint32_t iDataLen, uint8_t** pszRetBuffer, uint32_t* iRetBufferLen);
    BOOL SendData(const char* pszDbusName, const char* pszSendData, uint32_t iDataLen);
    BOOL SendReplay(DBusMessage* pDBusMessage, const char* pszSendData, uint32_t iDataLen);
    void SetTimeOut(int iTimeOutSec);
    void SetSignalCallBack(OnDbusSignal callBack);
    BOOL SendSignal(const char* pszSendData, uint32_t iDataLen);
private:
    DBusMessage* GetDBusMessage(const char* pszDbusName);
    static DWORD DbusMessageProc(void* lpParameter);
    static DWORD DbusMessageDealProc(void* lpParameter);
    BOOL DoMessageRecv(DBusMessage* pDBusMessage, uint8_t** pszRetBuffer, uint32_t* piRetBufferLen);
    uint8_t* OnMessageReturn(DBusMessage* pDBusMessage, DBusMessageIter* pMsg, DBusMessageIter* pSubMsg, uint32_t* piRetBufferLen, uint32_t* piSerialID);
    BOOL WaitForRecive(DBusMessage* pDBusMessage, uint8_t** pszRetBuffer, uint32_t* piRetBufferLen);
    time_t GetCurRunTime();
    static DBusHandlerResult HandleSignal(DBusConnection* pDBusConnection, DBusMessage* pDBusMessage, void* pUserData);
private:
    DBusConnection* m_pDBusConnection;
    HANDLE m_hDbusRecv;
    CRITICAL_SECTION m_DbusMessageMapSection;
    std::map<uint32_t, DbusMessageItem*> m_pDbusMessageMap;
    BOOL m_bExit;
    void* m_pParam;
    int m_iTimeOut;
    char m_szSignalInterfaceName[MAX_PATH];
    char m_szMessageInterfaceName[MAX_PATH];
    char m_szDbusName[MAX_PATH];
    OnDbusMessage m_CallBack;
    OnDbusSignal m_SignalCallBack;
};
#endif