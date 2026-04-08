#pragma once
#include "stdafx.h"
typedef void(*MediaScanCallBack)(int iPrecent, LPVOID* lpParameter);
typedef struct MediaScanDriver
{
    
}MediaScanDriver;
class CMediaScan
{
private:
public:
    CMediaScan();
    ~CMediaScan();
    BOOL ScanThumb(MediaScanCallBack callBack, LPVOID* lpParameter);
    BOOL ScanOriginal(MediaScanCallBack callBack, LPVOID* lpParameter);
    
    void RunThumbRoot();
    void RunOriginalRoot();
    void Stop();
private:
    void StoreFile(const char* pszFile, BOOL bExtra);
    void ScanDir(const char* pszDir);
    BOOL IsNeedEnterDir(const char* pDir);
    static DWORD MediaScanThumbProc(LPVOID* lpParameter);
    static DWORD MediaScanOriginalProc(LPVOID* lpParameter);
private:
    string m_strRoot;
    BOOL m_bEnd;
    int m_iPrefxLen;
    list<string> m_FileList;
    MediaScanCallBack m_CallBack;
    LPVOID* m_lpParameter;
    HANDLE m_hScanThumb;
    HANDLE m_hScanOriginal;
};


