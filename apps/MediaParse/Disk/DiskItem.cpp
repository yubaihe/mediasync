#include "DiskItem.h"
#include <mntent.h>
#include <sys/statvfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <exiv2/exiv2.hpp>
#include <iostream>
#include "CommonUtil.h"
#include "Util/JsonUtil.h"
extern std::string g_strStoragePath; //存储路径 /home/relech/mediasync/media/.disk
extern std::string g_strMountPoint; //u盘路径 /mnt
CDiskItem::CDiskItem()
{
    m_strLabel = "";
    m_strUuid = "";
    m_strDev = "";
    m_iCapacity = 0;
    m_iFree = 0;
    m_iPrecent = 0;
    m_eStatus = DISKITEMSTATUS_NONE;
    InitializeCriticalSection(&m_Section);
    m_bExit = FALSE;
    m_hTimer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DiskItemProc, this, 0, NULL);
}

CDiskItem::~CDiskItem()
{
    printf("CDiskItem::~CDiskItem:%s\n", m_strDev.c_str());
    m_bExit = TRUE;
    if(NULL != m_hTimer)
    {
        WaitForSingleObject(m_hTimer, INFINITE);
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
    }
}
BOOL CDiskItem::Parse(const char* pszDevName)
{
    BOOL bNeedStart = TRUE;
    EnterCriticalSection(&m_Section);
    if(DISKITEMSTATUS_NONE != m_eStatus)
    {
        bNeedStart = FALSE;
    }
    LeaveCriticalSection(&m_Section);
    if(FALSE == bNeedStart)
    {
        return TRUE;
    }
    std::string strDir(g_strMountPoint);
    strDir.append(pszDevName);
    struct statvfs stat;
    if (statvfs(strDir.c_str(), &stat) != 0) 
    {
        return FALSE;
    }
    m_iCapacity = stat.f_frsize * stat.f_blocks;
    m_iFree = stat.f_frsize * stat.f_bfree;
    m_strDev = pszDevName;
    m_strLabel = CCommonUtil::ExecuteCmd("blkid -o value -s LABEL /dev/%s | tr -d '\n'", pszDevName);
    m_strUuid = CCommonUtil::ExecuteCmd("blkid -o value -s UUID /dev/%s  | tr -d '\n'", pszDevName);
    if(m_strLabel.length() == 0)
    {
        m_strLabel = m_strUuid;
    }
    EnterCriticalSection(&m_Section);
    m_eStatus = DISKITEMSTATUS_SYNC;
    LeaveCriticalSection(&m_Section);
    return TRUE;
}
std::string CDiskItem::GetDevInfo(const char* pszDevName)
{
    std::string strDir(g_strMountPoint);
    strDir.append(pszDevName);
    struct statvfs stat;
    if (statvfs(strDir.c_str(), &stat) != 0) 
    {
        return "";
    }
    uint64_t iCapacity = stat.f_frsize * stat.f_blocks;
    uint64_t iFree = stat.f_frsize * stat.f_bfree;
    std::string strDev = pszDevName;
    std::string strLabel = CCommonUtil::ExecuteCmd("blkid -o value -s LABEL /dev/%s | tr -d '\n'", pszDevName);
    std::string strUuid = CCommonUtil::ExecuteCmd("blkid -o value -s UUID /dev/%s  | tr -d '\n'", pszDevName);
    if(strLabel.length() == 0)
    {
        strLabel = pszDevName;
        std::transform(strLabel.begin(), strLabel.end(), strLabel.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    }
    int iBufferLen = strDev.length() + strLabel.length() + strUuid.length() + 100;
    char szBuffer[iBufferLen] = {0};
    sprintf(szBuffer, "{\"dev\":\"%s\",\"label\":\"%s\",\"uuid\":\"%s\",\"capacity\":%ld,\"free\":%ld}", 
            strDev.c_str(), strLabel.c_str(), strUuid.c_str(), iCapacity/1024, iFree/1024);
    return szBuffer;
}
void CDiskItem::Out()
{
    printf("Dev:%s\n", m_strDev.c_str());
    printf("Label:%s\n", m_strLabel.c_str());
    printf("Uuid:%s\n", m_strUuid.c_str());
    printf("Capacity:%ld\n", m_iCapacity/1024);
    printf("Free:%ld\n", m_iFree/1024);
}
DWORD CDiskItem::DiskItemProc(void* lpParameter)
{
    CDiskItem* pDiskItem = (CDiskItem*)lpParameter;
    HANDLE hSync = NULL;
    std::string strThumbRoot = "";
    string strDev = "";
    while (FALSE == pDiskItem->m_bExit)
    {
        Sleep(1000);
        if(strThumbRoot.length() == 0)
        {
            if(pDiskItem->m_strDev.length() != 0)
            {
                // /home/relech/mediasync/media/.disk/sda4/
                strThumbRoot = g_strStoragePath;
                strThumbRoot.append(pDiskItem->m_strDev);
                strThumbRoot.append("/");
                strDev = CCommonUtil::StringFormat("/dev/%s", pDiskItem->m_strDev.c_str());
            }
            else
            {
                continue;
            }
        }
        
 //       printf("CDiskItem::DiskItemProc:%d\n", pDiskItem->m_eStatus);
        switch(pDiskItem->m_eStatus)
        {
            case DISKITEMSTATUS_SYNC:
            {
                //需要同步文件目录
                EnterCriticalSection(&pDiskItem->m_Section);
                pDiskItem->m_eStatus = DISKITEMSTATUS_SYNCING;
                LeaveCriticalSection(&pDiskItem->m_Section);
                hSync = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MediaSyncProc, lpParameter, 0, NULL);
                break;
            }
            case DISKITEMSTATUS_NONE:
            case DISKITEMSTATUS_SYNCING:
            case DISKITEMSTATUS_SYNCED:
            {
//                printf("CDiskItem::DiskItemProc check:%d\n", pDiskItem->m_bExit);
                //已经同步完成了等待超时移除
                if(FALSE == pDiskItem->m_bExit)
                {
                    
                    if(FALSE == CCommonUtil::CheckFileExist(strDev.c_str()))
                    {
//                        printf("CDiskItem::DiskItemProc check status: umount %s Remove Tmp:%s\n", pDiskItem->m_strDev.c_str(), strThumbRoot.c_str());
                        //挂载点不存在了
                        CCommonUtil::RemoveFold(strThumbRoot.c_str());
                        pDiskItem->m_bExit = TRUE;
                    }
                }
                break;
            }
        }
    }
    if(NULL != hSync)
    {
        WaitForSingleObject(hSync, INFINITE);
        CloseHandle(hSync);
        hSync = NULL;
    }
    
    return 1;
}
void CDiskItem::OnInitPhotoCallBack(int iPrecent, LPVOID* lpParameter)
{
    CDiskItem* pDiskItem = (CDiskItem*)lpParameter;
    EnterCriticalSection(&pDiskItem->m_Section);
    pDiskItem->m_iPrecent = iPrecent;
    LeaveCriticalSection(&pDiskItem->m_Section);
    printf("Precent:%d\n", iPrecent);
}
DWORD CDiskItem::MediaSyncProc(void* lpParameter)
{
    CDiskItem* pDiskItem = (CDiskItem*)lpParameter;
    std::string strThumbRoot(g_strStoragePath);
    strThumbRoot.append(pDiskItem->m_strDev);
    strThumbRoot.append("/");
    std::string strOrigin(g_strMountPoint);
    strOrigin.append(pDiskItem->m_strDev);
    strOrigin.append("/");
    printf("Original Root:%s\n", strOrigin.c_str());
    printf("Thumb Root:%s\n", strThumbRoot.c_str());

    CCommonUtil::RemoveFold(strThumbRoot.c_str());
    CCommonUtil::CreateFold(strThumbRoot.c_str());

    pDiskItem->m_PhotoManager.Init(strOrigin.c_str(), strThumbRoot.c_str(), OnInitPhotoCallBack, (LPVOID*)lpParameter);
    printf("over\n");
    EnterCriticalSection(&pDiskItem->m_Section);
    pDiskItem->m_eStatus = DISKITEMSTATUS_SYNCED;
    LeaveCriticalSection(&pDiskItem->m_Section);
    pDiskItem->m_PhotoManager.Print();
    return 1;
}
void CDiskItem::Print()
{
    m_PhotoManager.Print();
}
int CDiskItem::GetPrecent()
{
    int iPrecent = 0;
    EnterCriticalSection(&m_Section);
    iPrecent = m_iPrecent;
    LeaveCriticalSection(&m_Section);
    return iPrecent;
}
list<string> CDiskItem::MediaItems(int iStart, int iLimit)
{
    return m_PhotoManager.GetItems(iStart, iLimit);
}
list<string> CDiskItem::MediaUploadItems(int iStart, int iLimit)
{
    return m_PhotoManager.GetItems2(iStart, iLimit);
}
void CDiskItem::AddIgnoreItem(int iItemID)
{
    m_PhotoManager.AddIgnoreItem(iItemID);
}
void CDiskItem::RemoveIgnoreItem(int iItemID)
{
    m_PhotoManager.RemoveIgnoreItem(iItemID);
}
list<string> CDiskItem::MediaIgnoreItems(int iStart, int iLimit)
{
    return m_PhotoManager.GetIgnoreItems(iStart, iLimit);
}
BOOL CDiskItem::UploadItem(int iItemID)
{
    //检查这个ItemID是否存在 设置回调函数
    std::string strThumb = m_PhotoManager.GetThumb(iItemID);
    std::string strFile = m_PhotoManager.GetFile(iItemID);
    if(FALSE == CCommonUtil::CheckFileExist(strThumb.c_str()) || 
        FALSE == CCommonUtil::CheckFileExist(strFile.c_str()))
    {
        printf("File not exist:\n%s\n%s\n", strThumb.c_str(), strFile.c_str());
        return FALSE;
    }
    return m_ItemUpload.UploadItem(iItemID, strThumb, strFile, this);
}
string CDiskItem::GetThumbFile(int iItemID)
{
    return m_PhotoManager.GetThumb(iItemID);
}
string CDiskItem::GetOriginalFile(int iItemID)
{
    return m_PhotoManager.GetFile(iItemID);
}
PhotoItem CDiskItem::GetItemByItemIndex(int iItemIndex)
{
    return m_PhotoManager.GetItemByItemIndex(iItemIndex);
}
PhotoExtra CDiskItem::GetItemExtraByItemIndex(int iItemIndex)
{
    return m_PhotoManager.GetExtraByItemIndex(iItemIndex);
}
string CDiskItem::GetCreateDate(int iItemIndex)
{
    return m_PhotoManager.GetCreateDate(iItemIndex);
}
 string CDiskItem::GetAddr(int iItemIndex)
{
    vector<string> vec = m_PhotoManager.GetAddr(iItemIndex);
    string strRet(vec[0]);
    strRet.append("&");
    strRet.append(vec[1]);
    return strRet;
}
string CDiskItem::GetDiskLabel()
{
    return m_strLabel;
}
int CDiskItem::GetDuration(int iItemIndex)
{
    return m_PhotoManager.GetDuration(iItemIndex);
}
void CDiskItem::GetPrecent(int* piItemID, int* piPrecent, char* pszErrorInfo)
{
    int iPrecent = m_ItemUpload.GetPrecent(piItemID, pszErrorInfo);
    *piPrecent = iPrecent;
}
string CDiskItem::GetDiskItemDetail(int iItemID, BOOL bIgnore)
{
    int iNextItemIndex = m_PhotoManager.GetNextVisiableIndex(iItemID, bIgnore);
    int iPrevItemIndex = m_PhotoManager.GetPrevVisiableIndex(iItemID, bIgnore);
    PhotoItem item = GetItemByItemIndex(iItemID);
    PhotoExtra itemExtra = GetItemExtraByItemIndex(iItemID);
    nlohmann::json jsonRoot;
    jsonRoot["cur"]["itemid"] = iItemID;
    jsonRoot["cur"]["mtype"] = item.iMediaType;
    jsonRoot["cur"]["file"] = itemExtra.strFileName;
    jsonRoot["cur"]["width"] = item.iWidth;
    jsonRoot["cur"]["height"] = item.iHeight;
    jsonRoot["cur"]["createtime"] = GetCreateDate(iItemID);
    jsonRoot["cur"]["weizhi"] = GetAddr(iItemID);
    jsonRoot["cur"]["location"] = "";
    jsonRoot["cur"]["size"] = (uint64_t)(item.iFileSize/1024);
    jsonRoot["cur"]["duration"] = (uint64_t)itemExtra.iDuration;
    jsonRoot["cur"]["ignore"] = m_PhotoManager.IsIgnore(iItemID);

    item = GetItemByItemIndex(iNextItemIndex);
    itemExtra = GetItemExtraByItemIndex(iNextItemIndex);
    if(item.strName.length() > 0)
    {
        jsonRoot["next"]["itemid"] = iNextItemIndex;
        jsonRoot["next"]["mtype"] = item.iMediaType;
        jsonRoot["next"]["file"] = itemExtra.strFileName;
        jsonRoot["next"]["width"] = item.iWidth;
        jsonRoot["next"]["height"] = item.iHeight;
        jsonRoot["next"]["createtime"] = GetCreateDate(iNextItemIndex);
        jsonRoot["next"]["weizhi"] = GetAddr(iNextItemIndex);
        jsonRoot["next"]["location"] = "";
        jsonRoot["next"]["size"] = (uint64_t)(item.iFileSize/1024);
        jsonRoot["next"]["duration"] = (uint64_t)itemExtra.iDuration;
        jsonRoot["next"]["ignore"] = m_PhotoManager.IsIgnore(iNextItemIndex);
    }
    item = GetItemByItemIndex(iPrevItemIndex);
    itemExtra = GetItemExtraByItemIndex(iPrevItemIndex);
    if(item.strName.length() > 0)
    {
        jsonRoot["prev"]["itemid"] = iPrevItemIndex;
        jsonRoot["prev"]["mtype"] = item.iMediaType;
        jsonRoot["prev"]["file"] = itemExtra.strFileName;
        jsonRoot["prev"]["width"] = item.iWidth;
        jsonRoot["prev"]["height"] = item.iHeight;
        jsonRoot["prev"]["createtime"] = GetCreateDate(iPrevItemIndex);
        jsonRoot["prev"]["weizhi"] = GetAddr(iPrevItemIndex);
        jsonRoot["prev"]["location"] = "";
        jsonRoot["prev"]["size"] = (uint64_t)(item.iFileSize/1024);
        jsonRoot["prev"]["duration"] = (uint64_t)itemExtra.iDuration;
        jsonRoot["prev"]["ignore"] = m_PhotoManager.IsIgnore(iPrevItemIndex);
    }
    string strRet = MediaParse::CJsonUtil::ToString(jsonRoot);
    return strRet;
}
void CDiskItem::DeleteItem(int iItemIndex)
{
    std::string strThumb = m_PhotoManager.GetThumb(iItemIndex);
    std::string strFile = m_PhotoManager.GetFile(iItemIndex);
    m_PhotoManager.RemoveItem(iItemIndex);
    CCommonUtil::RemoveFile(strThumb.c_str());
    CCommonUtil::RemoveFile(strFile.c_str());
}

void CDiskItem::PrintItem(int iItemIndex)
{
    std::string strFile = m_PhotoManager.GetFile(iItemIndex);
    CImageParse imageParse;
    imageParse.Parse(strFile.c_str(), TRUE);
    imageParse.Out();
    try {
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(strFile.c_str());
        image->readMetadata();
 
        uint32_t width = image->exifData()["Exif.Photo.PixelXDimension"].toLong();
        uint32_t height = image->exifData()["Exif.Photo.PixelYDimension"].toLong();
        printf("Width:%d\tHeight:%d\n", width, height);
 
    } catch (const Exiv2::Error& e) {
        printf("Exiv2 exception:%s\n", e.what());
    }
    string strRet = CCommonUtil::ExecuteCmd("ffprobe -v error -select_streams v:0 -show_entries format=width,height \"%s\"", strFile.c_str());
    printf("ffprobe\n%s\n", strRet.c_str());
}
BOOL CDiskItem::IsExit()
{
    //printf("CDiskItem::IsExit:%s :%d\n", m_strDev.c_str(), m_bExit);
    return m_bExit;
}
size_t CDiskItem::GetIgnoreCount()
{
    return m_PhotoManager.GetIgnoreItemCount();
}