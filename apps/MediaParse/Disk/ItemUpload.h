#pragma once
#include "../stdafx.h"
class CDiskItem;
class CItemUpload
{
public:
    CItemUpload();
    ~CItemUpload();
    BOOL UploadItem(int iItemID, string strThumb, string strFile, CDiskItem* pDiskItem);
    int GetPrecent(int* piItemID, char* pszErrorInfo);
private:
    void Reset();
    static DWORD UploadItemProc(void* lpParameter);
    void Upload();
    BOOL CopyFile(const char* pszSrc, const char* pszDest);
private:
    size_t m_iThumbSize;
    size_t m_iOrigianlSize;
    size_t m_iTransSize;
    size_t m_iTotalSize;
    string m_strErrorInfo;

    int m_iItemID;
    BOOL m_bExit;
    string m_strThumbFile;
    string m_strOriginalFile;
    CDiskItem* m_pDiskItem;
    HANDLE m_hUpload;
    
};


