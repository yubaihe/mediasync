#include "stdafx.h"
#include "DataBaseDriver.h"
#include "FileUtil.h"
//#include "test.h"
#include "ConnectBroadCast.h"
#include "ConnectServer.h"
#include "ConnectServerMessage.h"
#include "ConnectBroadCastMessage.h"
#include "DataBaseDevice.h"
#include "HttpFileServer.h"
#include "MediaScanDriver.h"
#include "NetWorkDriver.h"
#include "ClearCache.h"
//ntp自动更新
//timedatectl status    查询状态
//timedatectl set-ntp no   　　     关闭
//timedatectl set-ntp yes           开启
// ln -s /home/relech/share/media /home/relech/share/lighttpd/www/
// ll /proc/19820/fd | wc -l 
//由于添加了修改时间指令 需要以sodu身份运行
//sudo valgrind --tool=memcheck  --leak-check=full --show-leak-kinds=all ./Server
struct ConnectBroadCast* g_pConnectBroadCast = NULL;
struct ConnectServer* g_pServer = NULL;
struct HttpFileServer* g_HttpFileServer = NULL;
MediaScanDriver* g_pMediaScanDriver = NULL;
NetWorkDriver* g_pNetWorkDriver = NULL;
struct ClearCache* g_pClearCache = NULL;

BOOL g_exit;
int SERVERPORT = 0;
int CASTPORT = 9082;
char FOLD_PREFIX[255] = {0};
char FOLDTHUMB_PREFIX[255] = {0};
char FOLDEX_PREFIX[255] = {0};
char TMPFILEPATH[255] = {0};
char TODIR[255] = {0};
BOOL g_bSamba = FALSE;
BOOL g_bEth = FALSE;
BOOL g_bWlan = FALSE;
BOOL g_bSupportGpsGet = TRUE;
void OnSystemSig(int iSigNo) 
{
	printf("OnSystemSig\n");
	g_exit = TRUE;
}

void OnSignalHandler(int nSigno)
{
	printf("OnSignalHandler\n");
}

void Exit()
{
	if(NULL != g_pConnectBroadCast)
	{
		ConnectBroadCast_Stop(g_pConnectBroadCast);
	}
    printf("BroadCast stop\n");
	if(NULL != g_pServer)
	{
		ConnectServer_Stop(g_pServer);
	}
    printf("ConnectServer stop\n");
    if(NULL != g_HttpFileServer)
	{
		HttpFileServer_Stop(g_HttpFileServer);
	}
    printf("HttpFileServer stop\n");
    if(NULL != g_pMediaScanDriver)
    {
        MediaScanDriver_Stop(g_pMediaScanDriver);
    }
    printf("NetWorkDriver stop\n");
    if(NULL != g_pNetWorkDriver)
    {
        NetWorkDriver_Stop(g_pNetWorkDriver);
    }
    printf("DataBaseDriver stop\n");
    DataBaseDriver_Release();
    if(NULL != g_pClearCache)
    {
        ClearCache_Release(g_pClearCache);
    }
}

void InitParam_CheckDisk(json_object* pJsonRoot)
{
    long iTotal = 0;
    long iUsed = 0;
    Tools_GetDiskInfo(&iTotal, &iUsed);
    if(iTotal != 0)
    {
        printf("Disk mount success=>Total:%ld used:%ld\n", iTotal, iUsed);
        return;
    }
    json_object* pStoreObj = json_object_object_get(pJsonRoot, "store");
    const char* pszMount = json_object_get_string(json_object_object_get(pStoreObj, "mount"));
    if(NULL == pszMount || 0 == strlen(pszMount))
    {
        printf("config mount error\n");
        exit(0);
        return;
    }
  
    setenv("SYNC_MOUNT", pszMount, 1);
    Tools_GetDiskInfo(&iTotal, &iUsed);
    if(iTotal == 0)
    {
        printf("get disk info failed\n");
        exit(0);
        return;
    }
    printf("Disk mount success=>Total:%ld used:%ld\n", iTotal, iUsed);
}

void InitParam_SetDevInfo(json_object* pJsonRoot)
{
    json_object* pDevObj = json_object_object_get(pJsonRoot, "dev");
    DevItem item;
    item.pszDevID = (char*)json_object_get_string(json_object_object_get(pDevObj, "devid"));
    item.pszDevName = (char*)json_object_get_string(json_object_object_get(pDevObj, "devname"));
    item.pszDevVersion = (char*)json_object_get_string(json_object_object_get(pDevObj, "devversion"));
    item.pszDevBlue = (char*)json_object_get_string(json_object_object_get(pDevObj, "devblue"));
   
    DevItem* pDevItem = DataBaseDevice_GetDevInfo();
    if(NULL == pDevItem)
    {    
        DataBaseDevice_AddOrUpdateDevItem(&item);
    }
    else
    {
        if(0 != strcmp(pDevItem->pszDevID, item.pszDevID))
        {
            DataBaseDevice_AddOrUpdateDevItem(&item);
        }
        DataBaseDevice_ReleaseItem(pDevItem);
    }
}

BOOL InitParam_SetSoftLink(json_object* pJsonRoot)
{
    json_object* pStoreObj = json_object_object_get(pJsonRoot, "store");
    const char* pszMount = json_object_get_string(json_object_object_get(pStoreObj, "mount"));
    char szMountPoint[MAX_PATH] = {0};
    BOOL bRet = Tools_GetMountPointFromDevName(pszMount, szMountPoint);
    if(FALSE == bRet)
    {
        return FALSE;
    }
    char* pFAddr = szMountPoint;
    const char* pToAddr = json_object_get_string(json_object_object_get(pStoreObj, "taddr"));
    if(0 == strlen(pFAddr) || 0 == strlen(pToAddr))
    {
        printf("config store error\n");
        return FALSE;
    }
    strcat(pFAddr, "/media");
    printf("From Fold: %s\n", pFAddr);
    printf("To Fold: %s\n", pToAddr);

    FileUtil_RemoveFold(pToAddr);
    if(FALSE == FileUtil_CheckFoldExist(pFAddr))
    {
        FileUtil_CreateFold(pFAddr);
    }
    
    char szCmd[1000] = {0};
    sprintf(szCmd, "ln -s %s %s", pFAddr, pToAddr);
    printf("%s\n", szCmd);
    int iRet = system(szCmd);
    if(iRet != 0)
    {
        return FALSE;
    }

    sprintf(FOLDTHUMB_PREFIX, "%s/.media", pToAddr);
    FileUtil_CreateFold(FOLDTHUMB_PREFIX);
    
    sprintf(FOLDEX_PREFIX, "%s/.media_ex", pToAddr);
    FileUtil_CreateFold(FOLDEX_PREFIX);

    sprintf(TMPFILEPATH, "%s/.media_tmp", pToAddr);
    FileUtil_RemoveFold(TMPFILEPATH);
    FileUtil_CreateFold(TMPFILEPATH);

    char szLighttptUpload[MAX_PATH] = {0};
    sprintf(szLighttptUpload, "%s/.uploadtmp", pToAddr);
    FileUtil_RemoveFold(szLighttptUpload);
    FileUtil_CreateFold(szLighttptUpload);

    sprintf(TODIR, "%s/", pToAddr);

    sprintf(FOLD_PREFIX, "%s", pToAddr);
    FileUtil_CreateFold(FOLD_PREFIX);
    return TRUE;
}

void InitParam()
{
    char* pszCfgFilePath = FileUtil_CurrentPath();   
    char szCfgFile[255] = {0}; 
    sprintf(szCfgFile, "%s/Config.json", pszCfgFilePath);
    free(pszCfgFilePath);
    pszCfgFilePath = NULL;

    printf("config path:%s\n", szCfgFile);
    json_object* pJsonRoot = json_object_from_file(szCfgFile);
    //检测磁盘是否挂在成功
    InitParam_CheckDisk(pJsonRoot);
    //设置设备信息
    InitParam_SetDevInfo(pJsonRoot);
    //相关端口号
    json_object* pPortObj = json_object_object_get(pJsonRoot, "port");
    SERVERPORT = json_object_get_int(json_object_object_get(pPortObj, "server"));
    CASTPORT = json_object_get_int(json_object_object_get(pPortObj, "broadcast"));
    if(FALSE == InitParam_SetSoftLink(pJsonRoot))
    {
        printf("create softlink failed\n");
		exit(0);
        return ;
    }

    //其他配置
    json_object* pOthers = json_object_object_get(pJsonRoot, "others");
    g_bSamba = json_object_get_int(json_object_object_get(pOthers, "samba")) == 0?FALSE:TRUE;
    g_bEth = json_object_get_int(json_object_object_get(pOthers, "eth")) == 0?FALSE:TRUE;
    g_bWlan = json_object_get_int(json_object_object_get(pOthers, "wlan")) == 0?FALSE:TRUE;
    json_object_put(pJsonRoot);
}

BOOL SupportGpsGet()
{
    char* pszRet = Tools_ExecuteCmd("MetaParse -c");
    printf("%s\n", pszRet);
    BOOL bRet = TRUE;
    if(strcmp(pszRet, "success") == 0)
    {
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }
    free(pszRet);
    pszRet = NULL;
    return bRet;
}
void Daemonize(void)
{
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    if (0 != fork()) exit(0);
    if (-1 == setsid()) exit(0);
    signal(SIGHUP, SIG_IGN);
    if (0 != fork()) exit(0);
    //if (0 != chdir("/")) exit(0);
}
void CheckDaemonize(int iArgc, char *pszArgsV[])
{
    int iType;
    while(-1 != (iType = getopt(iArgc, pszArgsV, "hDd"))) 
    {
        printf("%d\n", iType);
        switch(iType) 
        {
            case 'D': 
            {
                Daemonize(); 
                break;
            }
            case 'd': 
            {
                break;
            }
            default:
            {
                break;
            }
        }
    } 
}

void CheckNotifyOnLine()
{
    //如果设备启动后亮绿灯(启动正常且读到IP地址),向局域网发送设备上线通知
    static BOOL bNotifyOnLine = FALSE;
    if(FALSE == bNotifyOnLine)
    {
        char* pszRet = Tools_ExecuteCmd("led get");
        int iReadRet = 0;
        int iReadValue = 0;
        sscanf(pszRet, "read ret: %d value:%d\n", &iReadRet, &iReadValue);
        if(2 == iReadValue)
        {
            printf("%s\n", "NotifyOnLine");
            bNotifyOnLine = TRUE;
            ConnectBroadCast_BroadCastNotifyOnLine(g_pConnectBroadCast, CASTPORT);
        }
        free(pszRet);
        pszRet = NULL;
    }
}

int main(int iArgc, char *pszArgsV[])
{
    printf("===========hello word=============\n");
    CheckDaemonize(iArgc, pszArgsV);
    g_bSupportGpsGet = SupportGpsGet();
    if(g_bSupportGpsGet)
    {
        printf("suport gps auto get\n");
    }
    else
    {
        printf("not suport gps auto get\n");
    }

    InitParam();
    printf("\n%s\n", "config orver");
    signal(SIGINT, OnSystemSig); 
	signal(SIGPIPE,  OnSignalHandler); 
 
    // UserItem useritem;
    // useritem.pszUserPwd = "96e79218965eb72c92a549dd5a330112";
    // useritem.pszUserPwdTip = "111111";
    // DataBaseUserInfo_AddRecord(&useritem);

    //TestMain();
    //samba文件删除确认完成了
    printf("Start ConnectServer");
    g_pServer = ConnectServer_Start(SERVERPORT);
    if(NULL == g_pServer)
    {
        printf(" %s\n", "FAILED");
        Exit();
        return 0;
    }
    else
    {
        printf(" %s\n", "SUCESS");
    }
    g_pServer->OnMessageProc = OnComputerServerMessage;
    g_pServer->ContectedConnProc = ComputerServerContectedConn;
    g_pServer->UnContectedConnProc = ComputerServerUnContectedConn;

    printf("Start BroadCast");
    g_pConnectBroadCast = ConnectBroadCast_Start(CASTPORT);
    if(NULL == g_pConnectBroadCast)
    {
        printf(" %s\n", "FAILED");
        Exit();
        return 0;
    }
    else
    {
        printf(" %s\n", "SUCESS");
    }
    g_pConnectBroadCast->OnMessageProc = OnConnectBroadCastMessage;
    
    // printf("Start HttpFileServer");
    // g_HttpFileServer = HttpFileServer_Start(SERVERPORT + 1);
    // if(NULL == g_HttpFileServer)
    // {
    //     printf(" %s\n", "FAILED");
    //     Exit();
    //     return 0;
    // }
    // else
    // {
    //     printf(" %s\n", "SUCESS");
    // }

    if(g_bEth && g_bWlan)
    {
        g_pNetWorkDriver = NetWorkDriver_Start();
    }
    
    while(!g_exit)
	{
		sleep(3);
#ifdef ARM
        CheckNotifyOnLine();
#endif
	}
    Exit();
    return 1;
}