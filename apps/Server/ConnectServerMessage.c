#include "stdafx.h"
#include "ConnectServerMessage.h"
#include "FileUtil.h"
#include "NetWorkUtil.h"
#include "DataBaseMediaJiShu.h"
#include "ClearCache.h"
#include "NetWorkDriver.h"
#include "DataBaseCover.h"
#include "DataBaseGroup.h"
#include "DataBaseGroupItems.h"
extern struct ClearCache* g_pClearCache;

BOOL ComputerServerMessage_SendCmdMessage(struct ConnectServer* pServer, SOCKET socket, const char* pBuffer)
{
	int iLen = strlen(pBuffer) + sizeof(InnerBase);
	char* pSendBuffer = (char*)malloc(iLen);
	memset((void*)pSendBuffer, 0, iLen);
	InnerBase* pMsg = (InnerBase*)pSendBuffer;
	pMsg->iMsgType = MESSAGETYPE_CMD;
	pMsg->iMsgLen = strlen(pBuffer) + 1;

	memcpy(pMsg->szBuf, pBuffer, strlen(pBuffer));

	BOOL bRet = ConnectServer_SendMessage(pServer, socket, (const char*)pSendBuffer, iLen);

	free(pSendBuffer);
	pSendBuffer = NULL;

	return bRet;
}
void ComputerServerMessage_OnTest(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", 1000);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}
void ComputerServerMessage_OnComputerServerCheckFile(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszPaiTime = json_object_get_string(json_object_object_get(pJsonRoot, "paitime"));
	const char* pszDevName = json_object_get_string(json_object_object_get(pJsonRoot, "devname"));
	const char* pszMediaType = json_object_get_string(json_object_object_get(pJsonRoot, "mediatype"));
	long iLFileSize = json_object_get_int64(json_object_object_get(pJsonRoot, "llen"));
	char szFileName[MAX_PATH] = {0};
	BOOL bMd5Exist = DataBaseMedia_FileNameFromPaiTime(pszPaiTime, pszDevName, pszMediaType, iLFileSize, szFileName);
	if(bMd5Exist)
	{
		//存在了报错
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\",\"filename\":\"%s\"}", MESSAGETYPECMD_CHECKFILE_ACK, "media exist", szFileName);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_CHECKFILE_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerCheckMd5(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszMd5 = json_object_get_string(json_object_object_get(pJsonRoot, "md5"));
	char szFileName[MAX_PATH] = {0};
	BOOL bMd5Exist = DataBaseMedia_FileNameFromMd5(pszMd5, szFileName);
	if(bMd5Exist)
	{
		//存在了报错
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\",\"filename\":\"%s\"}", MESSAGETYPECMD_CHECKMD5_ACK, "media exist", szFileName);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_CHECKMD5_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}
extern struct HttpFileServer* g_HttpFileServer;
void ComputerServerMessage_OnComputerServerUploadFile(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//int iFileSize = json_object_get_uint64(json_object_object_get(pJsonRoot, "filesize"));
	//取一个文件名称 名称为当前毫秒数
	char szFileName[MAX_PATH] = {0};
	FileUtil_GetNewFileName(szFileName, TMPFILEPATH);
	int iServerPort = SERVERPORT + 1;
	if(NULL == g_HttpFileServer)
	{
		iServerPort = 80;
	}
	// SOCKET iServerSocket = 0;
	// while(1)
	// {
	// 	iServerPort = Tools_RandomRangeValue(30000, 50000);
	// 	iServerSocket = SocketServer_OpenServer(iServerPort);
	// 	if(iServerSocket > 0)
	// 	{
	// 		break;
	// 	}
	// 	else
	// 	{
	// 		printf("get port failed:%d\n", iServerPort);
	// 		sleep(1);
	// 	}
	// }
	// SocketServer_Loop(iFileSize, szFileName, iServerSocket);
	//文件名称 端口号都有了 让客户端发送缩略图吧
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"filename\":\"%s\",\"conport\":\"%d\"}", MESSAGETYPECMD_UPLOADFILE_ACK, szFileName, iServerPort);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerReportInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//fname sfile slen lfile llen md5 ptime weizhi meititype meitisize deviceidentify mediaaddr addtime
	const char* pszFileName = json_object_get_string(json_object_object_get(pJsonRoot, "fname"));
	char szName[MAX_PATH] = {0};
	char szPostFix[MAX_PATH] = {0};
	FileUtil_SepFile(pszFileName, szName, szPostFix);
	//小文件信息
	const char* pszSFileName = json_object_get_string(json_object_object_get(pJsonRoot, "sfile"));
	long iSFileSize = json_object_get_int64(json_object_object_get(pJsonRoot, "slen"));
	//大文件信息
	const char* pszLFileName = json_object_get_string(json_object_object_get(pJsonRoot, "lfile"));
	long iLFileSize = json_object_get_int64(json_object_object_get(pJsonRoot, "llen"));
	//LivePhoto
	const char* pszExFileName = json_object_get_string(json_object_object_get(pJsonRoot, "exfile"));
	long iExFileSize = json_object_get_int64(json_object_object_get(pJsonRoot, "exlen"));
	//先判断文件在不再
	char szPathFile[MAX_PATH] = {0};
	sprintf(szPathFile, "%s/%s", TMPFILEPATH, pszSFileName);
	printf("~~~%s\n", szPathFile);
	if(FileUtil_GetFileSize(szPathFile) != iSFileSize)
	{
		//printf("%s device:%ld server:%ld\n", szPathFile, iSFileSize, FileUtil_GetFileSize(szPathFile));
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "small file size not match");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	memset(szPathFile, 0, MAX_PATH);
	sprintf(szPathFile, "%s/%s", TMPFILEPATH, pszLFileName);
	if(FileUtil_GetFileSize(szPathFile) != iLFileSize)
	{
		printf("%s device:%ld server:%ld\n", szPathFile, iLFileSize, FileUtil_GetFileSize(szPathFile));
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "large file size not match");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	memset(szPathFile, 0, MAX_PATH);
	sprintf(szPathFile, "%s/%s", TMPFILEPATH, pszExFileName);
	if(iExFileSize > 0 && FileUtil_GetFileSize(szPathFile) != iExFileSize)
	{
		printf("%s device:%ld server:%ld\n", szPathFile, iExFileSize, FileUtil_GetFileSize(szPathFile));
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "exfile file size not match");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	//检查当前时间是否正确
	if(Tools_GetYear(Tools_CurTimeSec()) == 1970)
	{
		long iUpdateTime = json_object_get_int64(json_object_object_get(pJsonRoot, "utimesec"));
		Tools_UpdateTimeSec(iUpdateTime);
	}
	//文件都是对的 下面开始存储信息了
	//判断年份文件夹是否存在
	int iPaiSheTime = json_object_get_uint64(json_object_object_get(pJsonRoot, "paitime"));
	if(iPaiSheTime == 0)
	{
		//没有拍摄时间用当前时间
		iPaiSheTime = Tools_CurTimeSec();
	}
	int iYear = Tools_GetYear(iPaiSheTime);
	printf("iPaiSheTime:%d year:%d\n", iPaiSheTime, iYear);
	char szDestFold[MAX_PATH] = {0};
	sprintf(szDestFold, "%s/%d/", FOLD_PREFIX, iYear);
	char szDestThumbFold[MAX_PATH] = {0};
	sprintf(szDestThumbFold, "%s/%d/", FOLDTHUMB_PREFIX, iYear);
	char szDestExFold[MAX_PATH] = {0};
	sprintf(szDestExFold, "%s/%d/", FOLDEX_PREFIX, iYear);
	if(FALSE == FileUtil_CheckFoldExist(szDestFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			return ;
		}
	}
	if(FALSE == FileUtil_CheckFoldExist(szDestThumbFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestThumbFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			return ;
		}
	}
	if(FALSE == FileUtil_CheckFoldExist(szDestExFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestExFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			return ;
		}
	}

	char szFileName[MAX_PATH] = {0};
	char szSmallDestFile[MAX_PATH] = {0};
	char szLargeDestFile[MAX_PATH] = {0};
	char szExDestFile[MAX_PATH] = {0};
	//使用图片原来的名字
	// FileUtil_GetNewFileName2(iYear, szName, szPostFix, szFileName);
	//使用毫秒数的名字
	FileUtil_GetNewFileName2(iYear, pszLFileName, szPostFix, szFileName);
	sprintf(szLargeDestFile, "%s%s.%s", szDestFold, szFileName, szPostFix);
	sprintf(szSmallDestFile, "%s%s_%s.%s", szDestThumbFold, szFileName, szPostFix, "jpg");
	sprintf(szExDestFile, "%s%s_%s.%s", szDestThumbFold, szFileName, szPostFix, "mp4");
	
	char szSmallSrcFile[MAX_PATH] = {0};
	char szLargeSrcFile[MAX_PATH] = {0};
	char szExSrcFile[MAX_PATH] = {0};
	sprintf(szSmallSrcFile, "%s/%s", TMPFILEPATH, pszSFileName);
	sprintf(szLargeSrcFile, "%s/%s", TMPFILEPATH, pszLFileName);
	sprintf(szExSrcFile, "%s/%s", TMPFILEPATH, pszExFileName);
	if(FALSE == FileUtil_MoveFile(szLargeSrcFile, szLargeDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "move lager file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	if(FALSE == FileUtil_MoveFile(szSmallSrcFile, szSmallDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "move small file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	if(iExFileSize > 0 && FALSE == FileUtil_MoveFile(szExSrcFile, szExDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "move ex file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}
	//const char* pPaiSheShiJian = json_object_get_string(json_object_object_get(pJsonRoot, "paitime"));
	const char* pszMediaType = json_object_get_string(json_object_object_get(pJsonRoot, "mediatype"));
	const char* pszMd5Num = json_object_get_string(json_object_object_get(pJsonRoot, "md5"));
	const char* pszWeiZhi = json_object_get_string(json_object_object_get(pJsonRoot, "weizhi"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devicename"));
	const char* pszWidth = json_object_get_string(json_object_object_get(pJsonRoot, "width"));
	const char* pszHeight = json_object_get_string(json_object_object_get(pJsonRoot, "height"));
	const char* pszDuration = json_object_get_string(json_object_object_get(pJsonRoot, "duration"));
	const char* pszLocation = json_object_get_string(json_object_object_get(pJsonRoot, "location"));

	//校验位置信息如果没有在获取一次
	float dLat = 0;
	float dLong = 0;
	GPSTYPE iGpsType = 0;
	char* pszWeiZhi2 = NULL;
	if(FALSE == Tools_CheckGps(pszWeiZhi, &iGpsType, &dLong, &dLat))
	{
		if(g_bSupportGpsGet)
		{
			char szCmd[300] = {0};
			sprintf(szCmd, "MetaParse -g %s", szLargeDestFile);
			pszWeiZhi2 = Tools_ExecuteCmd(szCmd);
		}
	}
	else
	{
		int iDestLen = strlen(pszWeiZhi) + 1;
		pszWeiZhi2 = malloc(iDestLen);
		memset(pszWeiZhi2, 0, iDestLen);
		strcpy(pszWeiZhi2, pszWeiZhi);
	}
	if(NULL == pszWeiZhi2)
	{
		pszWeiZhi2 = malloc(4);
		memset(pszWeiZhi2, 0, 4);
		strcpy(pszWeiZhi2, "0&0");
	}
	else if(strlen(pszWeiZhi2) == 0)
	{
		free(pszWeiZhi2);
		pszWeiZhi2 = NULL;
		pszWeiZhi2 = malloc(4);
		memset(pszWeiZhi2, 0, 4);
		strcpy(pszWeiZhi2, "0&0");
	}

	char szFileAddr[MAX_PATH] = {0};
	sprintf(szFileAddr, "%d/%s.%s", iYear, szFileName, szPostFix);

	int iBufferLen = sizeof(MediaItem);
	char* pBuffer = malloc(iBufferLen);
	memset(pBuffer, 0, iBufferLen);
	MediaItem* pMediaItem = (MediaItem*)pBuffer;
	
	pMediaItem->iPaiSheTime = iPaiSheTime; 							//拍摄时间
	pMediaItem->iAddTime = Tools_CurTimeSec(); 					//添加时间
	pMediaItem->iMediaType = atoi(pszMediaType); 					//媒体类型
	pMediaItem->pszMd5Num = (char*)pszMd5Num;  								//MD5值
	pMediaItem->pszWeiZhi = (char*)pszWeiZhi2; 								//拍摄时候的精度纬度 
	pMediaItem->iMeiTiSize = iLFileSize; 							//拍摄图片的大小
	pMediaItem->iWidth = atoi(pszWidth);							//宽度
	pMediaItem->iHeight = atoi(pszHeight);							//高度
	pMediaItem->iDuration = atoi(pszDuration);						//持续时间
	pMediaItem->pszDeviceIdentify = (char*)pszDeviceName; 					//设备名称
	pMediaItem->pszAddr = szFileAddr;									//媒体地址
	pMediaItem->pszLocation = pszLocation==NULL?"":(char*)pszLocation;  //拍摄时候地理位置
	pMediaItem->iHasExtra = iExFileSize > 0?TRUE:FALSE;
	BOOL bRet = DataBaseMedia_AddItem(pMediaItem);
	if(FALSE == bRet)
	{
		//入库失败了
		FileUtil_RemoveFile(szSmallDestFile);
		FileUtil_RemoveFile(szLargeDestFile);
		FileUtil_RemoveFile(szExDestFile);
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, "add record error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"file\":\"%s\"}", MESSAGETYPECMD_REPORTINFO_ACK, szFileAddr);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	free(pszWeiZhi2);
	pszWeiZhi2 = NULL;
	free(pBuffer);
	pBuffer = NULL;
}

void ComputerServerMessage_OnComputerServerDeviceInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//获取设备名称 版本号
	DevItem* pItem = DataBaseDevice_GetDevInfo();
	if(NULL == pItem)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_DEVICEINFO_ACK, "inner error");
		printf("%s\n", szBuffer);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	int iRecordCount = 0;
	BOOL bRet = DataBaseMedia_GetRecordCount(&iRecordCount);
	if(FALSE == bRet)
	{
		DataBaseDevice_ReleaseItem(pItem);

		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_DEVICEINFO_ACK, "inner error 1");
		printf("%s\n", szBuffer);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	//是否需要登录
	int iUserCount = 0;
	bRet = DataBaseUserInfo_GetRecordCount(&iUserCount);
	if(FALSE == bRet)
	{
		DataBaseDevice_ReleaseItem(pItem);

		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_DEVICEINFO_ACK, "inner error 2");
		printf("%s\n", szBuffer);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	//最后更新时间 这个指针有可能是空的 用的时候需要做好判断
	MediaItem* pMediaItem = DataBaseMedia_GetLatestItem();
	
	//读取封面
	char* pszCover = DataBaseCover_GetHomeCover();
	char szCoverFile[MAX_PATH] = {0};
	if(strlen(pszCover) == 0)
	{
		strcpy(szCoverFile, NULL == pMediaItem?"":pMediaItem->pszAddr);
	}
	else
	{
		sprintf(szCoverFile, "%s/%s", FOLD_PREFIX, pszCover);
		BOOL bCover = FileUtil_CheckFileExist(szCoverFile);
		if(FALSE == bCover)
		{
			//文件不存在
			DataBaseCover_RemoveHomeCover();
			strcpy(szCoverFile, NULL == pMediaItem?"":pMediaItem->pszAddr);
		}
		else
		{
			memset(szCoverFile, 0, MAX_PATH);
			strcpy(szCoverFile, pszCover);
		}
	}

	free(pszCover);
	pszCover = NULL;

	char* pszMac = Tools_GetMacAddr();
	char* pszLicence = FileUtil_GetLicence();

	json_object* pJsonRet = json_object_new_object();
	json_object_object_add(pJsonRet, "otype", json_object_new_int(MESSAGETYPECMD_DEVICEINFO_ACK));
	json_object_object_add(pJsonRet, "status", json_object_new_int(1));
	json_object_object_add(pJsonRet, "devid", json_object_new_string(pItem->pszDevID));
	json_object_object_add(pJsonRet, "devname", json_object_new_string(NULL == pItem->pszDevName ? "":pItem->pszDevName));
	json_object_object_add(pJsonRet, "devversion", json_object_new_string(pItem->pszDevVersion));
	json_object_object_add(pJsonRet, "devblue", json_object_new_string(pItem->pszDevBlue));
	json_object_object_add(pJsonRet, "mediacount", json_object_new_int(iRecordCount));
	json_object_object_add(pJsonRet, "lastupdatetime", json_object_new_int64(NULL == pMediaItem?0:pMediaItem->iAddTime));
	json_object_object_add(pJsonRet, "login", json_object_new_int(iUserCount > 0?1:0));
	json_object_object_add(pJsonRet, "pic", json_object_new_string(szCoverFile));
	json_object_object_add(pJsonRet, "pic2", json_object_new_string(NULL == pMediaItem?"":pMediaItem->pszAddr));
	json_object_object_add(pJsonRet, "eth", json_object_new_int(g_bEth));
	json_object_object_add(pJsonRet, "wlan", json_object_new_int(g_bWlan));
	json_object_object_add(pJsonRet, "mac", json_object_new_string(pszMac));
	json_object_object_add(pJsonRet, "licence", json_object_new_string(pszLicence));
	json_object_object_add(pJsonRet, "buildtime", json_object_new_string(BUILDDATE));//版本编译时间

	free(pszMac);
	pszMac = NULL;
	free(pszLicence);
	pszLicence = NULL;

	const char* pszJson = json_object_to_json_string(pJsonRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszJson);
	printf("%s\n", pszJson);
	json_object_put(pJsonRet);
	DataBaseDevice_ReleaseItem(pItem);
	if(NULL != pMediaItem)
	{
		DataBaseMedia_ReleaseItem(pMediaItem);
	}
}

void ComputerServerMessage_OnComputerServerDiskInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	long iTotal = 0;
	long iUsed = 0;
	Tools_GetDiskInfo(&iTotal, &iUsed);
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"total\":\"%ld\",\"used\":\"%ld\"}", MESSAGETYPECMD_DISKINFO_ACK, iTotal, iUsed);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerLogin(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszPwd = json_object_get_string(json_object_object_get(pJsonRoot, "pwd"));
	printf("%s\n", pszPwd);
	BOOL bRet = DataBaseUserInfo_CheckPwd(pszPwd);
	if(TRUE == bRet)
	{
		printf("%s\n", "success");
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_LOGIN_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		printf("%s\n", "failed");
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_LOGIN_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
}

void ComputerServerMessage_OnComputerServerUpdateDeviceTime(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszTime = json_object_get_string(json_object_object_get(pJsonRoot, "time"));
	long iTime = atol(pszTime);
	BOOL bRet = TRUE;
	if(Tools_CurTimeSec() < iTime)
	{
		bRet = Tools_UpdateTimeSec(atol(pszTime));
	}
	if(TRUE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_UPDATEDEVICETIME_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_UPDATEDEVICETIME_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
}

void ComputerServerMessage_OnComputerServerPwdTip(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	char* pPwdTip = (char*)malloc(255);
	BOOL bRet = DataBaseUserInfo_GetPwdTip(&pPwdTip);
	if(TRUE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"pwdtip\":\"%s\"}", MESSAGETYPECMD_PWDTIP_ACK, "");
		json_object* pJsonItem = json_tokener_parse(szBuffer);
		json_object_object_del(pJsonItem, "pwdtip"); //json对象删除
		json_object_object_add(pJsonItem, "pwdtip", json_object_new_string(pPwdTip)); //json对象添加
		const char* pszJson = json_object_to_json_string(pJsonItem);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszJson);
		json_object_put(pJsonItem);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"pwdtip\":\"%s\"}", MESSAGETYPECMD_PWDTIP_ACK, "");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	free(pPwdTip);
	pPwdTip = NULL;
}

void ComputerServerMessage_OnComputerServerResetUser(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszPwd = json_object_get_string(json_object_object_get(pJsonRoot, "pwd"));
	const char* pszPwdTip = json_object_get_string(json_object_object_get(pJsonRoot, "pwdtip"));
	printf("PWD:%s\n", pszPwd);
	printf("PwdTip:%s\n", pszPwdTip);
    BOOL bRet = FALSE;
    char szSmbPasswd[20] = {0};//pwd maxlen 10
    
	if(strlen(pszPwd) == 0)
	{
		bRet = DataBaseUserInfo_DeleteRecord();
	}
    else
    {
		strcpy(szSmbPasswd, pszPwd);

        char* pszPwdMd5 = Tools_GetMd5(pszPwd);
	    UserItem item;
	    item.pszUserPwd = (char*)pszPwdMd5;
	    item.pszUserPwdTip = (char*)pszPwdTip;
	    bRet = DataBaseUserInfo_AddRecord(&item);
	    free(pszPwdMd5);
	    pszPwdMd5 = NULL;
    }
	
	if(TRUE == bRet && TRUE == g_bSamba)
	{
		//修改samba密码
		if(strlen(szSmbPasswd) == 0)
		{
			DevItem* pItem = DataBaseDevice_GetDevInfo();
        	strcpy(szSmbPasswd, pItem->pszDevID);
			DataBaseDevice_ReleaseItem(pItem);
		}
		char szSambaCmd[1000] = {0};
		sprintf(szSambaCmd, "passwd=%s && (echo $passwd;echo $passwd) | smbpasswd -a root -s", szSmbPasswd);
		int iStatus = system(szSambaCmd);
		bRet = FileUtil_CheckStatus(iStatus);	
	}
	if(TRUE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_RESETUSER_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_RESETUSER_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
}

void ComputerServerMessage_OnComputerServerUpdateDeviceInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devname"));
	const char* pszDeviceBlue = json_object_get_string(json_object_object_get(pJsonRoot, "devblue"));
	
	DevItem item;
	item.pszDevBlue = (char*)pszDeviceBlue;
	item.pszDevName = (char*)pszDeviceName;
	
	BOOL bRet = DataBaseDevice_AddOrUpdateDevItem(&item);
	if(TRUE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"devname\":\"%s\",\"devblue\":\"%s\"}", MESSAGETYPECMD_UPDATEDEVICEINFO_ACK, "", pszDeviceBlue);

		json_object* pJsonItem = json_tokener_parse(szBuffer);
		json_object_object_del(pJsonItem, "devname"); 
		json_object_object_add(pJsonItem, "devname", json_object_new_string(pszDeviceName)); 
		const char* pszJson = json_object_to_json_string(pJsonItem);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszJson);
		json_object_put(pJsonItem);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_UPDATEDEVICEINFO_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
}

void ComputerServerMessage_OnComputerServerServerItems(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//type : 0最近 1收藏 2:年份
	int iType = json_object_get_int(json_object_object_get(pJsonRoot,"type"));
	int iPage = json_object_get_int(json_object_object_get(pJsonRoot,"page"));
	int iLimited = json_object_get_int(json_object_object_get(pJsonRoot, "limit"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	
    int iRecordBufferLen = iLimited*300;
	char* pRecordBuffer = (char*)malloc(iRecordBufferLen);
	memset(pRecordBuffer, 0, iRecordBufferLen);
	BOOL bRet = FALSE;
	int iTotalCount = 0;
	switch (iType)
	{
		case 0:
		{
			// 0最近
			bRet = DataBaseMedia_GetRecentRecords(iPage, iLimited, pszDevice, &pRecordBuffer);
			break;
		}
		case 1:
		{
			//2收藏
			bRet = DataBaseMedia_GetFavoriteRecords(iPage, iLimited, pszDevice, &pRecordBuffer);
			break;
		}
		case 2:
		{
			//年份
			int iYear = json_object_get_int(json_object_object_get(pJsonRoot,"year"));
			int iMonth = json_object_get_int(json_object_object_get(pJsonRoot,"month"));
			int iDay = json_object_get_int(json_object_object_get(pJsonRoot,"day"));
			char* pszLocation = (char*)json_object_get_string(json_object_object_get(pJsonRoot, "location"));
			bRet = DataBaseMedia_GetYearRecords(iPage, iLimited, iYear, iMonth, iDay, pszDevice, pszLocation, &pRecordBuffer);
			break;
		}
		case 3:
		{
			//分组查询
			int iGid = json_object_get_int(json_object_object_get(pJsonRoot,"gid"));
			char* pszMediaIDs = DataBaseGroupItems_MediaIds(iPage, iLimited, iGid, pszDevice);
			char* pszMediaIds = Tools_ReplaceString((char*)pszMediaIDs, "&", "','");
			free(pszMediaIDs);
			pszMediaIDs = NULL;
			bRet =  DataBaseMedia_RecordsFromIds(pszMediaIds, &pRecordBuffer);
			free(pszMediaIds);
			pszMediaIds = NULL;
			if (iPage == 0)
			{
				iTotalCount = DataBaseGroupItems_MediaItemCount(iGid, pszDevice);
			}
			break;
		}
	}

	free(pszDevice);
	pszDevice = NULL;
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_SERVERITEMS_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		free(pRecordBuffer);
		pRecordBuffer = NULL;
		return;
	}
	
	int iRetLen = strlen(pRecordBuffer);
	char* pszRetBuffer = malloc(iRetLen + 255);
	memset(pszRetBuffer, 0, iRetLen + 255);
	if(iTotalCount == 0)
	{
		sprintf(pszRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_SERVERITEMS_ACK, pRecordBuffer);
	}
	else
	{
		sprintf(pszRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"tcount\":%d,\"items\":[%s]}", MESSAGETYPECMD_SERVERITEMS_ACK, iTotalCount, pRecordBuffer);
	}
	
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetBuffer);

    free(pRecordBuffer);
	pRecordBuffer = NULL;
	free(pszRetBuffer);
	pszRetBuffer = NULL;
}

void ComputerServerMessage_OnComputerServerMediaItemInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iID = json_object_get_int(json_object_object_get(pJsonRoot,"id"));
	char* pBuffer = (char*)malloc(500);
	memset(pBuffer, 0, 500);
	BOOL bRet = DataBaseMedia_GetItem(iID, &pBuffer);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMINFO_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	int iRetLen = strlen(pBuffer);
	char* pszRetBuffer = malloc(iRetLen + 255);
	memset(pszRetBuffer, 0, iRetLen + 255);
	sprintf(pszRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_MEDIAITEMINFO_ACK, pBuffer);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetBuffer);
	printf("%s\n", pszRetBuffer);
	free(pBuffer);
	pBuffer = NULL;
	free(pszRetBuffer);
	pszRetBuffer = NULL;
}

void ComputerServerMessage_OnComputerServerMediaItemFavorite(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iID = json_object_get_int(json_object_object_get(pJsonRoot,"id"));
	int iFavorite = json_object_get_int(json_object_object_get(pJsonRoot,"favorite"));
	BOOL bRet = DataBaseMedia_UpdateFavorite(iID, iFavorite);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMFAVORITE_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"id\":\"%d\"}", MESSAGETYPECMD_MEDIAITEMFAVORITE_ACK, iID);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerMediaItemDelete(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iID = json_object_get_int(json_object_object_get(pJsonRoot,"id"));
	char szFileName[MAX_PATH] = {0};
	BOOL bRet = DataBaseMedia_GetFileName(iID, szFileName);
	if(FALSE == bRet || strlen(szFileName) == 0)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMDEL_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	bRet = DataBaseGroupItems_RemoveFromItemID(iID);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMDEL_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	//删除group cover
	bRet = DataBaseGroup_SetCoverEmpty(szFileName);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMDEL_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	bRet = DataBaseMedia_RemoveItem(iID);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIAITEMDEL_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"id\":\"%d\"}", MESSAGETYPECMD_MEDIAITEMDEL_ACK, iID);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);

	//删除文件
	char szThumbFileName[255] = {0};
	FileUtil_ThumbNameFromFileName(szFileName, szThumbFileName);
	char szPathFile[255] = {0};
	sprintf(szPathFile, "%s/%s", FOLD_PREFIX, szFileName);
	FileUtil_RemoveFile(szPathFile);
	printf("remove file:%s\n", szPathFile);
	char szExFileName[255] = {0};
	FileUtil_FileExNameFromFileName(szFileName, szExFileName);
	memset(szPathFile, 0, 255);
	sprintf(szPathFile, "%s/%s", FOLDEX_PREFIX, szExFileName);
	FileUtil_RemoveFile(szPathFile);
	printf("remove file:%s\n", szPathFile);
	memset(szPathFile, 0, 255);
	sprintf(szPathFile, "%s/%s", FOLDTHUMB_PREFIX, szThumbFileName);
	FileUtil_RemoveFile(szPathFile);
	printf("remove file:%s\n", szPathFile);
	//删除目录
	int iYear = FileUtil_GetYear(szFileName);
	FileUtil_RemoveYearEmptyFold(iYear);
}

void ComputerServerMessage_OnComputerServerMediaMonthList(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot, "year"));
	char* pszMonth = DataBaseMediaJiShu_GetMonths(iYear, pszDevice);
	free(pszDevice);
	pszDevice = NULL;
	int iLen = strlen(pszMonth) + 100;
	char* pBuffer = (char*)malloc(iLen);
	memset(pBuffer, 0, iLen);
	sprintf(pBuffer, "{\"otype\":\"%d\",\"status\":1,\"months\":[%s]}", MESSAGETYPECMD_MEDIAMONTHLIST_ACK, pszMonth);
	printf("%s\n", pBuffer);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pBuffer);
	free(pBuffer);
	pBuffer = NULL;
	free(pszMonth);
	pszMonth = NULL;
}

void ComputerServerMessage_OnComputerServerMediaDayList(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	printf("ComputerServerMessage_OnComputerServerMediaDayList\n");
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot, "year"));
	int iMonth = json_object_get_int(json_object_object_get(pJsonRoot, "month"));
	char* pszDays = DataBaseMediaJiShu_GetDays(iYear, iMonth, pszDevice);
	free(pszDevice);
	pszDevice = NULL;

	int iLen = strlen(pszDays) + 100;
	char* pBuffer = (char*)malloc(iLen);
	memset(pBuffer, 0, iLen);
	sprintf(pBuffer, "{\"otype\":\"%d\",\"status\":1,\"days\":[%s]}", MESSAGETYPECMD_MEDIADAYSLIST_ACK, pszDays);
	printf("%s\n", pBuffer);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pBuffer);
	free(pBuffer);
	pBuffer = NULL;
	free(pszDays);
	pszDays = NULL;
}

void ComputerServerMessage_OnComputerServerMediaYearList(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	char* pszYears = DataBaseMediaJiShu_GetYears(pszDevice);
	free(pszDevice);
	pszDevice = NULL;
	int iLen = strlen(pszYears) + 100;
	char* pBuffer = (char*)malloc(iLen);
	memset(pBuffer, 0, iLen);
	sprintf(pBuffer, "{\"otype\":\"%d\",\"status\":1,\"years\":\"%s\"}", MESSAGETYPECMD_MEDIAYEARLIST_ACK, pszYears);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pBuffer);
	free(pBuffer);
	pBuffer = NULL;
	free(pszYears);
	pszYears = NULL;
}

void ComputerServerMessage_OnComputerServerMediaYearInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot,"year"));

	//int iStartTime = Tools_YearStartSec(iYear);
	//int iEndTime = Tools_YearEndSec(iYear);

	//int iPicCount = DataBaseMedia_GetMediaYearCount(iStartTime, iEndTime, MEDIATYPE_IMAGE, pszDevice);
	//int iVideoCount = DataBaseMedia_GetMediaYearCount(iStartTime, iEndTime, MEDIATYPE_VIDEO, pszDevice);
	int iPicCount = 0;
	int iVideoCount = 0;
	DataBaseMediaJiShu_GetYearInfo(iYear, pszDevice, &iPicCount, &iVideoCount);
	free(pszDevice);
	pszDevice = NULL;

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"year\":\"%d\",\"piccount\":\"%d\",\"videocount\":\"%d\"}", MESSAGETYPECMD_MEDIAYEARINFO_ACK, iYear, iPicCount, iVideoCount);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerDevNames(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	char* pDevNames = DataBaseMediaJiShu_GetDevNames();
	int iRetBufferLen = strlen(pDevNames) + 1024;
	char* pRetBuffer = malloc(iRetBufferLen);
	memset(pRetBuffer, 0, iRetBufferLen);
	
	sprintf(pRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"devnames\":[%s]}", MESSAGETYPECMD_MEDIADEVNAMES_ACK, pDevNames);

	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pRetBuffer);
	free(pRetBuffer);
	pRetBuffer = NULL;
	free(pDevNames);
	pDevNames = NULL;
}

void ComputerServerMessage_OnComputerServerRecentInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	
	int iPicCount = DataBaseMedia_GetRecentRecordCount(MEDIATYPE_IMAGE, pszDevice);
	int iVideoCount = DataBaseMedia_GetRecentRecordCount(MEDIATYPE_VIDEO, pszDevice);

	free(pszDevice);
	pszDevice = NULL;

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"piccount\":\"%d\",\"videocount\":\"%d\"}", MESSAGETYPECMD_MEDIARECENTINFO_ACK, iPicCount, iVideoCount);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerMediaFavoriteInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	
	int iPicCount = DataBaseMedia_GetRecordFavoriteCount(MEDIATYPE_IMAGE, pszDevice);
	int iVideoCount = DataBaseMedia_GetRecordFavoriteCount(MEDIATYPE_VIDEO, pszDevice);

	free(pszDevice);
	pszDevice = NULL;

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"piccount\":\"%d\",\"videocount\":\"%d\"}", MESSAGETYPECMD_MEDIAFAVORITEINFO_ACK, iPicCount, iVideoCount);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerMediaUnCheckGps(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iLimited = json_object_get_int(json_object_object_get(pJsonRoot, "limit"));
	char* pszGPS = DataBaseMedia_GetUnCheckWeiZhi(iLimited);//{"weizhi":"118.745003&32.054980"},{"weizhi":"118.745003&32.054980"}
	int iRetBufferLen = strlen(pszGPS) + 1024;
	char* pRetBuffer = malloc(iRetBufferLen);
	memset(pRetBuffer, 0, iRetBufferLen);
	if(NULL == pszGPS)
	{
		sprintf(pRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_MEDIAUNCHECKGPS_ACK, "");
	}
	else
	{
		sprintf(pRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_MEDIAUNCHECKGPS_ACK, pszGPS);
	}
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pRetBuffer);
	free(pRetBuffer);
	pRetBuffer = NULL;
	free(pszGPS);
	pszGPS = NULL;
}

void ComputerServerMessage_OnComputerServerMediaUpdateGpsWeiZhi(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszWeiZhi = json_object_get_string(json_object_object_get(pJsonRoot, "weizhi"));
	const char* pszBaiDuWeiZhi = json_object_get_string(json_object_object_get(pJsonRoot, "bdweizhi"));
	const char* pszLocation = json_object_get_string(json_object_object_get(pJsonRoot, "location"));
	BOOL bRet = DataBaseMedia_UpdateGpsWeiZhi(pszWeiZhi, pszBaiDuWeiZhi, pszLocation);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_MEDIANUPDATEGPSWEIZHI_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_MEDIANUPDATEGPSWEIZHI_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnComputerServerMediaYearLocationGroup(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iStart = json_object_get_int(json_object_object_get(pJsonRoot,"start"));
	int iLimited = json_object_get_int(json_object_object_get(pJsonRoot, "limit"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot,"year"));
	int iMonth = json_object_get_int(json_object_object_get(pJsonRoot,"month"));
	int iDay = json_object_get_int(json_object_object_get(pJsonRoot,"day"));
    
	char* pRetBuffer = DataBaseMedia_GetLocationGroup(iStart, iLimited, iYear, iMonth, iDay, pszDevice);

	free(pszDevice);
	pszDevice = NULL;
	if(NULL == pRetBuffer)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_YEARLOCATIONGROUP_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	
	int iRetLen = strlen(pRetBuffer);
	char* pszRetBuffer = malloc(iRetLen + 255);
	memset(pszRetBuffer, 0, iRetLen + 255);
	sprintf(pszRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_YEARLOCATIONGROUP_ACK, pRetBuffer);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetBuffer);

	free(pszRetBuffer);
	pszRetBuffer = NULL;

	free(pRetBuffer);
	pRetBuffer = NULL;
}

void ComputerServerMessage_OnComputerServerMediaYearLocationGroupTongJi(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszLocation = json_object_get_string(json_object_object_get(pJsonRoot, "location"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot,"year"));
	int iMonth = json_object_get_int(json_object_object_get(pJsonRoot,"month"));
	int iDay = json_object_get_int(json_object_object_get(pJsonRoot,"day"));

	char* pRet = DataBaseMedia_YearLocationGroupTongJi(pszLocation, iYear, iMonth, iDay, pszDevice);
	free(pszDevice);
	pszDevice = NULL;
	if(NULL == pRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0}", MESSAGETYPECMD_YEARLOCATIONGROUPTONGJI_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}

	int iLen = strlen(pRet) + 1024;
	char* pszRetBuffer = malloc(iLen);
	memset(pszRetBuffer, 0, iLen);
	sprintf(pszRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_YEARLOCATIONGROUPTONGJI_ACK, pRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetBuffer);
	free(pRet);
	pRet = NULL;

	free(pszRetBuffer);
	pszRetBuffer = NULL;
}

void ComputerServerMessage_OnUpdatePaiSheTime(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iItemID = json_object_get_uint64(json_object_object_get(pJsonRoot, "id"));
	int iTimeSec = json_object_get_uint64(json_object_object_get(pJsonRoot, "timesec"));
	MediaItem* pItem = DataBaseMedia_GetItemByID(iItemID);
	if(NULL == pItem)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "item not find");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return ;
	}

	int iYear = Tools_GetYear(iTimeSec);
	char szDestFold[MAX_PATH] = {0};
	sprintf(szDestFold, "%s/%d/", FOLD_PREFIX, iYear);
	char szDestThumbFold[MAX_PATH] = {0};
	sprintf(szDestThumbFold, "%s/%d/", FOLDTHUMB_PREFIX, iYear);
	char szDestExFold[MAX_PATH] = {0};
	sprintf(szDestExFold, "%s/%d/", FOLDEX_PREFIX, iYear);
	if(FALSE == FileUtil_CheckFoldExist(szDestFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			DataBaseMedia_ReleaseItem(pItem);
			return ;
		}
	}
	if(FALSE == FileUtil_CheckFoldExist(szDestThumbFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestThumbFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			DataBaseMedia_ReleaseItem(pItem);
			return ;
		}
	}
	if(FALSE == FileUtil_CheckFoldExist(szDestExFold))
	{
		//文件夹不存在 创建
		if(FALSE == FileUtil_CreateFold(szDestExFold))
		{
			char szBuffer[255] = {0};
			sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "fold no permission");
			ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
			DataBaseMedia_ReleaseItem(pItem);
			return ;
		}
	}
	//2020/IMG_2937_1.jpg   => IMG_2937_1.jpg
	char szFileName[MAX_PATH] = {0};
	FileUtil_GetFileOnlyName(pItem->pszAddr, szFileName);

	//IMG_2937_1.jpg => szName(IMG_2937_1) szPostFix(jpg)
	char szName[MAX_PATH] = {0};
	char szPostFix[MAX_PATH] = {0};
	FileUtil_SepFile(szFileName, szName, szPostFix);

	//get new filename
	memset(szFileName, 0, MAX_PATH);
	FileUtil_GetNewFileName2(iYear, szName, szPostFix, szFileName);

	char szSmallDestFile[MAX_PATH] = {0};
	char szLargeDestFile[MAX_PATH] = {0};
	char szExDestFile[MAX_PATH] = {0};
	sprintf(szLargeDestFile, "%s%s.%s", szDestFold, szFileName, szPostFix);
	sprintf(szSmallDestFile, "%s%s_%s.%s", szDestThumbFold, szFileName, szPostFix, "jpg");
	sprintf(szExDestFile, "%s%s_%s.%s", szDestExFold, szFileName, szPostFix, "mp4");
	
	char szSmallSrcFile[MAX_PATH] = {0};
	char szLargeSrcFile[MAX_PATH] = {0};
	char szExSrcFile[MAX_PATH] = {0};
	
	memset(szName, 0, MAX_PATH);
	memset(szPostFix, 0, MAX_PATH);
	FileUtil_SepFile(pItem->pszAddr, szName, szPostFix);

	sprintf(szSmallSrcFile, "%s/%s_%s.jpg", FOLDTHUMB_PREFIX, szName, szPostFix);
	sprintf(szExSrcFile, "%s/%s_%s.mp4", FOLDEX_PREFIX, szName, szPostFix);
	sprintf(szLargeSrcFile, "%s/%s", FOLD_PREFIX, pItem->pszAddr);

	if(FALSE == FileUtil_MoveFile(szLargeSrcFile, szLargeDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "move lager file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		DataBaseMedia_ReleaseItem(pItem);
		return ;
	}

	if(FALSE == FileUtil_MoveFile(szSmallSrcFile, szSmallDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "move small file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		DataBaseMedia_ReleaseItem(pItem);
		return ;
	}
	if(1 == pItem->iHasExtra && FALSE == FileUtil_MoveFile(szExSrcFile, szExDestFile))
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, "move ex file error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		DataBaseMedia_ReleaseItem(pItem);
		return ;
	}
	
	int iOldYear = FileUtil_GetYear(pItem->pszAddr);
	FileUtil_RemoveYearEmptyFold(iOldYear);
    
	memset(szName, 0, MAX_PATH);
	sprintf(szName, "%d/%s.%s", iYear, szFileName, szPostFix);
	BOOL bRet = DataBaseMedia_UpdateItemPaiSheShiJian(iItemID, iTimeSec, szName);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"id\":%d}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, iItemID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"id\":%d}", MESSAGETYPECMD_UPDATEPAISHETIME_ACK, iItemID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}

	//计数 -1
	DataBaseMediaJiShu_Decrease(pItem->iPaiSheTime, pItem->iMediaType, pItem->pszDeviceIdentify);
	//计数 +1
	DataBaseMediaJiShu_Increase(iTimeSec, pItem->iMediaType, pItem->pszDeviceIdentify);

	DataBaseMedia_ReleaseItem(pItem);
}
void ComputerServerMessage_OnUpdateGpsAddr(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iItemID = json_object_get_uint64(json_object_object_get(pJsonRoot, "id"));
	const char* pszGps = json_object_get_string(json_object_object_get(pJsonRoot, "gps"));
	const char* pszAddr = json_object_get_string(json_object_object_get(pJsonRoot, "addr"));

	BOOL bRet = DataBaseMedia_UpdateItemGpsAddr(iItemID, pszGps, pszAddr);
	if(FALSE == bRet)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_UPDATEGPSADDR_ACK, "update failed");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
	else
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_UPDATEGPSADDR_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	}
}
void ComputerServerMessage_OnAboutDevices(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	long iTotal = 0;
    long iUsed = 0;
    Tools_GetDiskInfo(&iTotal, &iUsed);
	//获取设备名称 版本号
	DevItem* pItem = DataBaseDevice_GetDevInfo();
	if(NULL == pItem)
	{
		char szBuffer[255] = {0};
		sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_ABOUTDEVICE_ACK, "inner error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
		return;
	}
	
	//取照片总数量
	//int iRecordCount = 0;
	//DataBaseMedia_GetRecordCount(&iRecordCount);
	char* pszMac = Tools_GetMacAddr();
	char szBuffer[255] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"space\":%ld,\"deviceid\":\"%s\",\"devversion\":\"%s\",\"wifi\":%d,\"eth\":%d,\"samba\":%d,\"mac\":\"%s\"}", MESSAGETYPECMD_ABOUTDEVICE_ACK, iTotal, pItem->pszDevID, pItem->pszDevVersion, g_bWlan == FALSE?0:1 , g_bEth == FALSE?0:1, g_bSamba == FALSE?0:1, pszMac);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	DataBaseDevice_ReleaseItem(pItem);
	free(pszMac);
	pszMac = NULL;
}

void ComputerServerMessage_OnGetBrType(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	BRTYPE brType = NetWorkUtil_GetBrtype();
	char szBuffer[100] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1,\"brtype\":%d}", MESSAGETYPECMD_GETBRTYPE_ACK, brType);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnSetBrType(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	BRTYPE brType = json_object_get_int(json_object_object_get(pJsonRoot, "brtype"));
	
	char szBuffer[100] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_SETBRTYPE_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
	//设置了br以后设备就可能访问不到了 这里先返回在处理
	NetWorkUtil_SetBrtype(brType);
}

void ComputerServerMessage_OnGetWifiItems(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//获取无线设备
	struct json_object* pScanItems = NetWorkDriver_GetWifiScanItems();
	struct json_object* pJsonRet = json_object_new_object();
	json_object_object_add(pJsonRet, "otype", json_object_new_int(MESSAGETYPECMD_GETWIFIITEMS_ACK)); 
	json_object_object_add(pJsonRet, "status", json_object_new_int(1)); 
	json_object_object_add(pJsonRet, "items", pScanItems); 
	const char* pszJson = json_object_to_json_string(pJsonRet);
	//printf("%s\n", pszJson);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszJson);
	json_object_put(pJsonRet);
	
	// int iRetLen = strlen(pItems) + 100;
	// char* pRetBuffer = malloc(iRetLen);
	// memset(pRetBuffer, 0, iRetLen);
	// sprintf(pRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":%s}", MESSAGETYPECMD_GETWIFIITEMS_ACK, pItems);
	// free(pItems);
	// pItems = NULL;
	// ComputerServerMessage_SendCmdMessage(pServer, iSocket, pRetBuffer);
	
	// free(pRetBuffer);
	// pRetBuffer = NULL;
}

void ComputerServerMessage_OnGetStoreWifiItems(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	struct json_object* pAllItems = NetWorkUtil_GetWlan1StoreItems();
	struct json_object* pJsonRet = json_object_new_object();
	json_object_object_add(pJsonRet, "otype", json_object_new_int(MESSAGETYPECMD_GETSTOREWIFIITEMS_ACK)); 
	json_object_object_add(pJsonRet, "status", json_object_new_int(1));
	json_object_object_add(pJsonRet, "items", pAllItems);
	const char* pszRet = json_object_to_json_string(pJsonRet);
	printf("%s\n", pszRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRet);
	// int iBufferLen = strlen(pszStoreItems) + 100;
	// char* pRetBuffer = malloc(iBufferLen);
	// memset(pRetBuffer, 0, iBufferLen);
	// sprintf(pRetBuffer, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_GETSTOREWIFIITEMS_ACK, pszStoreItems);
	// ComputerServerMessage_SendCmdMessage(pServer, iSocket, pRetBuffer);
	// free(pRetBuffer);
	// pRetBuffer = NULL;
	// free(pszStoreItems);
	// pszStoreItems = NULL;
}

void ComputerServerMessage_OnConnectWifiItem(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	if(NetWorkUtil_GetBrtype() == BRTYPE_WLAN1)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_CONNECTWIFIITEM_ACK, "wlan1 br open");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	const char* pszSsid = json_object_get_string(json_object_object_get(pJsonRoot, "ssid"));
	const char* pszPasswd = json_object_get_string(json_object_object_get(pJsonRoot, "passwd"));
	int iNetWorkID = json_object_get_int(json_object_object_get(pJsonRoot, "networkid"));
	int iHidSsid = json_object_get_int(json_object_object_get(pJsonRoot, "hidssid"));
	BOOL bFromNetWorkID = TRUE;
	if(NULL != pszSsid && strlen(pszSsid) > 0)
	{
		iNetWorkID = NetWorkUtil_AddWifiItem(pszSsid, pszPasswd, iHidSsid);
		bFromNetWorkID = FALSE;
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"networkid\":\"%d\"}", MESSAGETYPECMD_CONNECTWIFIITEM_ACK, iNetWorkID);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	//先返回 不然连接状态改变了 消息就发不出去了
	if(bFromNetWorkID)
	{
		NetWorkDriver_ConnectNetWork(iNetWorkID, pszPasswd);
	}
	else
	{
		//如果不是从networkid配置的 上面AddWifiItem已经配置过密码了 这里不需要在配置了
		NetWorkDriver_ConnectNetWork(iNetWorkID, "");
	}
}

void ComputerServerMessage_OnUnConnectWifiItem(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	//int iNetWorkIndex = json_object_get_int(json_object_object_get(pJsonRoot, "networkid"));
	NetWorkDriver_DisConnectNetWork();
	char szBuffer[100] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_UNCONNECTWIFIITEM_ACK, 1);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnDeleteStoreWifiItem(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iNetWorkIndex = json_object_get_int(json_object_object_get(pJsonRoot, "networkid"));
	BOOL bRet = NetWorkUtil_RemoveStoreWifiItem(iNetWorkIndex);
	char szBuffer[100] = {0};
	sprintf(szBuffer, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_DELSTOREWIFIITEM_ACK, bRet == TRUE?1:0);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szBuffer);
}

void ComputerServerMessage_OnGetHotPot(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	char szSsid[MAX_SSID_LEN] = {0};
    char szPasswd[MAX_WIFIPASSWORDLEN] = {0};
	BOOL bRet = NetWorkUtil_GetWifiName(szSsid, szPasswd);
	if(bRet)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"ssid\":\"%s\",\"pwd\":\"%s\"}", MESSAGETYPECMD_GETHOTPOT_ACK, "", "");
		struct json_object* pRetJson = json_tokener_parse(szRet);
		json_object_object_del(pRetJson, "ssid"); 
		json_object_object_add(pRetJson, "ssid", json_object_new_string(szSsid)); 
		json_object_object_del(pRetJson, "pwd"); 
		json_object_object_add(pRetJson, "pwd", json_object_new_string(szPasswd)); 
		const char* pszRetJson = json_object_to_json_string(pRetJson);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetJson);
		json_object_put(pRetJson);
	}
	else
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_GETHOTPOT_ACK, "get hotpot error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	}
}
void ComputerServerMessage_OnSetHotPot(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszSsid = json_object_get_string(json_object_object_get(pJsonRoot, "ssid"));
	const char* pszPasswd = json_object_get_string(json_object_object_get(pJsonRoot, "passwd"));
	if(strlen(pszSsid) == 0 || strlen(pszPasswd) < 8)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"input error\"}", MESSAGETYPECMD_SETHOTPOT_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_SETHOTPOT_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);

	NetWorkUtil_SetHotPot(pszSsid, pszPasswd);
}
void ComputerServerMessage_OnGetWifiStatus(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	char szSsid[MAX_SSID_LEN] = {0};
    char szIpAddr[MAX_IPADDR_LEN] = {0};
	int iNetWorkID = 0;
	int iWlan1Status = NetWorkDriver_GetWlan1Status(szSsid, szIpAddr, &iNetWorkID);
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"ssid\":\"%s\",\"networkid\":%d,\"ipaddr\":\"%s\",\"wlan1status\":%d}", MESSAGETYPECMD_GETWIFISTATUS_ACK, "", iNetWorkID, szIpAddr, iWlan1Status);
	struct json_object* pRetJson = json_tokener_parse(szRet);
	json_object_object_del(pRetJson, "ssid"); 
	json_object_object_add(pRetJson, "ssid", json_object_new_string(szSsid)); 
	const char* pszRetJson = json_object_to_json_string(pRetJson);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRetJson);
	json_object_put(pRetJson);
}

void ComputerServerMessage_OnSetCover(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszCover = json_object_get_string(json_object_object_get(pJsonRoot, "coveraddr"));
	//2021/IMG_20210412_072616.jpg
	BOOL bRet = DataBaseCover_SetHomeCover(pszCover);
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_SETCOVER_ACK, bRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
}

void ComputerServerMessage_GetYearMonthDayCover(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iYear = json_object_get_int(json_object_object_get(pJsonRoot,"year"));
	int iMonth = json_object_get_int(json_object_object_get(pJsonRoot,"month"));
	int iDay = json_object_get_int(json_object_object_get(pJsonRoot,"day"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	char szCover[MAX_PATH] = {0};
	BOOL bRet = DataBaseMedia_YearMonthDayCover(iYear, iMonth, iDay, pszDevice, szCover);
	free(pszDevice);
	pszDevice = NULL;
	if (FALSE == bRet)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_MEDIAYEARMONTHDAYCOVER_ACK, "opt db error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	if (0 == strlen(szCover))
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_MEDIAYEARMONTHDAYCOVER_ACK, "no data find");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	char szRet[500] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"cover\":\"%s\",\"year\":%d,\"month\":%d,\"day\":%d}", MESSAGETYPECMD_MEDIAYEARMONTHDAYCOVER_ACK, szCover, iYear, iMonth, iDay);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
}
void ComputerServerMessage_ClearCacheStart(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	if (NULL != g_pClearCache && g_pClearCache->eStatus == CLEARCACHE_PROCESS)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_MEDIACLEARCACHESTART_ACK);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	BOOL bRet = ClearCache_Start(&g_pClearCache); 
	if (FALSE == bRet)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_MEDIACLEARCACHESTART_ACK, "start failed");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1}", MESSAGETYPECMD_MEDIACLEARCACHESTART_ACK);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
}

void ComputerServerMessage_ClearCacheProcess(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	int iPrecent = 100;
	CLEARCACHESTATUS eStatus = CLEARCACHE_SUCCESS;
	if(NULL != g_pClearCache)
	{
		iPrecent = g_pClearCache->iPrecent;
		eStatus = g_pClearCache->eStatus;
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"pstatus\":%d,\"precent\":%d}", MESSAGETYPECMD_MEDIACLEARCACHEPROCESS_ACK, eStatus, iPrecent);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);	
}

void ComputerServerMessage_GroupInfo(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	int iEmptyFilter = json_object_get_int(json_object_object_get(pJsonRoot, "emptyfilter"));
	char* pszAllGroup = NULL;
	if(NULL == pszDeviceName || strlen(pszDeviceName) == 0)
	{
		pszAllGroup = DataBaseGroup_GetJsonAllGroups();
	}
	else
	{
		char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
		char* pszGids = DataBaseGroupItems_GidsFromDevNames(pszDevice);
		free(pszDevice);
		pszDevice = NULL;
		free(pszGids);
		pszGids = NULL;
		pszAllGroup = DataBaseGroup_GetJsonAllGroups(pszGids);
		free(pszGids);
		pszGids = NULL;
	}
	
	if(iEmptyFilter)
	{
		int iTmpLen = strlen(pszAllGroup) + 3;
		char* pszTmp = malloc(iTmpLen);
		memset(pszTmp, 0, iTmpLen);
		sprintf(pszTmp, "[%s]", pszAllGroup);
		free(pszAllGroup);
		pszAllGroup = NULL;
		json_object* pJsonItems = json_tokener_parse(pszTmp);
		free(pszTmp);
		pszTmp = NULL;
		int iItemLen = json_object_array_length(pJsonItems);
		char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
		for (int i = 0; i < iItemLen; ++i)
		{
			struct json_object* pItem = json_object_array_get_idx(pJsonItems, i); 
			int iGid = json_object_get_int(json_object_object_get(pItem, "id"));
			uint32_t iPicCount = 0;
			uint32_t iVideoCount = 0;
			DataBaseGroupItems_Detail(iGid, pszDevice, &iPicCount, &iVideoCount);
			if(iPicCount != 0 || iVideoCount != 0)
			{
				const char* psz = json_object_get_string(pItem);
				if(NULL == pszAllGroup)
				{
					int iLen = strlen(psz) + 1;
					pszAllGroup = malloc(iLen);
					memset(pszAllGroup, 0, iLen);
					strcpy(pszAllGroup, psz);
				}
				else
				{
					int iLen = strlen(pszAllGroup) + strlen(psz) + 2;
					pszAllGroup = realloc(pszAllGroup, iLen);
					strcat(pszAllGroup, ",");
					strcat(pszAllGroup, psz);
				}
			}
		}
		free(pszDevice);
		pszDevice = NULL;
		json_object_put(pJsonItems);
	}
	if(NULL == pszAllGroup)
	{
		pszAllGroup = malloc(1);
		memset(pszAllGroup, 0, 1);
	}
	int iLen = strlen(pszAllGroup) + 100;
	char* pszRet = malloc(iLen);
	memset(pszRet, 0, iLen);
	sprintf(pszRet, "{\"otype\":\"%d\",\"status\":1,\"items\":[%s]}", MESSAGETYPECMD_GROUPINFO_ACK, pszAllGroup);
	printf("%s\n", pszRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRet);	
	free(pszAllGroup);
	pszAllGroup = NULL;
	free(pszRet);
	pszRet = NULL;
}

void ComputerServerMessage_GroupItemDetail(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	printf("ComputerServerMessage_GroupItemDetail\n");
	uint32_t iGid = json_object_get_int64(json_object_object_get(pJsonRoot, "id"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));
	char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
	//取piccount videocount
	uint32_t iPicCount = 0;
	uint32_t iVideoCount = 0;
	BOOL bRet = DataBaseGroupItems_Detail(iGid, pszDevice, &iPicCount, &iVideoCount);
	if(FALSE == bRet)
	{
		free(pszDevice);
		pszDevice = NULL;
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":0,\"info\":\"%s\"}", MESSAGETYPECMD_GROUPITEMDETAIL_ACK, "get pic video count error");
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
		return;
	}
	free(pszDevice);
	pszDevice = NULL;
	//取cover
	char* pszCover = DataBaseGroup_GetCover(iGid);
	//取name
	char* pszName = DataBaseGroup_GroupNameFromID(iGid);
	
	if(strlen(pszCover) > 0)
	{
		char szCoverFile[MAX_PATH] = {0};
		sprintf(szCoverFile, "%s/%s", FOLD_PREFIX, pszCover);
		BOOL bCover = FileUtil_CheckFileExist(szCoverFile);
		if(FALSE == bCover)
		{
			DataBaseGroup_SetCoverEmpty(pszCover);
			free(pszCover);
			pszCover = malloc(1);
			memset(pszCover, 0, 1);
		}
	}
	
	char szRet[300] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":1,\"gid\":%d,\"name\":\"%s\",\"piccount\":%d,\"videocount\":%d,\"cover\":\"%s\"}", 
		MESSAGETYPECMD_GROUPITEMDETAIL_ACK, iGid, pszName, iPicCount, iVideoCount, pszCover);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	free(pszCover);
	pszCover = NULL;
	free(pszName);
	pszName = NULL;
}

void ComputerServerMessage_GroupAdd(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	printf("ComputerServerMessage_GroupAdd\n");
	const char* pszGroupName = json_object_get_string(json_object_object_get(pJsonRoot, "name"));
	printf("%s\n", pszGroupName);
	uint32_t iGroupID = DataBaseGroup_AddGroup(pszGroupName);
	if(iGroupID == 0)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_GROUPADD_ACK, 0);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);	
	}
	else
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d,\"id\":%d,\"name\":\"%s\"}", MESSAGETYPECMD_GROUPADD_ACK, 1, iGroupID, pszGroupName);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);	
	}
}

void ComputerServerMessage_GroupUpdate(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pszGroupName = json_object_get_string(json_object_object_get(pJsonRoot, "name"));
	uint32_t iGroupID = json_object_get_int64(json_object_object_get(pJsonRoot, "id"));
	BOOL bRet = DataBaseGroup_GroupItemUpdate(iGroupID, pszGroupName);
	if(FALSE == bRet)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d, \"id\":%d}", MESSAGETYPECMD_GROUPUPDATE_ACK, 0, iGroupID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	}
	else
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d, \"id\":%d}", MESSAGETYPECMD_GROUPUPDATE_ACK, 1, iGroupID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	}
}

void ComputerServerMessage_GroupDelete(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	uint32_t iGroupID = json_object_get_int64(json_object_object_get(pJsonRoot, "id"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devnames"));

	BOOL bRet = DataBaseGroupItems_RemoveFromGroupID(iGroupID);
	if(bRet)
	{
		char* pszDevice = Tools_ReplaceString((char*)pszDeviceName, "&", "','");
		int iMediaCount = DataBaseGroupItems_MediaItemCount(iGroupID, pszDevice);
		free(pszDevice);
		pszDevice = NULL;
		if(iMediaCount == 0)
		{
			bRet = DataBaseGroup_RemoveItemFromID(iGroupID);
		}
	}
	if(FALSE == bRet)
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d, \"id\":%d}", MESSAGETYPECMD_GROUPDEL_ACK, 0, iGroupID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	}
	else
	{
		char szRet[200] = {0};
		sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d, \"id\":%d}", MESSAGETYPECMD_GROUPDEL_ACK, 1, iGroupID);
		ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	}
}

void ComputerServerMessage_GroupNamesFromItemID(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	uint32_t iItemID = json_object_get_int64(json_object_object_get(pJsonRoot, "id"));
	char* pszGroupNames = DataBaseMedia_GetGroupInfoFomItemID(iItemID);
	int iRetLen = strlen(pszGroupNames) + 200;
	char* pszRet = malloc(iRetLen);
	memset(pszRet, 0, iRetLen);
	sprintf(pszRet, "{\"otype\":\"%d\",\"status\":%d, \"groups\":[%s]}", MESSAGETYPECMD_GROUPNAMESFROMITEMID_ACK, 1, pszGroupNames);
	printf("%s\n", pszRet);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, pszRet);
	free(pszRet);
	pszRet = NULL;
	free(pszGroupNames);
	pszGroupNames = NULL;
}

void ComputerServerMessage_MediaItemGroupsSetting(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	
	uint32_t iItemID = json_object_get_int64(json_object_object_get(pJsonRoot, "itemid"));
	const char* pszDeviceName = json_object_get_string(json_object_object_get(pJsonRoot, "devname"));
	const char* pszMediaType = json_object_get_string(json_object_object_get(pJsonRoot, "mediatype"));
	const char* pszMediaAddr = json_object_get_string(json_object_object_get(pJsonRoot, "mediaaddr"));
	printf("ComputerServerMessage_MediaItemGroupsSetting:%s\n", pszDeviceName);
	DataBaseGroupItems_RemoveFromItemID(iItemID);
	
	int iBufferLen = sizeof(DataBaseGroupItem);
	char* pszBuffer = malloc(iBufferLen);
	memset(pszBuffer, 0, iBufferLen);
	DataBaseGroupItem* pItem = (DataBaseGroupItem*)pszBuffer;

	pItem->iItemID = iItemID;
	pItem->iItemType = atoi(pszMediaType);
	pItem->pszDeviceIdentify = (char*)pszDeviceName;

	DataBaseGroup_SetCoverEmpty(pszMediaAddr);
	json_object* pGroupsArray = json_object_object_get(pJsonRoot, "groups");
	for(int i = 0; i < json_object_array_length(pGroupsArray); ++i) 
	{
		struct json_object* pItemObj = json_object_array_get_idx(pGroupsArray, i);
		uint32_t iGid = json_object_get_int(json_object_object_get(pItemObj, "gid"));
		int iCover = json_object_get_int(json_object_object_get(pItemObj, "cover"));
		if(iCover == 1)
		{
			DataBaseGroup_SetCover(iGid, pszMediaAddr);
		}
		pItem->iGID = iGid;
		DataBaseGroupItems_AddItem(pItem);
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_MEDIAITEMGROUPSETTING_ACK, 1);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	free(pszBuffer);
	pszBuffer = NULL;
}

void ComputerServerMessage_GroupNameFromID(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	uint32_t iGid = json_object_get_int64(json_object_object_get(pJsonRoot, "id"));
	char* pszName = DataBaseGroup_GroupNameFromID(iGid);
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d,\"name\":\"%s\"}", MESSAGETYPECMD_GROUPNAMEFROMID_ACK, 1, pszName);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
	free(pszName);
	pszName = NULL;
}

void ComputerServerMessage_MediaItemsGroupAdd(struct ConnectServer* pServer, SOCKET iSocket, json_object* pJsonRoot)
{
	const char* pItemIds = json_object_get_string(json_object_object_get(pJsonRoot, "itemids"));
	const char* pGroupIds = json_object_get_string(json_object_object_get(pJsonRoot, "groupids"));
	char* pItemIdsTemp = NULL;
	char* pszItemIds = (char*)pItemIds;
	char* pszItemId = (char*)strtok_r(pszItemIds, "&", &pItemIdsTemp);
	while (NULL != pszItemId)
	{
		char* pGroupIdsTemp = NULL;
		char* pszGroupIds = (char*)pGroupIds;
		char* pszGroupId = (char*)strtok_r(pszGroupIds, "&", &pGroupIdsTemp);
		while (NULL != pszGroupId)
		{
			uint32_t iGID = atol(pszGroupId);
			uint32_t iItemID = atol(pszItemId);
			
			MediaItem* pMediaItem = DataBaseMedia_GetItemByID(iItemID);
			int iBufferLen = sizeof(DataBaseGroupItem);
			char* pszBuffer = malloc(iBufferLen);
			memset(pszBuffer, 0, iBufferLen);
			DataBaseGroupItem* pItem = (DataBaseGroupItem*)pszBuffer;

			pItem->iItemID = iItemID;
			pItem->iItemType = pMediaItem->iMediaType;
			pItem->pszDeviceIdentify = (char*)pMediaItem->pszDeviceIdentify;
			pItem->iGID = iGID;
			DataBaseGroupItems_AddItem(pItem);
			free(pszBuffer);
			pszBuffer = NULL;
			DataBaseMedia_ReleaseItem(pMediaItem);
			
			pszGroupId = strtok_r(NULL, "&", &pGroupIdsTemp);
		}
		pszItemId = strtok_r(NULL, "&", &pItemIdsTemp);
	}
	char szRet[200] = {0};
	sprintf(szRet, "{\"otype\":\"%d\",\"status\":%d}", MESSAGETYPECMD_MEDIAITEMSGROUPADD_ACK, 1);
	ComputerServerMessage_SendCmdMessage(pServer, iSocket, szRet);
}
void ComputerServerMessage_OnComputerServerCmd(struct ConnectServer* pServer, SOCKET iSocket, char* pszBuffer)
{
	printf("%s\n", pszBuffer);
	json_object* pJsonRoot = json_tokener_parse(pszBuffer);
	int iOtype = json_object_get_int(json_object_object_get(pJsonRoot,"otype"));
	switch (iOtype)
	{
		case MESSAGETYPECMD_CHECKFILE:
		{
			ComputerServerMessage_OnComputerServerCheckFile(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_CHECKMD5:
		{
			ComputerServerMessage_OnComputerServerCheckMd5(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UPLOADFILE:
		{
			ComputerServerMessage_OnComputerServerUploadFile(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_REPORTINFO:
		{
			ComputerServerMessage_OnComputerServerReportInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_DEVICEINFO:
		{
			ComputerServerMessage_OnComputerServerDeviceInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_DISKINFO:
		{
			ComputerServerMessage_OnComputerServerDiskInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_LOGIN:
		{
			ComputerServerMessage_OnComputerServerLogin(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UPDATEDEVICETIME:
		{
			ComputerServerMessage_OnComputerServerUpdateDeviceTime(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_PWDTIP:
		{
			ComputerServerMessage_OnComputerServerPwdTip(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_RESETUSER:
		{
			ComputerServerMessage_OnComputerServerResetUser(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UPDATEDEVICEINFO:
		{
			ComputerServerMessage_OnComputerServerUpdateDeviceInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_SERVERITEMS:
		{
			ComputerServerMessage_OnComputerServerServerItems(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAITEMINFO:
		{
			ComputerServerMessage_OnComputerServerMediaItemInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAITEMDEL:
		{
			ComputerServerMessage_OnComputerServerMediaItemDelete(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAITEMFAVORITE:
		{
			ComputerServerMessage_OnComputerServerMediaItemFavorite(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAYEARLIST:
		{
			ComputerServerMessage_OnComputerServerMediaYearList(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAMONTHLIST:
		{
			//月份列表
			ComputerServerMessage_OnComputerServerMediaMonthList(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIADAYSLIST:
		{
			//日列表
			ComputerServerMessage_OnComputerServerMediaDayList(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAYEARINFO:
		{
			ComputerServerMessage_OnComputerServerMediaYearInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIADEVNAMES:
		{
			ComputerServerMessage_OnComputerServerDevNames(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIARECENTINFO:
		{
			ComputerServerMessage_OnComputerServerRecentInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAFAVORITEINFO:
		{
			ComputerServerMessage_OnComputerServerMediaFavoriteInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAUNCHECKGPS:
		{
			//获取数据库Location为空的GPS类别^分割
			ComputerServerMessage_OnComputerServerMediaUnCheckGps(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIANUPDATEGPSWEIZHI:
		{
			//通过GPS更新位置
			ComputerServerMessage_OnComputerServerMediaUpdateGpsWeiZhi(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_YEARLOCATIONGROUP:
		{
			//一年内的相片按照位置分组
			ComputerServerMessage_OnComputerServerMediaYearLocationGroup(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_YEARLOCATIONGROUPTONGJI:
		{
			ComputerServerMessage_OnComputerServerMediaYearLocationGroupTongJi(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UPDATEPAISHETIME:
		{
			ComputerServerMessage_OnUpdatePaiSheTime(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UPDATEGPSADDR:
		{
			ComputerServerMessage_OnUpdateGpsAddr(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_ABOUTDEVICE:
		{
			ComputerServerMessage_OnAboutDevices(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GETBRTYPE:
		{
			//读取br信息
			ComputerServerMessage_OnGetBrType(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_SETBRTYPE:
		{
			//设置br信息
			ComputerServerMessage_OnSetBrType(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GETWIFIITEMS:
		{
			//获取wifi设备
			ComputerServerMessage_OnGetWifiItems(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GETSTOREWIFIITEMS:
		{
			//获取存储的wifi设备
			ComputerServerMessage_OnGetStoreWifiItems(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_CONNECTWIFIITEM:
		{
			//中继到指定热点
			ComputerServerMessage_OnConnectWifiItem(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GETHOTPOT:
		{
			//获取热点信息
			ComputerServerMessage_OnGetHotPot(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_SETHOTPOT:
		{
			//设置热点信息
			ComputerServerMessage_OnSetHotPot(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GETWIFISTATUS:
		{
			//获取无线wlan1状态
			ComputerServerMessage_OnGetWifiStatus(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_UNCONNECTWIFIITEM:
		{
			//断开wlan1设备连接
			ComputerServerMessage_OnUnConnectWifiItem(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_DELSTOREWIFIITEM:
		{
			//删除wlan口配置项
			ComputerServerMessage_OnDeleteStoreWifiItem(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_SETCOVER:
		{
			//设置封面
			ComputerServerMessage_OnSetCover(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAYEARMONTHDAYCOVER:
		{
			//获取当天的图片封面
			ComputerServerMessage_GetYearMonthDayCover(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIACLEARCACHESTART:
		{
			//开始清理缓存
			ComputerServerMessage_ClearCacheStart(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIACLEARCACHEPROCESS:
		{
			//查询清理缓存进度
			ComputerServerMessage_ClearCacheProcess(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPINFO:
		{
			//获取分组信息
			ComputerServerMessage_GroupInfo(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPITEMDETAIL:
		{
			//获取分组详情
			ComputerServerMessage_GroupItemDetail(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPADD:
		{
			//添加分组信息
			ComputerServerMessage_GroupAdd(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPUPDATE:
		{
			//更新分组信息
			ComputerServerMessage_GroupUpdate(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPDEL:
		{
			//删除分组信息
			ComputerServerMessage_GroupDelete(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPNAMESFROMITEMID:
		{
			//获取ITEM的信息配置
			ComputerServerMessage_GroupNamesFromItemID(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAITEMGROUPSETTING:
		{
			//媒体信息配置
			ComputerServerMessage_MediaItemGroupsSetting(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_GROUPNAMEFROMID:
		{
			//GROUPID获取分组名称
			ComputerServerMessage_GroupNameFromID(pServer, iSocket, pJsonRoot);
			break;
		}
		case MESSAGETYPECMD_MEDIAITEMSGROUPADD:
		{
			//给照片和视频批量添加分组
			ComputerServerMessage_MediaItemsGroupAdd(pServer, iSocket, pJsonRoot);
			break;
		}
		case 999:
		{
			ComputerServerMessage_OnTest(pServer, iSocket, pJsonRoot);
			break;
		}
	}
	json_object_put(pJsonRoot);
}

void OnComputerServerMessage(struct ConnectServer* pServer, BaseMsg* pMsg, SOCKET iSocket)
{
	InnerBase* pBase = (InnerBase*)pMsg->pszBuf;
	switch (pBase->iMsgType)
	{
		case MESSAGETYPE_CMD:
		{
			ComputerServerMessage_OnComputerServerCmd(pServer, iSocket, pBase->szBuf);
			break;
		}
		default:
		{
			break;
		}
	}
}

void ComputerServerContectedConn(struct ConnectServer* pServer, SOCKET iSocket)
{
	//printf("connect con:%ld\n", iSocket);
}

void ComputerServerUnContectedConn(struct ConnectServer* pServer, SOCKET iSocket)
{
	//printf("UnContectedConn con:%ld\n", iSocket);
}

