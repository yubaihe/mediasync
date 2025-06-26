#pragma once
#include "stdafx.h"
#include <list>
#include "DiskItem.h"
class CDiskManager
{
public:
    CDiskManager();
    ~CDiskManager();
    static CDiskManager* GetInstance();
    static void Release();
    static string GetDiskRoot(const char* pszDevName);
    BOOL EnterDev(const char* pszDevName);
    int GetPrecentDev(const char* pszDevName);
    std::string ListDev();
    list<string> MediaItems(const char* pszDevName, int iStart, int iLimit);
    list<string> MediaUploadItems(const char* pszDevName, int iStart, int iLimit);
    list<string> MediaIgnoreItems(const char* pszDevName, int iStart, int iLimit);
    void SetItemsIgnore(const char* pszDevName, const char* pszItemIds);
    void RemoveIgnoreItem(const char* pszDevName, const char* pszItemIds);
    BOOL UploadItem(const char* pszDevName, int iItemID);
    void GetUploadInfo(const char* pszDevName, int* piItemID, int* piPrecent, char* pszError);
    string GetDiskItemDetail(const char* pszDevName, int iItemID, BOOL bIgnore);
    void AddIgnoreItem(const char* pszDevName, int iItemID);
    void RemoveIgnoreItem(const char* pszDevName, int iItemID);
    void DeleteItem(const char* pszDevName, int iItemID);
    void PrintItem(const char* pszDevName, int iItemIndex);
    void DevCheck();
    string GetThumbFile(const char* pszDevName, int iItemID);
    string GetOriginalFile(const char* pszDevName, int iItemID);
private:
    std::list<std::string> GetDiskNames(const char* pszPrefix);
private:
    CRITICAL_SECTION m_Section;
    std::map<std::string, CDiskItem*> m_pDiskItemMap;
    static CDiskManager* m_pInstance;
};

