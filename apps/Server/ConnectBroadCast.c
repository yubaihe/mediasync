#include "Tools.h"
#include "ConnectBroadCast.h"
#include "ConnectBroadCastMessage.h"
void ConnectBroadCast_CloseConn(struct ConnectBroadCast* pConnectBroadCast)
{
	if (pConnectBroadCast->iSocketFd > 0)
	{
		shutdown(pConnectBroadCast->iSocketFd, SD_BOTH);
		closesocket(pConnectBroadCast->iSocketFd);
		pConnectBroadCast->iSocketFd = 0;
	}
}

void ConnectBroadCast_Close(SOCKET iSocket)
{
    if(iSocket > 0)
    {
        shutdown(iSocket, SD_BOTH);
        closesocket(iSocket);
        iSocket = 0;
    }
}

BOOL ConnectBroadCast_Init(struct ConnectBroadCast* pConnectBroadCast, int iPort)
{
	// 创建socket
	pConnectBroadCast->iSocketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == pConnectBroadCast->iSocketFd)
	{
		printf("error! error code is %d/n", errno);
		return FALSE;
	}

	BOOL bOpt = TRUE;
	//设置该套接字为广播类型
	setsockopt(pConnectBroadCast->iSocketFd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	int iOn = 1;
	setsockopt(pConnectBroadCast->iSocketFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&iOn, sizeof(iOn));

	//将sock绑定到本机某端口上。 
	struct sockaddr_in local;
	// int iLen = sizeof(struct sockaddr_in);
	local.sin_family = AF_INET;
	local.sin_port = htons(iPort);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(pConnectBroadCast->iSocketFd, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind failed with:%d \n", errno);
		ConnectBroadCast_CloseConn(pConnectBroadCast);
		return FALSE;
	}

	return TRUE;
}

struct BaseMsg* ConnectBroadCast_GetMsg(struct ConnectBroadCast* pConnectBroadCast)
{
	EnterCriticalSection(&pConnectBroadCast->MsgQueueSection);
	struct BaseMsg* pMsg = RelechQueue_Front(pConnectBroadCast->pMsgQueue);
	RelechQueue_PopFront(pConnectBroadCast->pMsgQueue);
	LeaveCriticalSection(&pConnectBroadCast->MsgQueueSection);
	return pMsg;
}


void ConnectBroadCast_AddToMsgList(struct ConnectBroadCast* pConnectBroadCast, char * pRecvBuff, char* pAddr)
{
    if (NULL == pRecvBuff )
    {
        return;
    }

    struct BaseMessage* pBaseMsg = (struct BaseMessage*)pRecvBuff;
	
	int iDataLen = sizeof(struct BaseMessage) - 1 + pBaseMsg->iDataLen;
	char* pDataBuffer = (char*)malloc(iDataLen);
	memcpy(pDataBuffer, pRecvBuff, iDataLen);

	int iMsgLen = sizeof(struct BaseMsg);
	char* pBuffer = (char*)malloc(iMsgLen);
	memset(pBuffer, 0, iMsgLen);

	struct BaseMsg* pMsg = (struct BaseMsg*)pBuffer;
	pMsg->iVersion = pBaseMsg->iVersion;
	pMsg->iDataLen = pBaseMsg->iDataLen;
	pMsg->pExtra = pAddr;
	pMsg->pszBuf = ((struct BaseMessage*)pDataBuffer)->szBuf;
	pMsg->pBaseMessage = (struct BaseMessage*)pDataBuffer;

	EnterCriticalSection(&pConnectBroadCast->MsgQueueSection);
	RelechQueue_PushBack(pConnectBroadCast->pMsgQueue, pMsg);
	LeaveCriticalSection(&pConnectBroadCast->MsgQueueSection);
}

DWORD ConnectBroadCast_RecvProc(LPVOID* lpParameter)
{
	struct ConnectBroadCast* pConnectBroadCast = (struct ConnectBroadCast*)lpParameter;
	char szRecvBuffer[MAX_DATALEN] = { 0 };
	while (pConnectBroadCast->iSocketFd > 0)
	{
		memset(szRecvBuffer, 0, MAX_DATALEN);
		int iRet = 0; 
		struct sockaddr_in from;
		int iAddrLen = sizeof(struct sockaddr);

		if ((iRet = recvfrom(pConnectBroadCast->iSocketFd, szRecvBuffer, MAX_DATALEN, 0, (struct sockaddr*)&from, (socklen_t*)&iAddrLen)) == SOCKET_ERROR)
		{
			//printf("recvfrom failed with:%d\n", errno);
			ConnectBroadCast_CloseConn(pConnectBroadCast);
			return -1;
		}
		if (iRet < sizeof(struct BaseMessage))
		{
			continue;
		}
		printf("found IP is %s\n", inet_ntoa(from.sin_addr));
		char* pBuffer = (char*)malloc(iAddrLen);
		memset(pBuffer, 0, iAddrLen);
		memcpy(pBuffer, (char*)&from, iAddrLen);
		ConnectBroadCast_AddToMsgList(pConnectBroadCast, szRecvBuffer, pBuffer);
	}

	return 1;
}

DWORD ConnectBroadCast_MsgProc(LPVOID* lpParameter)
{
	struct ConnectBroadCast* pConnectBroadCast = (struct ConnectBroadCast*)lpParameter;
	while(pConnectBroadCast->iSocketFd > 0)
	{
		struct BaseMsg* pBaseMsg = ConnectBroadCast_GetMsg(pConnectBroadCast);
		if(NULL == pBaseMsg)
		{
			Sleep(10);
			continue;
		}

		pConnectBroadCast->OnMessageProc(pConnectBroadCast, pBaseMsg, pBaseMsg->iSockfd);
		if(NULL != pBaseMsg->pExtra)
		{
			free((char*)pBaseMsg->pExtra);
			pBaseMsg->pExtra = NULL;
		}
		
		free((char*)pBaseMsg->pBaseMessage);
		pBaseMsg->pBaseMessage = NULL;
		free((char*)pBaseMsg);
		pBaseMsg = NULL;
	}

	EnterCriticalSection(&pConnectBroadCast->MsgQueueSection);
	while(pConnectBroadCast->pMsgQueue->iItemLen > 0)
	{
		struct BaseMsg* pMsg = RelechQueue_Front(pConnectBroadCast->pMsgQueue);
		RelechQueue_PopFront(pConnectBroadCast->pMsgQueue);
		free((char*)pMsg);
		pMsg = NULL;
	}
	RelechQueue_Clear(pConnectBroadCast->pMsgQueue);
	LeaveCriticalSection(&pConnectBroadCast->MsgQueueSection);
	return 1;
}

BOOL ConnectBroadCast_SendMessage(struct ConnectBroadCast* pConnectBroadCast, const char* pSendBuff, DWORD dwLen, struct sockaddr_in* pSocket)
{
	if (0 == pConnectBroadCast->iSocketFd)
	{
		return FALSE;
	}
	//LOG(INFO) << pSendBuff;	
	DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
	char* pBuffer = (char*)malloc(dwMsgLen);
	memset(pBuffer, 0, dwMsgLen);
	struct BaseMessage* pMsg = (struct BaseMessage*)pBuffer;
	pMsg->iDataLen = dwLen;
	memcpy(pMsg->szBuf, pSendBuff, pMsg->iDataLen);

	//防止多线程发送数据错乱
	EnterCriticalSection(&pConnectBroadCast->SocketSection);
	int iLen = sendto(pConnectBroadCast->iSocketFd, pBuffer, dwMsgLen, 0, (const struct sockaddr *)pSocket, sizeof(struct sockaddr));
	LeaveCriticalSection(&pConnectBroadCast->SocketSection);
	//LOGI("CBaseBroadCast::SendMessage sendlen:%d", iLen);
	free(pBuffer);
	pBuffer = NULL;

	if (iLen <= 0)
	{
		return FALSE;
	}
	return TRUE;

}


struct ConnectBroadCast* ConnectBroadCast_Start(int iPort)
{
	struct ConnectBroadCast* pConnectBroadCast = (struct ConnectBroadCast*)malloc(sizeof(struct ConnectBroadCast));
	BOOL bRet = ConnectBroadCast_Init(pConnectBroadCast, iPort);
	if(FALSE == bRet)
	{
		free((char*)pConnectBroadCast);
		pConnectBroadCast = NULL;
		return NULL;
	}
	InitializeCriticalSection(&pConnectBroadCast->MsgQueueSection);
	InitializeCriticalSection(&pConnectBroadCast->SocketSection);
	pConnectBroadCast->pMsgQueue = RelechQueue_Init();
	pConnectBroadCast->hRecvProcHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectBroadCast_RecvProc, pConnectBroadCast, 0, NULL);
	pConnectBroadCast->hMessageHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectBroadCast_MsgProc, pConnectBroadCast, 0, NULL);
	return pConnectBroadCast;
}

void ConnectBroadCast_Stop(struct ConnectBroadCast* pConnectBroadCast)
{
	ConnectBroadCast_CloseConn(pConnectBroadCast);
	
	if(NULL != pConnectBroadCast->hRecvProcHandle)
	{
		WaitForSingleObject(pConnectBroadCast->hRecvProcHandle, INFINITE);
		CloseHandle(pConnectBroadCast->hRecvProcHandle);
		pConnectBroadCast->hRecvProcHandle = NULL;
	}
	if(NULL != pConnectBroadCast->hMessageHandle)
	{
		WaitForSingleObject(pConnectBroadCast->hMessageHandle, INFINITE);
		CloseHandle(pConnectBroadCast->hMessageHandle);
		pConnectBroadCast->hMessageHandle = NULL;
	}

	DeleteCriticalSection(&pConnectBroadCast->MsgQueueSection);
	DeleteCriticalSection(&pConnectBroadCast->SocketSection);
	free((char*)pConnectBroadCast);
	pConnectBroadCast = NULL;
}
#ifndef __IOS_
char* ConnectBroadCast_GetBroadCastIps()
{
	int iMallocLen = strlen("255.255.255.255&");
	char* pRet = (char*)malloc(iMallocLen + 1);
	memset(pRet, 0, iMallocLen + 1);
	strcpy(pRet, "255.255.255.255&");
	
	int iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if(iSocketFd <= 0)
	{
		int iIndex = strlen(pRet) - 1;
		char* pBuffer = pRet + iIndex;
		pBuffer[0] = '\0';
		return pRet;
	}

	struct ifreq ifReqBuffer[16];
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ifReqBuffer);
    ifc.ifc_buf = (caddr_t) ifReqBuffer;
	
	if (0 != ioctl (iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
	{
		int iIndex = strlen(pRet) - 1;
		char* pBuffer = pRet + iIndex;
		pBuffer[0] = '\0';
		return pRet;
	}

	int iLen = ifc.ifc_len / sizeof (struct ifreq);
	for(int i = 0; i < iLen; ++i)
	{
		char szIpAddr[16] = {0};
		char szMask[16] = {0};
		if (0 == (ioctl (iSocketFd, SIOCGIFADDR, (char *) &ifReqBuffer[i]))) 
		{
			  //printf ("IP: %s\n", inet_ntoa(((struct sockaddr_in*)(&ifReqBuffer[i].ifr_addr))->sin_addr));
			 strcpy(szIpAddr, inet_ntoa(((struct sockaddr_in*)(&ifReqBuffer[i].ifr_addr))->sin_addr));			               
		}
		if(0 == strcmp(szIpAddr, "127.0.0.1"))
		{
			continue;
		}
		if (0 == (ioctl (iSocketFd, SIOCGIFNETMASK, (char *) &ifReqBuffer[i]))) 
		{
			
			  //printf ("MASK %s\n", inet_ntoa(((struct sockaddr_in*)(&ifReqBuffer[i].ifr_netmask))->sin_addr));
			  strcpy(szMask, inet_ntoa(((struct sockaddr_in*)(&ifReqBuffer[i].ifr_netmask))->sin_addr));	
		}

		struct sockaddr_in broadcast;
			broadcast.sin_addr.s_addr = (inet_addr(szIpAddr)& 
								inet_addr(szMask))
								| (~inet_addr(szMask));
		
			char szBuffer[20] = {0};
			sprintf(szBuffer, "%s&", inet_ntoa(broadcast.sin_addr));
			printf("IP:%s MASK:%s\n", szIpAddr, szMask);
			int iOldLen = strlen(pRet);
			pRet = (char*)realloc(pRet, iOldLen + strlen(szBuffer) + 1);
			strcpy(pRet + iOldLen, szBuffer);
	}
	if(strlen(pRet) > 0)
	{
		int iIndex = strlen(pRet) - 1;
		char* pBuffer = pRet + iIndex;
		pBuffer[0] = '\0';
	}
	return pRet;
}

char* ConnectBroadCast_GetLocalMacs()
{
	char* pRet = (char*)malloc(1);
	memset(pRet, 0, 1);
	int iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if(iSocketFd <= 0)
	{
		return pRet;
	}
	struct ifreq ifReqBuffer[16];
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ifReqBuffer);
    ifc.ifc_buf = (caddr_t) ifReqBuffer;
	
	if (0 != ioctl (iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
	{
		int iIndex = strlen(pRet) - 1;
		char* pBuffer = pRet + iIndex;
		pBuffer[0] = '\0';
		return pRet;
	}

	int iLen = ifc.ifc_len / sizeof (struct ifreq);
	for(int i = 0; i < iLen; ++i)
	{
		if (0 == (ioctl (iSocketFd, SIOCGIFHWADDR, (char *) &ifReqBuffer[i]))) 
		{
			char szMac[19] = {0};
			 sprintf(szMac, "%02x:%02x:%02x:%02x:%02x:%02x&",
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[0],
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[1],
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[2],
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[3],
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[4],
                        (unsigned char)ifReqBuffer[i].ifr_hwaddr.sa_data[5]);
			 if(0 == strcmp(szMac, "00:00:00:00:00:00&"))
			 {
			 	continue;
			 }
			int iOldLen = strlen(pRet);
			pRet = (char*)realloc(pRet, iOldLen + strlen(szMac) + 1);
			strcpy(pRet + iOldLen, szMac);
		}
	}
	if(strlen(pRet) > 0)
	{
		int iIndex = strlen(pRet) - 1;
		char* pBuffer = pRet + iIndex;
		pBuffer[0] = '\0';
	}
	return pRet;
}
#else
/*
//以下内容写在.m文件中
//在IOS里面这个文件对部分机型不适配,广播不出去
+(NSString *)GetIPAddress
{
    NSString *pstrAddrs = @"";
    struct ifaddrs *pIfAddrs = NULL;
    struct ifaddrs *pTmpIfAddrs = NULL;
    int iSuccess = 0;
    iSuccess = getifaddrs(&pIfAddrs);
    if (iSuccess == 0)
    {
        pTmpIfAddrs = pIfAddrs;
        while(pTmpIfAddrs != NULL)
        {
            if(pTmpIfAddrs->ifa_addr->sa_family == AF_INET)
            {
                //if([[NSString stringWithUTF8String:pTmpIfAddrs->ifa_name] isEqualToString:@"en0"])
                //NSLog(@"子网掩码:%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)pTmpIfAddrs->ifa_netmask)->sin_addr)]);
                //NSLog(@"本地IP:%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)pTmpIfAddrs->ifa_addr)->sin_addr)]);
                //NSLog(@"广播地址:%@",[NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)pTmpIfAddrs->ifa_dstaddr)->sin_addr)]);
                NSString* pstrBroadCastAddr = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)pTmpIfAddrs->ifa_dstaddr)->sin_addr)];
               pstrAddrs = [pstrAddrs stringByAppendingString:[NSString stringWithFormat:@"%@&", pstrBroadCastAddr]];
            }
            pTmpIfAddrs = pTmpIfAddrs->ifa_next;
        }
    }
    
    freeifaddrs(pIfAddrs);
    if(pstrAddrs.length > 0)
    {
        pstrAddrs = [pstrAddrs substringWithRange:NSMakeRange(0, pstrAddrs.length - 1)];
    }
    return pstrAddrs;
}

char* ConnectBroadCast_GetBroadCastIps()
{
    NSString* pAddr = [MediaSyncNetWork GetIPAddress];
    const char * pRet =[pAddr UTF8String];
    long iLen = strlen(pRet) + 1;
    char* pBuffer = malloc(iLen);
    memset(pBuffer, 0, iLen);
    strcpy(pBuffer, pRet);
    return pBuffer;
}*/
#endif

char* ConnectBroadCast_BroadCastOne(const char* pSendBuff, DWORD dwLen, int iPort, int iTimeOut, char* pRetIp, int* piRetPort)
{
	SOCKET iSocketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	BOOL bOpt = TRUE;
	//设置该套接字为广播类型
	setsockopt(iSocketFd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	int iOn = 1;
	setsockopt(iSocketFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&iOn, sizeof(iOn));

	struct timeval timeout = { iTimeOut,0 };//1s
	setsockopt(iSocketFd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

	DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
	char* pBuffer = (char*)malloc(dwMsgLen);
	memset(pBuffer, 0, dwMsgLen);
	struct BaseMessage* pMsg = (struct BaseMessage*)pBuffer;
	pMsg->iDataLen = dwLen;
	memcpy(pMsg->szBuf, pSendBuff, pMsg->iDataLen);

	char* pBroadCastIps = ConnectBroadCast_GetBroadCastIps();
	printf("BroadCast Addr:%s\n", pBroadCastIps);
	char* pTemp = NULL;
	char* pszBroadCastIps = pBroadCastIps;
	char *pRet = (char*)strtok_r(pszBroadCastIps, "&", &pTemp);
	while(NULL != pRet)
	{
	
		printf("BroadCast Addr step:%s\n", pRet);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(pRet);
		//LOGI("%s", inet_ntoa(addr.sin_addr));
		addr.sin_port = htons(iPort);
		int iAddrLen = sizeof(addr);
		int iSendLen = sendto(iSocketFd, pBuffer, dwMsgLen, 0, (struct sockaddr*)&addr, iAddrLen);
		if(iSendLen <= 0)
		{
			continue;
		}
		char szRecvBuffer[65535] = { 0 };
		struct sockaddr_in ClientAddr;
		iAddrLen = sizeof(ClientAddr);
		int iRecvLen = recvfrom(iSocketFd, szRecvBuffer, 65535, 0, (struct sockaddr*)&ClientAddr, (socklen_t*)&iAddrLen);
		char* pRetBuffer = NULL;
		if (iRecvLen > 0)
		{
			pRetBuffer = (char*)malloc(iRecvLen);
			memcpy(pRetBuffer, szRecvBuffer, iRecvLen);
			if (NULL != pRetIp)
			{
				strcpy(pRetIp, inet_ntoa(ClientAddr.sin_addr));
			}
			if (NULL != piRetPort)
			{
				*piRetPort = ClientAddr.sin_port;
			}

			free(pBuffer);
			pBuffer = NULL;
			free(pBroadCastIps);
			pBroadCastIps = NULL;
			return pRetBuffer;
		}
		pRet = strtok_r(NULL, "&", &pTemp);
	}
	free(pBuffer);
	pBuffer = NULL;
	free(pBroadCastIps);
	pBroadCastIps = NULL;
	return NULL;
}

void ConnectBroadCast_BroadCast(struct ConnectBroadCast* pConnectBroadCast, const char* pSendBuff, DWORD dwLen, int iPort)
{
    DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
    char* pBuffer = (char*)malloc(dwMsgLen);
    memset(pBuffer, 0, dwMsgLen);
    struct BaseMessage* pMsg = (struct BaseMessage*)pBuffer;
    pMsg->iDataLen = dwLen;
    memcpy(pMsg->szBuf, pSendBuff, pMsg->iDataLen);

    char* pBroadCastIps = ConnectBroadCast_GetBroadCastIps();
    printf("BroadCast Addr:%s\n", pBroadCastIps);
    char* pTemp = NULL;
	char* pszBroadCastIps = pBroadCastIps;
    char *pRet = strtok_r(pszBroadCastIps, "&", &pTemp);
    while(NULL != pRet)
    {
        printf("BroadCast Addr step:%s\n", pRet);
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(pRet);
        //LOGI("%s", inet_ntoa(addr.sin_addr));
        addr.sin_port = htons(iPort);
        int iAddrLen = sizeof(addr);
        int iSendLen = sendto(pConnectBroadCast->iSocketFd, pBuffer, dwMsgLen, 0, (struct sockaddr*)&addr, iAddrLen);
        if(iSendLen <= 0)
        {
            printf("BroadCast %s failed\n", pRet);
        }
        pRet = strtok_r(NULL, "&", &pTemp);
    }
    free(pBuffer);
    pBuffer = NULL;
    free(pBroadCastIps);
    pBroadCastIps = NULL;
}

void ConnectBroadCast_BroadCastNotifyOnLine(struct ConnectBroadCast* pConnectBroadCast, int iPort)
{
	int iIdentifyLen = strlen(CASTTAG);
	int iSendLen = sizeof(InnerBase) + iIdentifyLen;
	char* pSendBuffer = (char*)malloc(iSendLen);
	memset(pSendBuffer, 0, iSendLen);
	InnerBase* pInner = (InnerBase*)pSendBuffer;
	pInner->iMsgType = CASTMSGTYPE_GETIDENTIFYIP_ACK;
	pInner->iMsgLen = iIdentifyLen;
	memcpy(pInner->szBuf, CASTTAG, iIdentifyLen);
	ConnectBroadCast_BroadCast(pConnectBroadCast, pSendBuffer, iSendLen, iPort);
	free(pSendBuffer);
	pSendBuffer = NULL;
}