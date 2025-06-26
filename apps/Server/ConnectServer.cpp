#include "ConnectServer.h"
#include "Config.h"
#include "Util/Tools.h"
#include "Util/FileUtil.h"
CConnectServer::CConnectServer()
{
    m_pDbusContext = NULL;
}

CConnectServer::~CConnectServer()
{
    if(NULL != m_pDbusContext)
    {
        LibDbus_UnInit(m_pDbusContext);
        m_pDbusContext = NULL;
    }
}
BOOL CConnectServer::Start(const char* pszDbusName)
{
    if(FALSE == InitStore())
    {
        return FALSE;
    }
    m_pDbusContext = LibDbus_Init(pszDbusName, 10, CConnectServer::OnServerMessage, this);
    if(NULL == m_pDbusContext)
    {
        return FALSE;
    }
    return TRUE;
}
BOOL CConnectServer::InitStore()
{
    ConfigStore store = CConfig::GetInstance()->GetStore();
    if(store.strAddr.length() == 0)
    {
        store.strAddr = "/";
        CConfig::GetInstance()->SetStore(store);
    }
    printf("Root:%s\n", store.strAddr.c_str());
    size_t iTotal = 0;
    size_t iUsed = 0;
    Server::CTools::GetDiskInfo(store.strAddr.c_str(), &iTotal, &iUsed);
    if(iTotal == 0)
    {
        printf("get disk info failed\n");
        exit(0);
        return FALSE;
    }
    printf("Addr:%s Total:%ld Used:%ld\n", store.strAddr.c_str(), iTotal, iUsed);

    string strStoreRoot = CConfig::GetInstance()->GetStoreRoot();
    printf("StoreRoot:%s\n", strStoreRoot.c_str());
    if(FALSE == CFileUtil::CheckFileExist(strStoreRoot))
    {
        CFileUtil::CreateFold(strStoreRoot);
    }
    string strThumbRoot = CConfig::GetInstance()->GetThumbRoot();
    printf("ThumbRoot:%s\n", strThumbRoot.c_str());
    if(FALSE == CFileUtil::CheckFileExist(strThumbRoot))
    {
        CFileUtil::CreateFold(strThumbRoot);
    }
    string strExtraRoot = CConfig::GetInstance()->GetExtraRoot();
    printf("ThumbRoot:%s\n", strExtraRoot.c_str());
    if(FALSE == CFileUtil::CheckFileExist(strExtraRoot))
    {
        CFileUtil::CreateFold(strExtraRoot);
    }
    string strTmpRoot = CConfig::GetInstance()->GetUploadRoot();
    printf("ThumbRoot:%s\n", strTmpRoot.c_str());
    if(FALSE == CFileUtil::CheckFileExist(strTmpRoot))
    {
        CFileUtil::CreateFold(strTmpRoot);
    }
    printf("media upload temp:%s\n", strTmpRoot.c_str());
    return TRUE;
}
void CConnectServer::OnServerMessage(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen)
{
    CConnectServer* pConnectServer = (CConnectServer*)pContext->pParam;
    printf("%s\n", pszMessage);
//    printf("command id:%d\n", iCommandID);
    char szBuffer[MAX_MESSAGE_LEN] = {0};
    pConnectServer->m_ConnectServerMessage.OnMessage(pszMessage, szBuffer);
    printf("%s\n", szBuffer);
    LibDbus_Reply(pContext, pDBusMessage, iCommandID, (char*)szBuffer, strlen(szBuffer) + 1);
}