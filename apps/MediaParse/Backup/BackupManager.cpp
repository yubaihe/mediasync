#include "BackupManager.h"
#include "../Util/CommonUtil.h"
#include "BackupDb.h"
#include "../Util/FileUtil.h"
#include "../Util/JsonUtil.h"
#include "BackupOrganize.h"
CBackupManager* CBackupManager::m_pInstance = NULL;
CBackupManager::CBackupManager()
{
    m_bSupportBackup = FALSE;
    InitializeCriticalSection(&m_Section);
}

CBackupManager::~CBackupManager()
{
    CBackupDb::Release();
    CBackupOrganize::Release();
    EnterCriticalSection(&m_Section);
    map<string, CBackupItemUpload*>::iterator itor = m_pBackupItemUploadMap.begin();
    while (itor != m_pBackupItemUploadMap.end()) 
    {
        delete itor->second;
        itor->second = NULL;
        itor = m_pBackupItemUploadMap.erase(itor);
    }
    LeaveCriticalSection(&m_Section);
    DeleteCriticalSection(&m_Section);
}
CBackupManager* CBackupManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CBackupManager();
    }
    return m_pInstance;
}
void CBackupManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
BOOL CBackupManager::Init(string strBackupRoot)
{
    if(strBackupRoot.length() > 0)
    {
        m_bSupportBackup = TRUE;
    }
    CFileUtil::CreateFold(strBackupRoot);
    string strBackupDb = strBackupRoot;
    strBackupDb.append(MEDIADBNAME);
    printf("Db File:%s\n", strBackupDb.c_str());
    CBackupDb::GetInstance()->InitAllTables(strBackupDb);

    //将不存在的文件夹从数据库里面删除掉
    CBackupTable table;
    list<string> tableFoldList = table.AllFolds();
    std::vector<FoldEntry> foldEntryVec = ScanFold(strBackupRoot);
    list<string>::iterator itor = tableFoldList.begin();
    for(; itor != tableFoldList.end(); ++itor)
    {
        BOOL bFind = FALSE;
        for(size_t i = 0; i < foldEntryVec.size(); ++i)
        {
            if(0 == strcmp(itor->c_str(), foldEntryVec[i].strName.c_str()))
            {
                foldEntryVec.erase(foldEntryVec.begin() + i);
                bFind = TRUE;
                break;
            }
        }
        if(FALSE == bFind)
        {
            table.DeleteFold(itor->c_str());
            string strThumbPath = GetBackupThumbRoot(*itor);
            CCommonUtil::RemoveFold(strThumbPath.c_str());
        }
    }
    return TRUE;
}
std::vector<FoldEntry> CBackupManager::ScanFold(const std::string strPath) 
{
    std::vector<FoldEntry> retList;
    DIR* pDir = opendir(strPath.c_str());
    if (NULL == pDir)
    {
        printf("无法打开目录: %s\n", strPath.c_str());
        return retList;
    }
    struct dirent* pDirent;
    while ((pDirent = readdir(pDir)) != nullptr) 
    {
        // 跳过特殊目录 "." 和 ".."
        if (std::string(pDirent->d_name) == "." || 
            std::string(pDirent->d_name) == "..") 
        {
            continue;
        }
 
        // 构建完整路径
        std::string strFullPath = strPath + "/" + pDirent->d_name;
        
        // 使用lstat获取文件信息（支持符号链接）
        struct stat pathStat;
        if (lstat(strFullPath.c_str(), &pathStat) == -1)
        {
            continue;  // 跳过无法访问的文件
        }
 
        // 检查是否为目录
        if (S_ISDIR(pathStat.st_mode)) 
        {
            if(0 != strcmp(pDirent->d_name, ".media"))
            {
                FoldEntry entry = {};
                entry.iCreateTimeSec = pathStat.st_ctime;
                entry.strName = pDirent->d_name;
                retList.push_back(entry);
            }
        }
    }
    closedir(pDir);
    return retList;
}
std::vector<BackupItem> CBackupManager::BackupFileList(string strName, int* piPicCount /*= NULL*/, int* piVideoCount /*= NULL*/)
{
    CBackupTable table;
    std::vector<BackupItem> retVec = table.BackupFileListShort(strName, piVideoCount, piPicCount);
    return retVec;
}
bool CBackupManager::CompareFoldWithCreatTime(const FoldEntry& a, const FoldEntry& b) 
{
    return a.iCreateTimeSec > b.iCreateTimeSec;
}
std::map<string, time_t> CBackupManager::BackupThumbFoldMap()
{
    std::vector<FoldEntry> foldEntryList = ScanFold(g_strBackThumbPath);
    std::sort(foldEntryList.begin(), foldEntryList.end(), CompareFoldWithCreatTime);
    std::map<string, time_t> retMap;
    std::vector<FoldEntry>::iterator itor =  foldEntryList.begin();
    for(; itor != foldEntryList.end(); ++itor)
    {
        retMap.insert(pair<string, time_t>(itor->strName, itor->iCreateTimeSec));
    }
    return retMap;
}
std::vector<string> CBackupManager::BackupFoldList()
{
    std::vector<FoldEntry> foldEntryList = ScanFold(g_strBackupPoint);
    std::sort(foldEntryList.begin(), foldEntryList.end(), CompareFoldWithCreatTime);
    std::vector<string> retList;
    std::vector<FoldEntry>::iterator itor =  foldEntryList.begin();
    for(; itor != foldEntryList.end(); ++itor)
    {
        if(0 == itor->strName.compare(".media_tmp"))
        {
            continue;
        }
        retList.push_back(itor->strName);
    }
    return retList;
}
std::map<string, time_t> CBackupManager::BackupFoldMap()
{
    std::vector<FoldEntry> foldEntryList = ScanFold(g_strBackupPoint);
    std::sort(foldEntryList.begin(), foldEntryList.end(), CompareFoldWithCreatTime);
    std::map<string, time_t> retMap;
    std::vector<FoldEntry>::iterator itor =  foldEntryList.begin();
    for(; itor != foldEntryList.end(); ++itor)
    {
        retMap.insert(pair<string, time_t>(itor->strName, itor->iCreateTimeSec));
    }
    return retMap;
}
std::vector<string> CBackupManager::BackupFoldList(int iStart, int iLimited, int* piFoldCount /*= NULL*/)
{
    std::vector<string> allFoldVec = BackupFoldList();
    if(NULL != piFoldCount)
    {
        *piFoldCount = allFoldVec.size();
    }
    std::vector<string> retVec;
    int iCount = 0;
    for(size_t i = iStart; i < allFoldVec.size(); ++i)
    {
        if(iCount == iLimited)
        {
            break;
        }
        iCount++;
        retVec.push_back(allFoldVec[i]);
    }
    return retVec;
}
string CBackupManager::GetBackupRoot(string strName)
{
    string strBackFold = CCommonUtil::StringFormat("%s%s/", g_strBackupPoint.c_str(), strName.c_str());
    return strBackFold;
}
string CBackupManager::GetBackupThumbRoot(string strName)
{
    string strBackThumb = CCommonUtil::StringFormat("%s%s/", g_strBackThumbPath.c_str(), strName.c_str());
    return strBackThumb;
}
string CBackupManager::GetBackupTempRoot(string strName)
{
    if(g_strBackupPoint.length() == 0)
    {
        return "";
    }
    if(strName.length() == 0)
    {
        string strBackTmp = CCommonUtil::StringFormat("%s/.media_tmp/", g_strBackupPoint.c_str());
        return strBackTmp;
    }
    else
    {
        string strBackTmp = CCommonUtil::StringFormat("%s/.media_tmp/%s/", g_strBackupPoint.c_str(), strName.c_str());
        return strBackTmp;
    }
}

string CBackupManager::GetBackupFoldItemDetail(int iItemID)
{
    CBackupTable table;
    BackupItemFull curItem = table.GetItem(iItemID);
    if(curItem.iID < 0)
    {
        return "";
    }
    BackupItemFull nextItem = table.GetNextItem(curItem);
    BackupItemFull prevItem = table.GetPrevItem(curItem);
     
    nlohmann::json jsonRoot;
    jsonRoot["cur"]["itemid"] = iItemID;
    jsonRoot["cur"]["mtype"] = curItem.eMediaType;
    jsonRoot["cur"]["fold"] = curItem.strFoldName;
    jsonRoot["cur"]["file"] = curItem.strFile;
    jsonRoot["cur"]["width"] = curItem.iWidth;
    jsonRoot["cur"]["height"] = curItem.iHeight;
    jsonRoot["cur"]["createtime"] = CCommonUtil::SecToTime(curItem.iCreateTimeSec);
    jsonRoot["cur"]["weizhi"] = curItem.strAddr;
    jsonRoot["cur"]["location"] = curItem.strLocation;
    jsonRoot["cur"]["size"] = (uint64_t)(curItem.iMeiTiSize/1024);
    jsonRoot["cur"]["duration"] = (uint64_t)curItem.iDuration;
    jsonRoot["cur"]["commentshort"] = curItem.strCommentShort;
    if(nextItem.strFile.length() > 0)
    {
        jsonRoot["next"]["itemid"] = nextItem.iID;
        jsonRoot["next"]["mtype"] = nextItem.eMediaType;
        jsonRoot["next"]["fold"] = nextItem.strFoldName;
        jsonRoot["next"]["file"] = nextItem.strFile;
        jsonRoot["next"]["width"] = nextItem.iWidth;
        jsonRoot["next"]["height"] = nextItem.iHeight;
        jsonRoot["next"]["createtime"] = CCommonUtil::SecToTime(nextItem.iCreateTimeSec);
        jsonRoot["next"]["weizhi"] = nextItem.strAddr;
        jsonRoot["next"]["location"] = nextItem.strLocation;
        jsonRoot["next"]["size"] = (uint64_t)(nextItem.iMeiTiSize/1024);
        jsonRoot["next"]["duration"] = (uint64_t)nextItem.iDuration;
        jsonRoot["next"]["commentshort"] = nextItem.strCommentShort;
    }
    if(prevItem.strFile.length() > 0)
    {
        jsonRoot["prev"]["itemid"] = prevItem.iID;
        jsonRoot["prev"]["mtype"] = prevItem.eMediaType;
        jsonRoot["prev"]["fold"] = prevItem.strFoldName;
        jsonRoot["prev"]["file"] = prevItem.strFile;
        jsonRoot["prev"]["width"] = prevItem.iWidth;
        jsonRoot["prev"]["height"] = prevItem.iHeight;
        jsonRoot["prev"]["createtime"] = CCommonUtil::SecToTime(prevItem.iCreateTimeSec);
        jsonRoot["prev"]["weizhi"] = prevItem.strAddr;
        jsonRoot["prev"]["location"] = prevItem.strLocation;
        jsonRoot["prev"]["size"] = (uint64_t)(prevItem.iMeiTiSize/1024);
        jsonRoot["prev"]["duration"] = (uint64_t)prevItem.iDuration;
        jsonRoot["prev"]["commentshort"] = prevItem.strCommentShort;
    }

    string strRet = MediaParse::CJsonUtil::ToString(jsonRoot);
    return strRet;
}
string CBackupManager::GetBackupFoldItemFromItemID(int iItemID)
{
    CBackupTable table;
    BackupItemFull curItem = table.GetItem(iItemID);
    if(curItem.iID < 0)
    {
        return "";
    }
     
    nlohmann::json jsonRoot;
    jsonRoot["itemid"] = iItemID;
    jsonRoot["mtype"] = curItem.eMediaType;
    jsonRoot["fold"] = curItem.strFoldName;
    jsonRoot["file"] = curItem.strFile;
    jsonRoot["width"] = curItem.iWidth;
    jsonRoot["height"] = curItem.iHeight;
    jsonRoot["createtime"] = curItem.iCreateTimeSec;
    jsonRoot["weizhi"] = curItem.strAddr;
    jsonRoot["location"] = curItem.strLocation;
    jsonRoot["size"] = (uint64_t)(curItem.iMeiTiSize/1024);
    jsonRoot["duration"] = (uint64_t)curItem.iDuration;
    jsonRoot["commentshort"] = curItem.strCommentShort;
    string strRet = MediaParse::CJsonUtil::ToString(jsonRoot);
    return strRet;
}
BOOL CBackupManager::UploadItem(string strSrcFile, string strSrcThumbFile, string strDestFold, string& strToken)
{
    CBackupItemUpload* pBackupItemUpload = new CBackupItemUpload();
    BOOL bRet = pBackupItemUpload->UploadItem(strSrcFile, strSrcThumbFile, strDestFold);
    if(FALSE == bRet)
    {
        delete pBackupItemUpload;
        pBackupItemUpload = NULL;
        return FALSE;
    }
    BOOL bGet = FALSE;
    while (FALSE == bGet)
    {        
        strToken = CCommonUtil::StringFormat("%ld%03d", CCommonUtil::CurTimeMilSec(), CCommonUtil::GetRandomNum(1000));
        EnterCriticalSection(&m_Section);
        if(m_pBackupItemUploadMap.end() == m_pBackupItemUploadMap.find(strToken))
        {
            bGet = TRUE;
            m_pBackupItemUploadMap.insert(pair<string, CBackupItemUpload*>(strToken, pBackupItemUpload));
        }
        LeaveCriticalSection(&m_Section);
    }
    return bRet;
}

int CBackupManager::GetUploadPrecent(string strToken, char* pszErrorInfo)
{
    EnterCriticalSection(&m_Section);
    map<string, CBackupItemUpload*>::iterator itor = m_pBackupItemUploadMap.find(strToken);
    if(itor == m_pBackupItemUploadMap.end())
    {
        LeaveCriticalSection(&m_Section);
        strcpy(pszErrorInfo, "token error");
        return 0;
    }
    int iPrecent = itor->second->GetPrecent(pszErrorInfo);
    LeaveCriticalSection(&m_Section);
    return iPrecent;
}
list<string> CBackupManager::GetStoreFoldList()
{
    if(FALSE == IsSupportBackup())
    {
        list<string> retList;
        return retList;
    }
    CBackupTable table;
    return table.AllFolds();
}
void CBackupManager::ClearCache()
{
    EnterCriticalSection(&m_Section);
    map<string, CBackupItemUpload*>::iterator itor = m_pBackupItemUploadMap.begin();
    while (itor != m_pBackupItemUploadMap.end()) 
    {
        if (TRUE == itor->second->CheckExit()) 
        {
            delete itor->second;
             itor->second = NULL;
            itor = m_pBackupItemUploadMap.erase(itor);
        }
        else
        {
            ++itor;
        }
    }
    LeaveCriticalSection(&m_Section);
}
BOOL CBackupManager::IsSupportBackup()
{
    return m_bSupportBackup;
}