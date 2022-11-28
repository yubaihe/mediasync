#ifndef MEDIASCANDRIVER_H_
#define MEDIASCANDRIVER_H_
#include "stdafx.h"
#include "RelechQueue.h"
typedef void(*MediaScanCallBack)(int iPrecent, LPVOID* lpParameter);
typedef struct MediaScanDriver
{
    int iPrefxLen;
    RelechQueue* pFileQueue;
    MediaScanCallBack callBack;
    LPVOID* lpParameter;
    HANDLE hScanHandle;
}MediaScanDriver;

MediaScanDriver* MediaScanDriver_Start(MediaScanCallBack callBack, LPVOID* lpParameter);
void MediaScanDriver_Stop(MediaScanDriver* pDriver);
void MediaScanDriver_Run(MediaScanDriver* pDriver);
#endif