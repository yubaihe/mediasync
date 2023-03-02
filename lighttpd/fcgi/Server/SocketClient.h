//
//  SocketClient.h
//  MediaSyncNetWork
//
//  Created by 俞力奇 on 2020/3/14.
//  Copyright © 2020 俞力奇. All rights reserved.
//

#ifndef SocketClient_h
#define SocketClient_h
#include <stdio.h>
#include "stdafx.h"
typedef enum
{
    PROGRESS_FAILED = -1,
    PROGRESS_SUCCESS,
    PROGRESS_UPLOADING
}EUPLOADSTATUS;
typedef void (*OnProgressCallBack)(long iTag, uint64_t iSendLen, EUPLOADSTATUS iStatus);
//void OnProgress(uint64_t iSendLen, EUPLOADSTATUS iStatus);

BOOL SocketClient_SendFile(long iTag, const char* pIp, int iPort, const char* pFileName, OnProgressCallBack callBack);
//返回值需要手动释放
char* SocketClient_SendMessage(const char* pIp, int iPort, const char* pstrMessage);
#endif /* SocketClient_h */
