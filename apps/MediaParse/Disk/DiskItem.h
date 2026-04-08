#pragma once
#include "stdafx.h"
#include "MediaDefine.h"
#include <list>
#include <map>
#include "MediaDefine.h"
#include "ImageParse.h"
#include "VideoParse.h"
#include "PhotoManager.h"
#include "ItemUpload.h"
typedef enum
{
    DISKITEMSTATUS_NONE = 0,
    DISKITEMSTATUS_SYNC,
    DISKITEMSTATUS_SYNCING,
    DISKITEMSTATUS_SYNCED,
}DISKITEMSTATUS;

class CDiskItem
{
public:
    CDiskItem();
    ~CDiskItem();
    BOOL Parse(const char* pszDevName);
    void Out();
    int GetPrecent();
    static std::string GetDevInfo(const char* pszDevName);
    void Print();
    list<string> MediaItems(int iStart, int iLimit);
    list<string> MediaUploadItems(int iStart, int iLimit);
    list<string> MediaIgnoreItems(int iStart, int iLimit);
    
    void AddIgnoreItem(int iItemID);
    void RemoveIgnoreItem(int iItemID);
    BOOL UploadItem(int iItemID);
    string GetThumbFile(int iItemID);
    string GetOriginalFile(int iItemID);
    PhotoItem GetItemByItemIndex(int iItemIndex);
    PhotoExtra GetItemExtraByItemIndex(int iItemIndex);
    string GetCreateDate(int iItemIndex);
    string GetAddr(int iItemIndex);
    int GetDuration(int iItemIndex);
    string GetDiskLabel();
    void GetPrecent(int* piItemID, int* piPrecent, char* pszErrorInfo);
    string GetDiskItemDetail(int iItemID, BOOL bIgnore);
    void DeleteItem(int iItemIndex);
    void PrintItem(int iItemIndex);
    BOOL IsExit();
    size_t GetIgnoreCount();
private:
    static DWORD DiskItemProc(void* lpParameter);
    static DWORD MediaSyncProc(void* lpParameter);
    static void OnInitPhotoCallBack(int iPrecent, LPVOID* lpParameter);
private:
    std::string m_strLabel; //CCSA_X64FRE
    std::string m_strUuid; //DE00-85B8
    std::string m_strDev; //sda4
    size_t m_iCapacity;
    size_t m_iFree;
    BOOL m_bExit;
    DISKITEMSTATUS m_eStatus;
    HANDLE m_hTimer;
    CRITICAL_SECTION m_Section;
    int m_iPrecent;
    CItemUpload m_ItemUpload;
    
    CPhotoManager m_PhotoManager;
};


