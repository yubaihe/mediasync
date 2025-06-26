#include "BackupFlod.h"
#include "../Util/CommonUtil.h"
#include "../Util/JsonUtil.h"
#include "../Util/FileUtil.h"
#include "BackupManager.h"
#include "../DataBase/BackupDb.h"
CBackupFlod::CBackupFlod()
{
}
CBackupFlod::CBackupFlod(string strFolderName, BOOL bScanFile/* = TRUE*/)
{
    m_strFoldName = strFolderName;
    string strBackupRoot = GetBackupRoot();
    if(TRUE == bScanFile)
    {
        m_ScanFileVec = AllFilesVec(strBackupRoot);
        std::sort(m_ScanFileVec.begin(), m_ScanFileVec.end(), CompareFileWithCreatTime);
    }
}
CBackupFlod::~CBackupFlod()
{
   
}
bool CBackupFlod::CompareFileWithCreatTime(const FileEntry& a, const FileEntry& b) 
{
    return a.iCreateTimeSec > b.iCreateTimeSec;
}
string CBackupFlod::GetBackupRoot()
{
    string strBackFold = CCommonUtil::StringFormat("%s%s/", g_strBackupPoint.c_str(), m_strFoldName.c_str());
    return strBackFold;
}
string CBackupFlod::GetBackupThumbRoot()
{
    string strBackTmp = CCommonUtil::StringFormat("%s%s/", g_strBackThumbPath.c_str(), m_strFoldName.c_str());
    return strBackTmp;
}
string CBackupFlod::GetThumbFile(MEDIATYPE eMediaType, string strFile)
{
    string strFilePrefx(""), strPostfix("");
    BOOL bRet = CCommonUtil::SpliteFile(strFile, strFilePrefx, strPostfix);
    if(FALSE == bRet)
    {
        strFilePrefx = strFile;
        switch (eMediaType)
        {
            case MEDIATYPE_IMAGE:
            {
                strPostfix = "jpg";
                break;
            }
            case MEDIATYPE_VIDEO:
            {
                strPostfix = "mp4";
                break;
            }
            default:
            {
                break;
            }
        }
    }
    return CCommonUtil::StringFormat("%s_%s.jpg", strFilePrefx.c_str(), strPostfix.c_str());
}
BOOL CBackupFlod::RemoveItem(int iItemID)
{    
    //删除数据库
    CBackupTable table;
    BackupItemFull item = table.GetItem(iItemID);
    if(item.iID < 0)
    {
        return FALSE;
    }
    m_strFoldName = item.strFoldName;
    string strFileName = item.strFile;
    string strThumbFileName = GetThumbFile(item.eMediaType, strFileName);
    table.RemoveItem(iItemID);
    
    //删除原始文件和缩略图
    string strFileRoot = GetBackupRoot();
    string strThumbRoot = GetBackupThumbRoot();
    string strFile = CCommonUtil::StringFormat("%s%s", strFileRoot.c_str(), strFileName.c_str());
    string strThumbFile = CCommonUtil::StringFormat("%s%s", strThumbRoot.c_str(), strThumbFileName.c_str());
    CCommonUtil::RemoveFile(strFile.c_str());
    CCommonUtil::RemoveFile(strThumbFile.c_str());
    printf("Remove item file: %s\n", strFile.c_str());
    printf("Remove item file: %s\n", strThumbFile.c_str());
    //删除缓存
    for(size_t i = 0; i < m_ScanFileVec.size(); ++i)
    {
        if(0 == m_ScanFileVec[i].strName.compare(item.strFile))
        {
            m_ScanFileVec.erase(m_ScanFileVec.begin() + i);
            break;
        }
    }
    return TRUE;
}
std::vector<FileEntry> CBackupFlod::AllFilesVec(string strPath)
{
    std::vector<FileEntry> retVec;
    //string strPath = GetBackupRoot();
    DIR* pDir = opendir(strPath.c_str());
    if (NULL == pDir)
    {
        printf("无法打开目录: %s\n", strPath.c_str());
        return retVec;
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
        if (FALSE == S_ISDIR(pathStat.st_mode)) 
        {
            FileEntry entry = {};
            entry.strName = pDirent->d_name;
            entry.iCreateTimeSec = pathStat.st_ctime;
            retVec.push_back(entry);
        }
    }
    closedir(pDir);
    return retVec;
}
map<string, time_t> CBackupFlod::AllFilesMap(string strPath)
{
    map<string, time_t> retMap;
    //string strPath = GetBackupRoot();
    DIR* pDir = opendir(strPath.c_str());
    if (NULL == pDir)
    {
        printf("无法打开目录: %s\n", strPath.c_str());
        return retMap;
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
        if (FALSE == S_ISDIR(pathStat.st_mode)) 
        {
            FileEntry entry = {};
            entry.strName = pDirent->d_name;
            entry.iCreateTimeSec = pathStat.st_ctime;
            retMap.insert(pair<string, time_t>(entry.strName, entry.iCreateTimeSec));
        }
    }
    closedir(pDir);
    return retMap;
}