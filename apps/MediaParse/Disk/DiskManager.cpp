#include "DiskManager.h"
#include "CommonUtil.h"
#include <mntent.h>
#include <sys/statvfs.h>
#include "exiv2/image.hpp"
extern std::string g_strMountPoint;
CDiskManager* CDiskManager::m_pInstance = NULL;
CDiskManager::CDiskManager()
{
    InitializeCriticalSection(&m_Section);
}

CDiskManager::~CDiskManager()
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.begin();
    for(; itor != m_pDiskItemMap.end();)
    {
        CDiskItem* pDiskItem = itor->second;
        delete pDiskItem;
        m_pDiskItemMap.erase(itor++);
    }
    LeaveCriticalSection(&m_Section);
    DeleteCriticalSection(&m_Section);
    Exiv2::XmpParser::terminate();
}
CDiskManager* CDiskManager::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CDiskManager();
    }
    return m_pInstance;
}
void CDiskManager::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
string CDiskManager::GetDiskRoot(const char* pszDevName)
{
    std::string strOriginRoot(g_strMountPoint);
    strOriginRoot.append(pszDevName);
    strOriginRoot.append("/");
    return strOriginRoot;
}
std::list<std::string> CDiskManager::GetDiskNames(const char* pszPrefix)
{
    std::list<std::string> retList;
    size_t iPrefixLen = strlen(pszPrefix);
    if(0 == iPrefixLen)
    {
        return retList;
    }
    
    FILE* pFile = NULL;
    pFile = setmntent("/etc/mtab", "r");
    if(NULL == pFile)
    {
        return retList;
    }
    struct mntent* pMntent = NULL;
    while ((pMntent = getmntent(pFile)) != NULL) 
    {
        printf("mnt_dir:%s   mnt_type:%s\n", pMntent->mnt_dir, pMntent->mnt_type);
        if (pMntent->mnt_type == NULL)
        {
            continue;
        }
        if(strlen(pMntent->mnt_dir) < iPrefixLen || 0 != strncmp(pMntent->mnt_dir, pszPrefix, iPrefixLen))
        {
            continue;
        }
        struct statvfs stat;
        if (statvfs(pMntent->mnt_dir, &stat) != 0) 
        {
            continue;
        }
        retList.push_back(pMntent->mnt_dir + iPrefixLen);
    }
    endmntent(pFile);
    return retList;
}
BOOL CDiskManager::EnterDev(const char* pszDevName)
{
    BOOL bRet = TRUE;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    CDiskItem* pDiskItem = NULL;
    if(itor == m_pDiskItemMap.end())
    {
        pDiskItem = new CDiskItem();
        bRet = pDiskItem->Parse(pszDevName);
        if(TRUE == bRet)
        {
            m_pDiskItemMap.insert(std::pair<std::string, CDiskItem*>(pszDevName, pDiskItem));
        }
        else
        {
            delete pDiskItem;
            pDiskItem = NULL;
        }
    }
    else
    {
        pDiskItem = itor->second;
    }
    LeaveCriticalSection(&m_Section);
    return bRet;
}
int CDiskManager::GetPrecentDev(const char* pszDevName)
{
    int iPrecent = 0;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        iPrecent = pDiskItem->GetPrecent();
    }
    LeaveCriticalSection(&m_Section);
    return iPrecent;
}
std::string CDiskManager::ListDev()
{
    std::list<std::string> itemList = GetDiskNames(g_strMountPoint.c_str());
    string strRet = "[";
    std::list<std::string>::iterator itor = itemList.begin();
    for (; itor != itemList.end(); ++itor)
    {
        string strDevDetail = CDiskItem::GetDevInfo((*itor).c_str());
        if(strDevDetail.length() > 0)
        {
            strRet.append(strDevDetail);
            strRet.append(",");
        }
    }
    if(strRet.length() > 1)
    {
        strRet[strRet.length() - 1] = ']';
    }
    else
    {
        strRet.append("]");
    }
    return strRet;
}
list<string> CDiskManager::MediaItems(const char* pszDevName, int iStart, int iLimit)
{
    list<string> retList;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        retList = pDiskItem->MediaItems(iStart, iLimit);
    }
    LeaveCriticalSection(&m_Section);
    return retList;
}
list<string> CDiskManager::MediaUploadItems(const char* pszDevName, int iStart, int iLimit)
{
    list<string> retList;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        retList = pDiskItem->MediaUploadItems(iStart, iLimit);
    }
    LeaveCriticalSection(&m_Section);
    return retList;
}
list<string> CDiskManager::MediaIgnoreItems(const char* pszDevName, int iStart, int iLimit)
{
    list<string> retList;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        retList = pDiskItem->MediaIgnoreItems(iStart, iLimit);
    }
    LeaveCriticalSection(&m_Section);
    return retList;
}

void CDiskManager::SetItemsIgnore(const char* pszDevName, const char* pszItemIds)
{
    std::vector<std::string> vec = CCommonUtil::StringSplit(pszItemIds, "&");
    if(vec.size() == 0)
    {
        return;
    }
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        for(size_t i = 0; i < vec.size(); ++i)
        {
            pDiskItem->AddIgnoreItem(atoi(vec[i].c_str()));
        }
    }
    LeaveCriticalSection(&m_Section);
}
void CDiskManager::RemoveIgnoreItem(const char* pszDevName, const char* pszItemIds)
{
    std::vector<std::string> vec = CCommonUtil::StringSplit(pszItemIds, "&");
    if(vec.size() == 0)
    {
        return;
    }
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        for(size_t i = 0; i < vec.size(); ++i)
        {
            pDiskItem->RemoveIgnoreItem(atoi(vec[i].c_str()));
        }
    }
    LeaveCriticalSection(&m_Section);
}
BOOL CDiskManager::UploadItem(const char* pszDevName, int iItemID)
{
    BOOL bRet = TRUE;
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        bRet = pDiskItem->UploadItem(iItemID);
    }
    LeaveCriticalSection(&m_Section);
    return bRet;
}
void CDiskManager::GetUploadInfo(const char* pszDevName, int* piItemID, int* piPrecent, char* pszError)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        pDiskItem->GetPrecent(piItemID, piPrecent, pszError);
    }
    LeaveCriticalSection(&m_Section);
}
string CDiskManager::GetDiskItemDetail(const char* pszDevName, int iItemID, BOOL bIgnore)
{
    string strDetail = "";
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        strDetail = pDiskItem->GetDiskItemDetail(iItemID, bIgnore);
    }
    LeaveCriticalSection(&m_Section);
    return strDetail;
}
void CDiskManager::AddIgnoreItem(const char* pszDevName, int iItemID)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        pDiskItem->AddIgnoreItem(iItemID);
    }
    LeaveCriticalSection(&m_Section);
}
void CDiskManager::RemoveIgnoreItem(const char* pszDevName, int iItemID)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        pDiskItem->RemoveIgnoreItem(iItemID);
    }
    LeaveCriticalSection(&m_Section);
}
void CDiskManager::DeleteItem(const char* pszDevName, int iItemID)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        pDiskItem->DeleteItem(iItemID);
    }
    LeaveCriticalSection(&m_Section);
}
void CDiskManager::PrintItem(const char* pszDevName, int iItemIndex)
{
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        pDiskItem->PrintItem(iItemIndex);
    }
    LeaveCriticalSection(&m_Section);
}
void CDiskManager::DevCheck()
{
//    printf("CDiskManager::DevCheck\n");
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.begin();
    for(;itor != m_pDiskItemMap.end();)
    {
        if(TRUE == itor->second->IsExit())
        {
            CDiskItem* pDiskItem = itor->second;
            itor = m_pDiskItemMap.erase(itor);
            delete pDiskItem;
            pDiskItem = NULL;
        }
        else
        {
            itor++;
        }
    }
    LeaveCriticalSection(&m_Section);
}
string CDiskManager::GetThumbFile(const char* pszDevName, int iItemID)
{
    string strRet("");
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        strRet = pDiskItem->GetThumbFile(iItemID);
    }
    LeaveCriticalSection(&m_Section);
    return strRet;
}
string CDiskManager::GetOriginalFile(const char* pszDevName, int iItemID)
{
    string strRet("");
    EnterCriticalSection(&m_Section);
    std::map<std::string, CDiskItem*>::iterator itor = m_pDiskItemMap.find(pszDevName);
    if(itor != m_pDiskItemMap.end())
    {
        CDiskItem* pDiskItem = itor->second;
        strRet = pDiskItem->GetOriginalFile(iItemID);
    }
    LeaveCriticalSection(&m_Section);
    return strRet;
}