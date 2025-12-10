#include "ItemUpload.h"
#include "DiskItem.h"
#include "DbusUtil.h"
#include "CommonUtil.h"
#include "FileUtil.h"
#include "Util/JsonUtil.h"
extern string g_strTempPath;
CItemUpload::CItemUpload()
{
    m_bExit = TRUE;
    m_hUpload = NULL;
}

CItemUpload::~CItemUpload()
{
}
void CItemUpload::Reset()
{
    m_bExit = TRUE;
    if(NULL != m_hUpload)
    {
        WaitForSingleObject(m_hUpload, INFINITE);
        CloseHandle(m_hUpload);
        m_hUpload = NULL;
    }
}
BOOL CItemUpload::UploadItem(int iItemID, string strThumb, string strFile, CDiskItem* pDiskItem)
{
    if(NULL != m_hUpload)
    {
        m_strErrorInfo = "transfering";
        return FALSE;
    }
    m_strThumbFile = strThumb;
    m_strOriginalFile = strFile;
    m_iThumbSize = CCommonUtil::GetFileSize(strThumb.c_str());
    m_iOrigianlSize = CCommonUtil::GetFileSize(strFile.c_str());
    if(0 == m_iThumbSize || 0 == m_iOrigianlSize)
    {
        m_strErrorInfo = "No file find";
        printf("%s\n", m_strErrorInfo.c_str());
        return FALSE;
    }
    m_iTotalSize = m_iThumbSize + m_iOrigianlSize;
    m_iTransSize = 0;
    m_strErrorInfo = "";
    m_iItemID = iItemID;
    m_pDiskItem = pDiskItem;
    m_bExit = FALSE;
    m_hUpload = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UploadItemProc, this, 0, NULL);
    return TRUE;
}
DWORD CItemUpload::UploadItemProc(void* lpParameter)
{
    CItemUpload* pItemUpload = (CItemUpload*)lpParameter;
    
    while (FALSE == pItemUpload->m_bExit)
    {
        pItemUpload->Upload();
    }
    CloseHandle(pItemUpload->m_hUpload);
    pItemUpload->m_hUpload = NULL;
    return 1;
}
void CItemUpload::Upload()
{
    //先提交缩略图 再更新原图 最后给到DiskItem用Dbus提交到Server
    m_iTransSize = 0;
    m_strErrorInfo = "";
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
            m_strErrorInfo = "";
            return;
        }
        //md5校验失败 结束
        m_bExit = TRUE;
        m_strErrorInfo = "Check md5 failed";
        return;
    }
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
    PhotoItem photoItem = m_pDiskItem->GetItemByItemIndex(m_iItemID);
    string strPaiSheTime = m_pDiskItem->GetCreateDate(m_iItemID);
    time_t iPaiSheTimeSec = CCommonUtil::TimeToSec(strPaiSheTime);
    nlohmann::json jsonRoot;
    jsonRoot["action"] = "reportinfo";
    jsonRoot["utimesec"] = CCommonUtil::CurTimeSec();//提交时间
    jsonRoot["fname"] = photoItem.strName.c_str();//文件名称
    jsonRoot["sfile"] = strThumbFile.c_str();//缩略图
    jsonRoot["slen"] = m_iThumbSize;//缩略图大小
    jsonRoot["lfile"] = strOriginalFile.c_str();//原图
    jsonRoot["llen"] = m_iOrigianlSize;//原图大小
    jsonRoot["exfile"] = "";//LivePhoto
    jsonRoot["exlen"] = 0;//LivePhoto大小
    jsonRoot["paitime"] = iPaiSheTimeSec; //拍摄时间
    jsonRoot["mediatype"] = photoItem.iMediaType; //媒体类型
    jsonRoot["md5"] = strMd5.c_str(); //MD5值
    jsonRoot["weizhi"] = m_pDiskItem->GetAddr(m_iItemID); //位置
    jsonRoot["location"] = ""; //位置
    jsonRoot["devicename"] = m_pDiskItem->GetDiskLabel(); //磁盘名称
    jsonRoot["width"] = photoItem.iWidth;
    jsonRoot["height"] = photoItem.iHeight;
    jsonRoot["duration"] = m_pDiskItem->GetDuration(m_iItemID);
    jsonRoot["comment"] = "";
    jsonRoot["commentshort"] = "";
    string strJson = MediaParse::CJsonUtil::ToString(jsonRoot);
    memset(szErrorInfo, 0, sizeof(szErrorInfo));
    CDbusUtil::ReportInfo(strJson.c_str(), szErrorInfo);
    m_bExit = TRUE;
}
BOOL CItemUpload::CopyFile(const char* pszSrc, const char* pszDest)
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
int CItemUpload::GetPrecent(int* piItemID, char* pszErrorInfo)
{
    int iPrcent = (int)(((m_iTransSize*1.0f)/m_iTotalSize)*100);
    strcpy(pszErrorInfo, m_strErrorInfo.c_str());
    if(iPrcent == 100 && NULL != m_hUpload)
    {
        iPrcent = 99;
    }
    *piItemID = m_iItemID;
    return iPrcent;
}