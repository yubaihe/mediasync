#include "BackupItemUpload.h"
#include "BackupManager.h"
#include "../Util/CommonUtil.h"
#include "DiskManager.h"
#include "../Util/FileUtil.h"
#include "../Util/DbusUtil.h"
extern string g_strTempPath;
CBackupItemUpload::CBackupItemUpload()
{
    m_hUpload = NULL;
    m_strErrorInfo = "";
    m_iThumbSize = 0;
    m_iOrigianlSize = 0;
    m_iTransSize = 0;
    m_iTotalSize = 0;
    m_iOldTimeSec = CCommonUtil::CurTimeSec();
}

CBackupItemUpload::~CBackupItemUpload()
{
}
void CBackupItemUpload::Reset()
{
    if(NULL != m_hUpload)
    {
        WaitForSingleObject(m_hUpload, INFINITE);
        CloseHandle(m_hUpload);
        m_hUpload = NULL;
    }
}
string CBackupItemUpload::GetNewFileName(string strFold, MEDIATYPE eMediaType, string& strOutThumbFile)
{
    string strFoldRoot = CBackupManager::GetInstance()->GetBackupRoot(strFold);
    string strThumbFoldRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(strFold);
    do
    {
        string strFileName = CCommonUtil::StringFormat("%lld", CCommonUtil::CurTimeMilSec());
        if(MEDIATYPE_IMAGE == eMediaType)
        {
            strFileName.append(".jpg");
        }
        else
        {
            strFileName.append(".mp4");
        }
        CBackupFlod fold;
        string strThumbFileName = fold.GetThumbFile(eMediaType, strFileName);
        
        string strFile = strFoldRoot;
        string strThumbFile = strThumbFoldRoot;
        strFile.append(strFileName);
        strThumbFile.append(strThumbFileName);

        BOOL bFileExist = CCommonUtil::CheckFileExist(strFile.c_str());
        BOOL bThumbFileExist = CCommonUtil::CheckFileExist(strThumbFile.c_str());

        //上传的临时文件
        string strTmpFile = CCommonUtil::StringFormat("%s%s", g_strTempPath.c_str(), strFileName.c_str());
        string strTmpFileThumb = CCommonUtil::StringFormat("%s%s", g_strTempPath.c_str(), strThumbFileName.c_str());
        BOOL bTmpFileExist = CCommonUtil::CheckFileExist(strTmpFile.c_str());
        BOOL bTmpFileThumbExist = CCommonUtil::CheckFileExist(strTmpFileThumb.c_str());

        if(FALSE == bFileExist && FALSE == bThumbFileExist&&FALSE == bTmpFileExist && FALSE == bTmpFileThumbExist)
        {
            strOutThumbFile = strThumbFileName;
            return strFileName;
        }
    }while(1);
}
BOOL CBackupItemUpload::UploadItem(string strSrcFile, string strSrcThumbFile, string strDestFold)
{
    printf("CBackupItemUpload::UploadItem\n");
    //源文件
    m_strOriginalFile = strSrcFile;
    m_strThumbFile = strSrcThumbFile;
    m_strFoldName = strDestFold;
    m_strErrorInfo = "";
    if(NULL != m_hUpload)
    {
        m_strErrorInfo = "transfering";
        printf("transfering\n");
        return FALSE;
    }
    m_iThumbSize = CCommonUtil::GetFileSize(m_strThumbFile.c_str());
    m_iOrigianlSize = CCommonUtil::GetFileSize(m_strOriginalFile.c_str());
    if(0 == m_iOrigianlSize || 0 == m_iThumbSize)
    {
        m_strErrorInfo = "No file find";
        printf("%s\n", m_strErrorInfo.c_str());
        return FALSE;
    }
    m_iTotalSize = m_iThumbSize + m_iOrigianlSize;
    
    m_iTransSize = 0;
    m_hUpload = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BackupItemProc, this, 0, NULL);
    return TRUE;
}
int CBackupItemUpload::GetPrecent(char* pszErrorInfo)
{
    m_iOldTimeSec = CCommonUtil::CurTimeSec();
    int iPrcent = (int)(((m_iTransSize*1.0f)/m_iTotalSize)*100);
    strcpy(pszErrorInfo, m_strErrorInfo.c_str());
    if(iPrcent == 100 && NULL != m_hUpload)
    {
        iPrcent = 99;
    }
    if(iPrcent == 100)
    {
        m_iOldTimeSec = 0;
    }
    return iPrcent;
}
DWORD CBackupItemUpload::BackupItemProc(void* lpParameter)
{
    CBackupItemUpload* pBackupItemUpload = (CBackupItemUpload*)lpParameter;
    BOOL bRet = pBackupItemUpload->Backup();
    if(TRUE == bRet)
    {
        if(0 == pBackupItemUpload->m_strErrorInfo.compare("has translated"))
        {
            pBackupItemUpload->m_strErrorInfo = "";
        }
        else
        {
            CCommonUtil::MoveFile(pBackupItemUpload->m_strTmpThumbFile, pBackupItemUpload->m_strBackupThumbFile);
            CCommonUtil::MoveFile(pBackupItemUpload->m_strTmpOriginalFile, pBackupItemUpload->m_strBackupOriginalFile);
            bRet = pBackupItemUpload->Save();
        }
    }
    if(FALSE == bRet)
    {
        pBackupItemUpload->m_strErrorInfo = "save to db error";
    }
    CloseHandle(pBackupItemUpload->m_hUpload);
    pBackupItemUpload->m_hUpload = NULL;
    return 1;
}
BOOL CBackupItemUpload::Save()
{
    BackupItemFull item = {};
    item.strFile = m_strDestFileName;
    item.strMd5 = CFileUtil::GetFileMd5(m_strBackupOriginalFile.c_str());
    item.eMediaType = m_eMediaType;
    item.iMeiTiSize = CCommonUtil::GetFileSize(m_strBackupOriginalFile.c_str());
    item.strLocation = "";
    item.strFoldName = m_strFoldName;
    item.iHasExtra = 0;
    switch(m_eMediaType)
    {
        case MEDIATYPE_IMAGE:
        {
            CImageParse imageParse;
	        imageParse.Parse(m_strBackupOriginalFile.c_str());
            item.iCreateTimeSec = CCommonUtil::TimeToSec(imageParse.GetCreateTime());
            std::vector<std::string> vecLocation = imageParse.GetLocaiton();
            item.strAddr = CCommonUtil::StringFormat("%s&%s", vecLocation[0].c_str(), vecLocation[1].c_str());
            item.iWidth = imageParse.GetWidth();
            item.iHeight = imageParse.GetHeight();
            item.iDuration = 0;
            break;
        }
        case MEDIATYPE_VIDEO:
        {
            CVideoParse videoParse;
	        videoParse.Parse(m_strBackupOriginalFile.c_str());
            item.iCreateTimeSec = CCommonUtil::TimeToSec(videoParse.GetCreateTime());
            std::vector<std::string> vecLocation = videoParse.GetLocaiton();
            item.strAddr = CCommonUtil::StringFormat("%s&%s", vecLocation[0].c_str(), vecLocation[1].c_str());
            item.iWidth = videoParse.GetWidth();
            item.iHeight = videoParse.GetHeight();
            item.iDuration = videoParse.GetDurationSec();
            break;
        }
        default:
        {
            return FALSE;
        }
    }
    CBackupTable table;
    return table.AddItem(item);
}
BOOL CBackupItemUpload::Backup()
{
    printf("CBackupItemUpload::Backup\n");
    //判断空间是否足够
    DiskInfo diskInfo = CDbusUtil::GetDiskInfo();
    if((diskInfo.iTotal - diskInfo.iUsed) < (m_iThumbSize + m_iOrigianlSize)/1024)
    {
        m_strErrorInfo = CCommonUtil::StringFormat("No enough space left:%ldKB need:%ldkB\n", diskInfo.iTotal - diskInfo.iUsed, (m_iThumbSize + m_iOrigianlSize)/1024);
        return FALSE;
    }
    //判断文件是否存在于表中
    string strMd5 = CFileUtil::GetFileMd5(m_strOriginalFile.c_str());
    CBackupTable table;
    BOOL bExist = table.CheckExist(m_strFoldName, strMd5);
    if(TRUE == bExist)
    {
        m_iTransSize = m_iTotalSize;
        m_strErrorInfo = "has translated";
        printf("has translated\n");
        return TRUE;
    }
    string strDestFile = "";
    string strDestThumbFile = "";
    string strMimeType = CCommonUtil::GetMime(m_strOriginalFile.c_str());
    if (TRUE == CCommonUtil::IsMimeTypeImage(strMimeType.c_str()))
    {
        m_eMediaType = MEDIATYPE_IMAGE;
        strDestFile = GetNewFileName(m_strFoldName, MEDIATYPE_IMAGE, strDestThumbFile);
    }
    else if (TRUE == CCommonUtil::IsMimeTypeVideo(strMimeType.c_str()))
    {
        m_eMediaType = MEDIATYPE_VIDEO;
        strDestFile = GetNewFileName(m_strFoldName, MEDIATYPE_VIDEO, strDestThumbFile);
    }
    if(strDestFile.length() == 0 || strDestThumbFile.length() == 0)
    {
        m_strErrorInfo = "not support file";
        printf("not support file\n");
        return FALSE;
    }
    //目的文件名
    m_strDestFileName = strDestFile;
    
    //备份文件
    m_strBackupOriginalFile = CBackupManager::GetInstance()->GetBackupRoot(m_strFoldName);
    if(FALSE == CCommonUtil::CheckFoldExist(m_strBackupOriginalFile.c_str()))
    {
        CCommonUtil::CreateFold(m_strBackupOriginalFile.c_str());
    }
    //备份源文件
    m_strBackupOriginalFile.append(strDestFile);
    m_strBackupThumbFile = CBackupManager::GetInstance()->GetBackupThumbRoot(m_strFoldName);
    if(FALSE == CCommonUtil::CheckFoldExist(m_strBackupThumbFile.c_str()))
    {
        CCommonUtil::CreateFold(m_strBackupThumbFile.c_str());
    }
    //备份目的文件
    m_strBackupThumbFile.append(strDestThumbFile);
    //上传临时文件
    m_strTmpOriginalFile = CCommonUtil::StringFormat("%s%s", g_strTempPath.c_str(), strDestFile.c_str());
    m_strTmpThumbFile = CCommonUtil::StringFormat("%s%s", g_strTempPath.c_str(), strDestThumbFile.c_str());

    printf("BackupOriginalFile:%s\n", m_strBackupOriginalFile.c_str());
    printf("BackupThumbFile:%s\n", m_strBackupThumbFile.c_str());
    //写第一个文件
    BOOL bSuccess = CopyFile(m_strThumbFile.c_str(), m_strTmpThumbFile.c_str());
    if(FALSE == bSuccess)
    {
        printf("Copy thumb file error\n");
        m_strErrorInfo = "Copy thumb file error";
        return FALSE;
    }
    //写第二个文件
    bSuccess = CopyFile(m_strOriginalFile.c_str(), m_strTmpOriginalFile.c_str());
    if(FALSE == bSuccess)
    {
        printf("Copy original file error\n");
        m_strErrorInfo = "Copy original file error";
        return FALSE;
    }
    return TRUE;
}
BOOL CBackupItemUpload::CopyFile(const char* pszSrc, const char* pszDest)
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
BOOL CBackupItemUpload::CheckExit()
{
    if(NULL == m_hUpload && CCommonUtil::CurTimeSec() - m_iOldTimeSec > 60)
    {
        if(0 != m_strErrorInfo.length())
        {
            CCommonUtil::RemoveFile(m_strTmpOriginalFile.c_str());
            CCommonUtil::RemoveFile(m_strTmpThumbFile.c_str());
        }
        return TRUE;
    }
    return FALSE;
}