#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include "DiskManager.h"
#include "Dbus/libdbus.h"
#include "MessageHandler.h"
#include "VideoParse.h"
#include "ImageParse.h"
#include "CommonUtil.h"
#include "./Util/CommonSdkUtil.h"
#include "JsonUtil.h"
#include "DbusUtil.h"
#include "CpuDetect.h"
#include "Util/FileUtil.h"
#include "BackupManager.h"
#include "SyncToDeviceManager.h"
#include "BackupTmpFoldMonitor.h"
#include "TranscodeManager.h"
void* g_pDbusContext = NULL;
BOOL g_bExit = TRUE;
CMessageHandler g_MessageHandler;
string g_strStoragePath;
string g_strMountPoint;
string g_strBackupPoint;
string g_strTempPath;
string g_strBackThumbPath;
void CommonSdk_WriteLog(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszLog)
{
    printf("%s\n", pszLog);
}
void MediaParse_OnDbusMessageHandler(DbusContext* pContext, DBusMessage* pDBusMessage, int iCommandID, const char* pszMessage, int iMessageLen)
{
    printf("%s\n", pszMessage);
//    printf("command id:%d\n", iCommandID);
    char szBuffer[MAX_MESSAGE_LEN] = {0};
    g_MessageHandler.OnMessage(pszMessage, szBuffer);
    printf("%s\n", szBuffer);
    LibDbus_Reply(pContext, pDBusMessage, iCommandID, (char*)szBuffer, strlen(szBuffer) + 1);
}
void OnSystemSig(int iSigNo) 
{
	printf("OnSystemSig\n");
	g_bExit = TRUE;
}

void OnSignalHandler(int nSigno)
{
	printf("OnSignalHandler\n");
}
void InitEnv()
{
    StoreInfo storeInfo = {};
    while (storeInfo.strStorage.length() == 0)
    {
        if(TRUE == g_bExit)
        {
            return ;
        }
        storeInfo = CDbusUtil::GetStoreInfo();
        if(storeInfo.strStorage.length() == 0)
        {
            printf("Get store info failed:%s\n", storeInfo.strStorage.c_str());
            Sleep(1000);
            continue;
        }
    }
    if(storeInfo.strStorage[storeInfo.strStorage.length() - 1] != '/')
    {
        storeInfo.strStorage.append("/");
    }
    g_strStoragePath = storeInfo.strStorage;
    g_strStoragePath.append(".disk/");
   
    if(storeInfo.strTempAddr[storeInfo.strTempAddr.length() - 1] != '/')
    {
        storeInfo.strTempAddr.append("/");
    }
    g_strTempPath = storeInfo.strTempAddr;
    printf("Storage:%s\n", g_strStoragePath.c_str());
    printf("Mount:%s\n", g_strMountPoint.c_str());
    printf("Temp:%s\n", g_strTempPath.c_str());

    CCommonUtil::RemoveFold(g_strStoragePath.c_str());
    CCommonUtil::CreateFold(g_strStoragePath.c_str());

}
void RemoveEmptyMountNode()
{
    if(g_strMountPoint.length() > 0)
    {
        //将磁盘重新挂载起来
        list<string> foldList = CFileUtil::GetSubFolds(g_strMountPoint.c_str());
        for(list<string>::iterator itor = foldList.begin(); itor != foldList.end(); ++itor)
        {
            string strNodeName = CCommonUtil::StringFormat("/dev/%s", itor->c_str());
            if(FALSE == CCommonUtil::CheckFileExist(strNodeName.c_str()))
            {
                string strFold = g_strMountPoint;
                strFold.append(itor->c_str());
                //不强制删除有文件的目录
                CCommonUtil::RemoveFold(strFold.c_str(), FALSE);
            }
        }
    }
}
//./MediaParse /home/relech/mediasync/external/ /home/relech/mediasync/backup/ 

int main(int iArgc, char *pszArgsV[])
{
    int iCpuCount = CCpuDetect::GetInstance()->GetCpuCount();
    int iCpuUage = CCpuDetect::GetInstance()->GetUsage();
    int iMemUsage = CCpuDetect::GetInstance()->GetMemUsage();
    printf("Cpu count:%d\tCpu usage:%d\tMem usage:%d\n", iCpuCount, iCpuUage, iMemUsage);
    CDiskManager::GetInstance();
    CBackupTmpFoldMonitor::GetInstance();

    signal(SIGINT, OnSystemSig); 
	signal(SIGPIPE,  OnSignalHandler);
    g_pDbusContext = LibDbus_Init(DBUS_MEDIAPARSE, 10, MediaParse_OnDbusMessageHandler, NULL);
    g_bExit = FALSE;
    if(iArgc >= 2)
    {
        printf("%s\n", pszArgsV[1]);
        g_strMountPoint = pszArgsV[1];
        if(g_strMountPoint[g_strMountPoint.length() - 1] != '/')
        {
            g_strMountPoint.append("/");
        }
        InitEnv();
    }
    if(iArgc >= 3)
    {
        
        g_strBackupPoint = pszArgsV[2];
        if(g_strBackupPoint[g_strBackupPoint.length() - 1] != '/')
        {
            g_strBackupPoint.append("/");
        }
    
        CCommonUtil::CreateFold(g_strBackupPoint.c_str());
        g_strBackThumbPath = g_strBackupPoint;
        g_strBackThumbPath.append(".media/");
        CCommonUtil::CreateFold(g_strBackThumbPath.c_str());
        printf("backup:%s\n", pszArgsV[2]);
        printf("backupthumb:%s\n", g_strBackThumbPath.c_str());
        CBackupManager::GetInstance()->Init(g_strBackupPoint);
    }
    if(g_strMountPoint.length() > 0)
    {
        //将磁盘重新挂载起来
        list<string> foldList = CFileUtil::GetSubFolds(g_strMountPoint.c_str());
        for(list<string>::iterator itor = foldList.begin(); itor != foldList.end(); ++itor)
        {
            string strNodeName = CCommonUtil::StringFormat("/dev/%s", itor->c_str());
            if(TRUE == CCommonUtil::CheckFileExist(strNodeName.c_str()))
            {
                CCommonUtil::ExecuteCmdWithOutReplay("mount /dev/%s %s%s", itor->c_str(), g_strMountPoint.c_str(), itor->c_str());
            }
            else
            {
                printf("%s not exist\n", strNodeName.c_str());
            }
        }
    }
    while (FALSE == g_bExit)
    {
        Sleep(3000);
        if(g_strMountPoint.length() > 0)
        {
            CDiskManager::GetInstance()->DevCheck();
            //将没有挂载的目录删除掉
            //应该放到系统启动的脚本去删除
            //RemoveEmptyMountNode();
            //备份目录清除缓存
            CBackupManager::GetInstance()->ClearCache();
            //同步到设备检查
            CSyncToDeviceManager::GetInstance()->ClearCache();
            //解码模块定时清理
            CTranscodeManager::GetInstance()->ClearCache();
        }
    }
    
    LibDbus_UnInit(g_pDbusContext);
    CDiskManager::Release();
    CCpuDetect::Release();
    CBackupManager::Release();
    CSyncToDeviceManager::Release();
    CBackupTmpFoldMonitor::Release();
   // stty 命令来重置终端设置
   system("stty sane");

    return 1;
}