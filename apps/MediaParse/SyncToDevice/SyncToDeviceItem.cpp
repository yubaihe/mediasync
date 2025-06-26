#include "SyncToDeviceItem.h"
#include "BackupManager.h"
#include "../Util/CommonUtil.h"
#include "../Util/FileUtil.h"
#include "../Util/DbusUtil.h"
#include "../Util/JsonUtil.h"
extern string g_strTempPath;
CSyncToDeviceItem::CSyncToDeviceItem()
{
    m_iThumbSize = 0;
    m_iOrigianlSize = 0;
    m_iTransSize = 0;
    m_iTotalSize = 0;
    m_strErrorInfo = "";

    m_bExit = TRUE;
    m_strThumbFile = "";
    m_strOriginalFile = "";

    m_hSync = NULL;
    m_iOldTimeSec = CCommonUtil::CurTimeSec();
}

CSyncToDeviceItem::~CSyncToDeviceItem()
{
    Reset();
}
void CSyncToDeviceItem::Reset()
{
    m_bExit = TRUE;
    if(NULL != m_hSync)
    {
        WaitForSingleObject(m_hSync, INFINITE);
        CloseHandle(m_hSync);
        m_hSync = NULL;
    }
}
BOOL CSyncToDeviceItem::Start(BackupItemFull item)
{
    m_Item = item;
    CBackupFlod backupfold(item.strFoldName, FALSE);
    string strFileName = item.strFile;
    string strThumbFileName = backupfold.GetThumbFile(item.eMediaType, strFileName);
    string strFileRoot = backupfold.GetBackupRoot();
    string strThumbRoot = backupfold.GetBackupThumbRoot();

    m_strOriginalFile = CCommonUtil::StringFormat("%s%s", strFileRoot.c_str(), strFileName.c_str());
    m_strThumbFile = CCommonUtil::StringFormat("%s%s", strThumbRoot.c_str(), strThumbFileName.c_str());

    printf("Original file:%s\n", m_strOriginalFile.c_str());
    printf("Thumb file:%s\n", m_strThumbFile.c_str());

    m_bExit = FALSE;
    m_iTransSize = 0;
    m_iTotalSize = 1;
    m_strErrorInfo = "";
    m_hSync = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SyncProc, this, 0, NULL);
    return TRUE;
}

DWORD CSyncToDeviceItem::SyncProc(void* lpParameter)
{
    CSyncToDeviceItem* pSyncToDeviceItem = (CSyncToDeviceItem*)lpParameter;
    
    while (FALSE == pSyncToDeviceItem->m_bExit)
    {
        pSyncToDeviceItem->Sync();
    }
    CloseHandle(pSyncToDeviceItem->m_hSync);
    pSyncToDeviceItem->m_hSync = NULL;
    return 1;
}
void CSyncToDeviceItem::Sync()
{
    //先提交缩略图 再更新原图 最后给到DiskItem用Dbus提交到Server
    //检验MD5是否存在
    string strMd5 = CFileUtil::GetFileMd5(m_strOriginalFile.c_str());
    if(strMd5.length() == 0)
    {
        m_bExit = TRUE;
        m_strErrorInfo = "Get md5 failed";
        return;
    }
    char szFile[MAX_PATH] = {0};
    char szErrorInfo[100] = {0};
    BOOL bSuccess = CDbusUtil::CheckMd5(strMd5.c_str(), szErrorInfo, szFile);
    if(FALSE == bSuccess)
    {
        if(0 == strcmp(szErrorInfo, "media exist"))
        {
            //之前已经提交过了 结束
            m_bExit = TRUE;
            m_iTransSize = m_iTotalSize;
            m_strErrorInfo = "media exist";
            return;
        }
        //md5校验失败 结束
        m_bExit = TRUE;
        m_strErrorInfo = "Check md5 failed";
        return;
    }
    m_iThumbSize = CCommonUtil::GetFileSize(m_strThumbFile.c_str());
    m_iOrigianlSize = CCommonUtil::GetFileSize(m_strOriginalFile.c_str());
    if(0 == m_iOrigianlSize || 0 == m_iThumbSize)
    {
        m_bExit = TRUE;
        m_strErrorInfo = "No file find";
        printf("%s\n", m_strErrorInfo.c_str());
        return ;
    }
    m_iTotalSize = m_iThumbSize + m_iOrigianlSize;
    //判断空间是否足够
    DiskInfo diskInfo = CDbusUtil::GetDiskInfo();
    if((diskInfo.iTotal - diskInfo.iUsed) < (m_iThumbSize + m_iOrigianlSize)/1024)
    {
        m_bExit = TRUE;
        m_strErrorInfo = CCommonUtil::StringFormat("No enough space left:%ldKB need:%ldkB\n", diskInfo.iTotal - diskInfo.iUsed, (m_iThumbSize + m_iOrigianlSize)/1024);
        return;
    }
    //向server发送消息创建两个文件 然后把路径返回给我
    string strThumbFile = CDbusUtil::GetUploadFileName();
    if(strThumbFile.length() == 0)
    {
        printf("Get thumb file name error\n");
        m_bExit = TRUE;
        m_strErrorInfo = "Get thumb file name error";
        return;
    }
    //写第一个文件
    char szDestFile[MAX_PATH + 1] = {0};
    sprintf(szDestFile, "%s%s", g_strTempPath.c_str(), strThumbFile.c_str());
    bSuccess = CopyFile(m_strThumbFile.c_str(), szDestFile);
    if(FALSE == bSuccess)
    {
        printf("Copy thumb file error\n");
        m_bExit = TRUE;
        m_strErrorInfo = "Copy thumb file error";
        return;
    }
    //写第二个文件
    string strOriginalFile = CDbusUtil::GetUploadFileName();
    if(strOriginalFile.length() == 0)
    {
        printf("Get file name error\n");
        m_bExit = TRUE;
        m_strErrorInfo = "Get file name error";
        return;
    }
    //写第一个文件
    memset(szDestFile, 0, sizeof(szDestFile));
    sprintf(szDestFile, "%s%s", g_strTempPath.c_str(), strOriginalFile.c_str());
    bSuccess = CopyFile(m_strOriginalFile.c_str(), szDestFile);
    if(FALSE == bSuccess)
    {
        printf("Copy original file error\n");
        m_bExit = TRUE;
        m_strErrorInfo = "Copy original file error";
        return;
    }
    //向server提交 写入数据库
    nlohmann::json jsonRoot;
    jsonRoot["action"] = "reportinfo";
    jsonRoot["utimesec"] = CCommonUtil::CurTimeSec();//提交时间
    jsonRoot["fname"] = m_Item.strFile.c_str();//文件名称
    jsonRoot["sfile"] = strThumbFile.c_str();//缩略图
    jsonRoot["slen"] = m_iThumbSize;//缩略图大小
    jsonRoot["lfile"] = strOriginalFile.c_str();//原图
    jsonRoot["llen"] = m_iOrigianlSize;//原图大小
    jsonRoot["exfile"] = "";//LivePhoto
    jsonRoot["exlen"] = 0;//LivePhoto大小
    jsonRoot["paitime"] = m_Item.iCreateTimeSec; //拍摄时间
    jsonRoot["mediatype"] = m_Item.eMediaType; //媒体类型
    jsonRoot["md5"] = strMd5.c_str(); //MD5值
    jsonRoot["weizhi"] = m_Item.strAddr.c_str(); //位置
    jsonRoot["location"] = m_Item.strLocation.c_str(); //位置
    jsonRoot["devicename"] = m_Item.strFoldName; //磁盘名称
    jsonRoot["width"] = m_Item.iWidth;
    jsonRoot["height"] = m_Item.iHeight;
    jsonRoot["duration"] = m_Item.iDuration;
    string strJson = MediaParse::CJsonUtil::ToString(jsonRoot);
    memset(szErrorInfo, 0, sizeof(szErrorInfo));
    CDbusUtil::ReportInfo(strJson.c_str(), szErrorInfo);
    m_bExit = TRUE;
}
BOOL CSyncToDeviceItem::CopyFile(const char* pszSrc, const char* pszDest)
{
    printf("Copy file:\nSrc:%s\nDest:%s\n", pszSrc, pszDest);
    FILE* pSrc = fopen(pszSrc, "rb");
    if(NULL == pSrc)
    {
        return FALSE;
    }
    FILE* pDest = fopen(pszDest, "wb");
    if(NULL == pDest)
    {
        fclose(pSrc);
        pSrc = NULL;
    }
    char szBuffer[2048] = { 0 };
    int iBufferLen = sizeof(szBuffer);
    while(TRUE)
    {
        memset(szBuffer, 0, iBufferLen);
        int iReadLen = fread(szBuffer, sizeof(char), iBufferLen, pSrc);
        if(iReadLen <= 0)
        {
            break;
        }
        int iWriteLen = fwrite(szBuffer, sizeof(char), iReadLen, pDest);
        if(iWriteLen != iReadLen)
        {
            break;
        }
        m_iTransSize += iWriteLen;
    }
    fclose(pSrc);
    fclose(pDest);
    size_t iSrcFileSize = CCommonUtil::GetFileSize(pszSrc);
    size_t iDestFileSize = CCommonUtil::GetFileSize(pszDest);
    return iSrcFileSize == iDestFileSize?TRUE:FALSE;
}
int CSyncToDeviceItem::GetPrecent(char* pszErrorInfo)
{
    m_iOldTimeSec = CCommonUtil::CurTimeSec();
    int iPrcent = (int)(((m_iTransSize*1.0f)/m_iTotalSize)*100);
    strcpy(pszErrorInfo, m_strErrorInfo.c_str());
    if(iPrcent == 100 && NULL != m_hSync)
    {
        iPrcent = 99;
    }
    if(iPrcent == 100)
    {
        m_iOldTimeSec = 0;
    }
    return iPrcent;
}
BOOL CSyncToDeviceItem::CheckExit()
{
    if(NULL == m_hSync && CCommonUtil::CurTimeSec() - m_iOldTimeSec > 60)
    {
        return TRUE;
    }
    return FALSE;
}