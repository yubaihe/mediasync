#include "MediaScan.h"
#include "Config.h"
#include "./Util/FileUtil.h"
#include "DataBase/MediaInfoTable.h"
#include "Util/DbusUtil.h"
CMediaScan::CMediaScan()
{
    m_bEnd = TRUE;
    m_iPrefxLen = 0;
    m_CallBack = NULL;
    m_lpParameter = NULL;
    m_hScanThumb = NULL;
    m_hScanOriginal = NULL;
}

CMediaScan::~CMediaScan()
{
}
BOOL CMediaScan::ScanThumb(MediaScanCallBack callBack, LPVOID* lpParameter)
{
    //通过缩略图找原图 如果没有找到就把缩略图删除掉
    if(FALSE == m_bEnd)
    {
        return FALSE;
    }
    Stop();
    m_strRoot = CConfig::GetInstance()->GetThumbRoot();
    m_iPrefxLen = m_strRoot.length();
    m_CallBack = callBack;
    m_lpParameter = lpParameter;
    m_hScanThumb = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MediaScanThumbProc, this, 0, NULL);
    return TRUE;
}
BOOL CMediaScan::ScanOriginal(MediaScanCallBack callBack, LPVOID* lpParameter)
{
    if(FALSE == m_bEnd)
    {
        return FALSE;
    }
    Stop();
    m_strRoot = CConfig::GetInstance()->GetStoreRoot();
    m_iPrefxLen = m_strRoot.length();
    m_CallBack = callBack;
    m_lpParameter = lpParameter;
    m_hScanOriginal = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MediaScanOriginalProc, this, 0, NULL);
    return TRUE;
}
void CMediaScan::Stop()
{
    if(NULL != m_hScanThumb)
    {
        WaitForSingleObject(m_hScanThumb, INFINITE);
        CloseHandle(m_hScanThumb);
        m_hScanThumb = NULL;
    }
    if(NULL != m_hScanOriginal)
    {
        WaitForSingleObject(m_hScanOriginal, INFINITE);
        CloseHandle(m_hScanOriginal);
        m_hScanOriginal = NULL;
    }
}
DWORD CMediaScan::MediaScanThumbProc(LPVOID* lpParameter)
{
    //pthread_detach(pthread_self());
    printf("=========MediaScanThumbProc start==========\n");
    CMediaScan* pMediaScan = (CMediaScan*)lpParameter;
    pMediaScan->m_FileList.clear();
    pMediaScan->RunThumbRoot();
    pMediaScan->m_CallBack(100, pMediaScan->m_lpParameter);
    pMediaScan->m_bEnd = TRUE;
    printf("=========MediaScanThumbProc end==========\n");
    return 1;
}
/*
 *首先读取缩略图里面的所有文件，判断原图文件是否存在不存在删除原图文件，然后从数据库中删除该记录
 */
BOOL CMediaScan::IsNeedEnterDir(const char* pDir)
{
    vector<string> vec = Server::CCommonUtil::StringSplit(pDir, "/");
    if(vec[vec.size() - 1].length() != 4)
    {
        return FALSE;
    }
    const char* psz = vec[vec.size() - 1].c_str();
    if(isdigit(psz[0]) && isdigit(psz[1]) &&  isdigit(psz[2]) &&  isdigit(psz[3]))
    {
        return TRUE;
    }
    return FALSE;
}
void CMediaScan::ScanDir(const char* pszDir)
{
    //printf("open dir: %s\n", pszDir);
    DIR* pRootDir = NULL;
    struct dirent* pEntry = NULL;
    if((pRootDir = opendir(pszDir)) == NULL)
    {
        //printf("can't open dir.\n");
        return;
    }
    chdir(pszDir);
    while(NULL != (pEntry = readdir(pRootDir))) 
    { 
        // 获取下一级目录信息
        struct stat statbuf;
        lstat(pEntry->d_name, &statbuf);   // 获取下一级成员属性
        
        if(S_ISDIR(statbuf.st_mode)) 
        {      
            // 判断下一级成员是否是目录  
            if (strcmp(".", pEntry->d_name) == 0 || strcmp("..", pEntry->d_name) == 0)
            {
                continue;
            }
            char szDir[520] = {0};
            sprintf(szDir, "%s%s", pszDir, pEntry->d_name);
            //printf("check dir:%s\n", szDir);
            BOOL bRet = IsNeedEnterDir(szDir);
            if(bRet)
            {
                ScanDir(szDir);
                string strBuffer(pszDir);
                strBuffer.append(pEntry->d_name);
                string strFile = strBuffer.substr(m_iPrefxLen);
                m_FileList.push_back(strFile);
            }
            else
            {
                printf("No need Enter:%s\n", szDir);
            }
        }
        else
        {
            string strBuffer(pszDir);
            strBuffer.append("/");
            strBuffer.append(pEntry->d_name);
            string strFile = strBuffer.substr(m_iPrefxLen);
            const char* pszFile = strFile.c_str();
            if(isdigit(pszFile[0]) && isdigit(pszFile[1]) &&  isdigit(pszFile[2]) &&  isdigit(pszFile[3]))
            {
                printf("strFile:%s\n", strFile.c_str());
                m_FileList.push_back(strFile);
            }
            else
            {
                printf("No need Add:%s\n", strBuffer.c_str());
            }
        }
    }
    chdir(".."); // 回到上级目录  
    closedir(pRootDir);
}
void CMediaScan::RunThumbRoot()
{
    ScanDir(CConfig::GetInstance()->GetThumbRoot().c_str());
    int iTotalLen = m_FileList.size();
    while (m_FileList.size() > 0)
    {
        if(NULL != m_CallBack)
        {
            int iDealLen = iTotalLen - m_FileList.size();
            int iPrecent = (iDealLen*100.0)/iTotalLen;
            m_CallBack(iPrecent, m_lpParameter);
        }
        string strThumbFileName = m_FileList.front();
        m_FileList.pop_front();

        string strThumbFile(CConfig::GetInstance()->GetThumbRoot());
        strThumbFile.append(strThumbFileName);
        printf("ThumbFile:%s\n", strThumbFile.c_str());
        // if(FileUtil_IsFile(szThumbFile) && FALSE == FileUtil_IsSoftLink(szThumbFile))
        if(CFileUtil::IsFile(strThumbFile))
        {
            string strOriginalFileName = CFileUtil::FileNameFromThumbName(strThumbFileName);
            if(strOriginalFileName.length() == 0)
            {
                printf("RemoveFile:%s\n", strThumbFile.c_str());
                CFileUtil::RemoveFile(strThumbFile);
                continue;
            }
            string strOriginalFile(CConfig::GetInstance()->GetStoreRoot());
            strOriginalFile.append(strOriginalFileName);
            //文件处理
            if(FALSE == CFileUtil::CheckFileExist(strOriginalFile))
            {
                printf("OriginalFile:%s\n", strOriginalFile.c_str());
                BOOL bRet = CFileUtil::RemoveFile(strThumbFile);
                if(TRUE == bRet)
                {
                    string strExFileName = CFileUtil::FileExNameFromFileName(strOriginalFileName);
                    string strExFile(CConfig::GetInstance()->GetExtraRoot());
                    strExFile.append(strExFileName);
                    printf("ExFile:%s\n", strExFile.c_str());
                    if(TRUE == bRet)
                    {
                        CFileUtil::RemoveFile(strExFile);
                    }
                    //数据库里面也要把这条记录删除掉
                    bRet = CMediaInfoTable::RemoveItemFromName(strOriginalFileName);
                    if(FALSE == bRet)
                    {
                        CFileUtil::CreateEmptyFile(strThumbFile);
                        CFileUtil::CreateEmptyFile(strExFile);
                    }
                }
            }
        }
        else if(CFileUtil::IsFold(strThumbFile))
        {
            if(TRUE == CFileUtil::IsEmptyDir(strThumbFile.c_str()))
            {
                CFileUtil::RemoveFold(strThumbFile.c_str());
                
                string strOriginalFile(CConfig::GetInstance()->GetStoreRoot());
                strOriginalFile.append(strThumbFileName);
                CFileUtil::RemoveFold(strOriginalFile.c_str());
                
                string strExFile(CConfig::GetInstance()->GetExtraRoot());
                strExFile.append(strThumbFileName);
                CFileUtil::RemoveFold(strExFile.c_str());
            }
        }
    }
}
//=========================================================原图处理=====================================
DWORD CMediaScan::MediaScanOriginalProc(LPVOID* lpParameter)
{
    //pthread_detach(pthread_self());
    printf("=======================Deal Original====================================\n");
    CMediaScan* pMediaScan = (CMediaScan*)lpParameter;
    pMediaScan->m_FileList.clear();
    pMediaScan->RunOriginalRoot();
    pMediaScan->m_CallBack(100, pMediaScan->m_lpParameter);
    pMediaScan->m_bEnd = TRUE;
    return 1;
}
void CMediaScan::StoreFile(const char* pszFileName, BOOL bExtra)
{
    vector<string> vec = Server::CCommonUtil::StringSplit(pszFileName, "/");
    if(vec.size() != 2)
    {
        return;
    }
    string strMediaRoot(CConfig::GetInstance()->GetStoreRoot());
    string strBackFold(strMediaRoot);
    strBackFold.append(vec[0]);
    strBackFold.append("/undefined/");
    //创建undefined目录
    CFileUtil::CreateFold(strBackFold);
    string strSrcFile("");
    if(TRUE == bExtra)
    {
        strSrcFile = CConfig::GetInstance()->GetExtraRoot();
    }
    else
    {
         strSrcFile = CConfig::GetInstance()->GetStoreRoot();
    }
    strSrcFile.append(pszFileName);


    CFileUtil::MoveFile(strSrcFile, strBackFold);
}
void CMediaScan::RunOriginalRoot()
{
    ScanDir(CConfig::GetInstance()->GetStoreRoot().c_str());
    int iTotalLen = m_FileList.size();
    while (m_FileList.size() > 0)
    {
        if(NULL != m_CallBack)
        {
            int iDealLen = iTotalLen - m_FileList.size();
            int iPrecent = (iDealLen*100.0)/iTotalLen;
            m_CallBack(iPrecent, m_lpParameter);
        }
        string strOriginalFileName = m_FileList.front();
        m_FileList.pop_front();
        printf("=========>%s\n", strOriginalFileName.c_str());
        string strOriginalFile(CConfig::GetInstance()->GetStoreRoot());
        strOriginalFile.append(strOriginalFileName);
        //文件处理
        if(CFileUtil::IsFold(strOriginalFile))
        {
            if(TRUE == CFileUtil::IsEmptyDir(strOriginalFile.c_str()))
            {
                CFileUtil::RemoveFold(strOriginalFile.c_str());
                
                string strThumbFile(CConfig::GetInstance()->GetThumbRoot());
                strThumbFile.append(strOriginalFileName);
                CFileUtil::RemoveFold(strThumbFile.c_str());
                
                string strExFile(CConfig::GetInstance()->GetExtraRoot());
                strExFile.append(strOriginalFileName);
                CFileUtil::RemoveFold(strExFile.c_str());
            }
        }
        else
        {
            //判断这个文件是不是在数据库里面
            MediaInfoItem item = CMediaInfoTable::GetItemFromName(strOriginalFileName);
            if(item.iID < 0)
            {
                //把ext文件和源文件存起来
                string strThumbFileName = CFileUtil::ThumbNameFromFileName(strOriginalFileName);
                printf("ThumbFileName:%s\n", strThumbFileName.c_str());
                string strExFileName = CFileUtil::FileExNameFromFileName(strOriginalFileName);
                printf("ExFileName:%s\n", strExFileName.c_str());

                StoreFile(strOriginalFileName.c_str(), FALSE);
                //CFileUtil::RemoveFile(strOriginalFile);

                string strThumbFile(CConfig::GetInstance()->GetThumbRoot());
                strThumbFile.append(strThumbFileName);
                CFileUtil::RemoveFile(strThumbFile.c_str());
                
                //string strExFile(CConfig::GetInstance()->GetExtraRoot());
                //strExFile.append(strExFileName);
                //CFileUtil::RemoveFile(strExFile.c_str());
                StoreFile(strExFileName.c_str(), TRUE);
                
                continue;
            }
            string strThumbFileName = CFileUtil::ThumbNameFromFileName(strOriginalFileName);
            string strThumbFile(CConfig::GetInstance()->GetThumbRoot());
            strThumbFile.append(strThumbFileName);
            printf("ThumbFile:%s\n", strThumbFile.c_str());
            if(FALSE == CFileUtil::CheckFileExist(strThumbFile))
            {
                printf("=============>Need gen ThumbFile:%s\n", strThumbFile.c_str());
                CDbusUtil::GenThumb(item.iMediaType, strOriginalFile, strThumbFile);
            }
        }
    }
}
