#include "ConnectBroadCast.h"
#include <vector>
#include <string>
#include <cstring>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
CConnectBroadCast::CConnectBroadCast()
{
    m_iSocketFd = 0;
	m_hRecvProcHandle = NULL;
	m_hMessageHandle = NULL;
    InitializeCriticalSection(&m_MsgQueueSection);
	InitializeCriticalSection(&m_SocketSection);
}

CConnectBroadCast::~CConnectBroadCast()
{
    Stop();
    DeleteCriticalSection(&m_MsgQueueSection);
	DeleteCriticalSection(&m_SocketSection);
}
BOOL CConnectBroadCast::Start(int iPort, OnConnectBroadCastMessageFun callBack, LPVOID param)
{
	Stop();
	m_OnMessageProc = callBack;
	m_Param = param;
	BOOL bRet = Init(iPort);
	if(FALSE == bRet)
	{
		return FALSE;
	}
	
	m_hRecvProcHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvProc, this, 0, NULL);
	m_hMessageHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MsgProc, this, 0, NULL);
    return TRUE;
}
void CConnectBroadCast::Stop()
{
    if(0 == m_iSocketFd)
    {
        return;
    }
    shutdown(m_iSocketFd, SD_BOTH);
    closesocket(m_iSocketFd);
    m_iSocketFd = 0;
    if(NULL != m_hRecvProcHandle)
    {
        WaitForSingleObject(m_hRecvProcHandle, INFINITE);
        CloseHandle(m_hRecvProcHandle);
        m_hRecvProcHandle = NULL;
    }
    if(NULL != m_hMessageHandle)
    {
        WaitForSingleObject(m_hMessageHandle, INFINITE);
        CloseHandle(m_hMessageHandle);
        m_hMessageHandle = NULL;
    }
    while (m_pMsgList.size() > 0)
    {
        struct BaseMsg* pBaseMsg = m_pMsgList.front();
        m_pMsgList.pop_front();
        if(NULL == pBaseMsg)
		{
			continue;
		}
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
    
}
BOOL CConnectBroadCast::SendMessage(const char* pszSendBuff, DWORD dwLen, struct sockaddr_in* pSocket)
{
    if (0 == m_iSocketFd)
	{
		return FALSE;
	}
	//LOG(INFO) << pSendBuff;	
	DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
	char* pBuffer = (char*)malloc(dwMsgLen);
	memset(pBuffer, 0, dwMsgLen);
	struct BaseMessage* pMsg = (struct BaseMessage*)pBuffer;
	pMsg->iDataLen = dwLen;
	memcpy(pMsg->szBuf, pszSendBuff, pMsg->iDataLen);

	//防止多线程发送数据错乱
	EnterCriticalSection(&m_SocketSection);
	int iLen = sendto(m_iSocketFd, pBuffer, dwMsgLen, 0, (const struct sockaddr *)pSocket, sizeof(struct sockaddr));
	LeaveCriticalSection(&m_SocketSection);
	//LOGI("CBaseBroadCast::SendMessage sendlen:%d", iLen);
	free(pBuffer);
	pBuffer = NULL;

	if (iLen <= 0)
	{
		return FALSE;
	}
	return TRUE;
}

#ifndef __IOS_
list<string> CConnectBroadCast::GetBroadCastIps()
{
    list<string> retList;
    retList.push_back("255.255.255.255");
	int iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if(iSocketFd <= 0)
	{
		return retList;
	}

	struct ifreq ifReqBuffer[16];
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ifReqBuffer);
    ifc.ifc_buf = (caddr_t) ifReqBuffer;
	
	if (0 != ioctl (iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
	{
		return retList;
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
			retList.push_back(szBuffer);
	}
	return retList;
}

std::list<std::string> ConnectBroadCast_GetLocalMacs()
{
    std::list<std::string> retList;
	int iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	if(iSocketFd <= 0)
	{
		return retList;
	}
	struct ifreq ifReqBuffer[16];
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ifReqBuffer);
    ifc.ifc_buf = (caddr_t) ifReqBuffer;
	
	if (0 != ioctl (iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
	{
		return retList;
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
			retList.push_back(szMac);
		}
	}
	return retList;
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

string CConnectBroadCast::BroadCastOne(const char* pszSendBuff, DWORD dwLen, int iPort, int iTimeOut, char* pszRetIp, int* piRetPort)
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
	char* pszBuffer = (char*)malloc(dwMsgLen);
	memset(pszBuffer, 0, dwMsgLen);
	struct BaseMessage* pMsg = (struct BaseMessage*)pszBuffer;
	pMsg->iDataLen = dwLen;
	memcpy(pMsg->szBuf, pszSendBuff, pMsg->iDataLen);

    list<string> broadCastIpList = GetBroadCastIps();
    for(list<string>::iterator itor = broadCastIpList.begin(); itor != broadCastIpList.end(); ++itor)
    {
        struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr((*itor).c_str());
		//LOGI("%s", inet_ntoa(addr.sin_addr));
		addr.sin_port = htons(iPort);
		int iAddrLen = sizeof(addr);
		int iSendLen = sendto(iSocketFd, pszBuffer, dwMsgLen, 0, (struct sockaddr*)&addr, iAddrLen);
		if(iSendLen <= 0)
		{
			continue;
		}
		char szRecvBuffer[MAX_MESSAGE_LEN] = { 0 };
		struct sockaddr_in ClientAddr;
		iAddrLen = sizeof(ClientAddr);
		int iRecvLen = recvfrom(iSocketFd, szRecvBuffer, MAX_MESSAGE_LEN, 0, (struct sockaddr*)&ClientAddr, (socklen_t*)&iAddrLen);
		if (iRecvLen > 0)
		{
			char* pszRetBuffer = (char*)malloc(iRecvLen + 1);
			memcpy(pszRetBuffer, szRecvBuffer, iRecvLen);
			if (NULL != pszRetIp)
			{
				strcpy(pszRetIp, inet_ntoa(ClientAddr.sin_addr));
			}
			if (NULL != piRetPort)
			{
				*piRetPort = ClientAddr.sin_port;
			}

			free(pszBuffer);
			pszBuffer = NULL;
            string strRetBuffer(pszRetBuffer);
            free(pszRetBuffer);
            pszRetBuffer = NULL;
			return strRetBuffer;
		}
    }
	free(pszBuffer);
	pszBuffer = NULL;
    return "";
}
void CConnectBroadCast::BroadCast(const char* pszSendBuff, DWORD dwLen, int iPort)
{
    DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
    char* pszBuffer = (char*)malloc(dwMsgLen);
    memset(pszBuffer, 0, dwMsgLen);
    struct BaseMessage* pMsg = (struct BaseMessage*)pszBuffer;
    pMsg->iDataLen = dwLen;
    memcpy(pMsg->szBuf, pszSendBuff, pMsg->iDataLen);

    list<string> broadCastIpList = GetBroadCastIps();
    list<string>::iterator itor = broadCastIpList.begin();
    for(; itor != broadCastIpList.end(); ++itor)
    {
        printf("BroadCast Addr:%s\n", (*itor).c_str());
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr((*itor).c_str());
        //LOGI("%s", inet_ntoa(addr.sin_addr));
        addr.sin_port = htons(iPort);
        int iAddrLen = sizeof(addr);
        int iSendLen = sendto(m_iSocketFd, pszBuffer, dwMsgLen, 0, (struct sockaddr*)&addr, iAddrLen);
        if(iSendLen <= 0)
        {
            printf("BroadCast %s failed\n", (*itor).c_str());
        }
    }
    free(pszBuffer);
    pszBuffer = NULL;
}
void CConnectBroadCast::BroadCastNotifyOnLine(int iPort)
{
    int iIdentifyLen = strlen(CASTTAG);
	int iSendLen = sizeof(InnerBase) + iIdentifyLen;
	char* pSendBuffer = (char*)malloc(iSendLen);
	memset(pSendBuffer, 0, iSendLen);
	InnerBase* pInner = (InnerBase*)pSendBuffer;
	pInner->iMsgType = CASTMSGTYPE_GETIDENTIFYIP_ACK;
	pInner->iMsgLen = iIdentifyLen;
	memcpy(pInner->szBuf, CASTTAG, iIdentifyLen);
	BroadCast(pSendBuffer, iSendLen, iPort);
	free(pSendBuffer);
	pSendBuffer = NULL;
}
BOOL CConnectBroadCast::Init(int iPort)
{
	// 创建socket
	m_iSocketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == m_iSocketFd)
	{
		printf("error! error code is %d/n", errno);
		return FALSE;
	}

	BOOL bOpt = TRUE;
	//设置该套接字为广播类型
	setsockopt(m_iSocketFd, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	int iOn = 1;
	setsockopt(m_iSocketFd, SOL_SOCKET, SO_REUSEADDR, (const char *)&iOn, sizeof(iOn));

	//将sock绑定到本机某端口上。 
	struct sockaddr_in local;
	// int iLen = sizeof(struct sockaddr_in);
	local.sin_family = AF_INET;
	local.sin_port = htons(iPort);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(m_iSocketFd, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
	{
		printf("bind failed with:%d \n", errno);
		return FALSE;
	}

	return TRUE;
}
void CConnectBroadCast::AddToMsgList( char * pszRecvBuff, char* pszAddr)
{
    if (NULL == pszRecvBuff )
    {
        return;
    }

    struct BaseMessage* pBaseMsg = (struct BaseMessage*)pszRecvBuff;
	
	int iDataLen = sizeof(struct BaseMessage) - 1 + pBaseMsg->iDataLen;
	char* pDataBuffer = (char*)malloc(iDataLen);
	memcpy(pDataBuffer, pszRecvBuff, iDataLen);

	int iMsgLen = sizeof(struct BaseMsg);
	char* pBuffer = (char*)malloc(iMsgLen);
	memset(pBuffer, 0, iMsgLen);

	struct BaseMsg* pMsg = (struct BaseMsg*)pBuffer;
	pMsg->iVersion = pBaseMsg->iVersion;
	pMsg->iDataLen = pBaseMsg->iDataLen;
	pMsg->pExtra = pszAddr;
	pMsg->pszBuf = ((struct BaseMessage*)pDataBuffer)->szBuf;
	pMsg->pBaseMessage = (struct BaseMessage*)pDataBuffer;

	EnterCriticalSection(&m_MsgQueueSection);
	m_pMsgList.push_back(pMsg);
	LeaveCriticalSection(&m_MsgQueueSection);
}
DWORD CConnectBroadCast::RecvProc(LPVOID* lpParameter)
{
	CConnectBroadCast* pConnectBroadCast = (CConnectBroadCast*)lpParameter;
	char szRecvBuffer[MAX_MESSAGE_LEN] = { 0 };
	while (pConnectBroadCast->m_iSocketFd > 0)
	{
		memset(szRecvBuffer, 0, MAX_MESSAGE_LEN);
		ssize_t iRet = 0; 
		struct sockaddr_in from;
		int iAddrLen = sizeof(struct sockaddr);

		if ((iRet = recvfrom(pConnectBroadCast->m_iSocketFd, szRecvBuffer, MAX_MESSAGE_LEN, 0, (struct sockaddr*)&from, (socklen_t*)&iAddrLen)) == SOCKET_ERROR)
		{
			printf("recvfrom failed with:%d\n", errno);
			return -1;
		}
		if (((size_t)iRet) < sizeof(struct BaseMessage))
		{
			continue;
		}
		printf("found IP is %s\n", inet_ntoa(from.sin_addr));
		char* pszBuffer = (char*)malloc(iAddrLen);
		memset(pszBuffer, 0, iAddrLen);
		memcpy(pszBuffer, (char*)&from, iAddrLen);
		pConnectBroadCast->AddToMsgList(szRecvBuffer, pszBuffer);
	}

	return 1;
}

DWORD CConnectBroadCast::MsgProc(LPVOID* lpParameter)
{
	CConnectBroadCast* pConnectBroadCast = (CConnectBroadCast*)lpParameter;
	while(pConnectBroadCast->m_iSocketFd > 0)
	{
        EnterCriticalSection(&pConnectBroadCast->m_MsgQueueSection);
        if(pConnectBroadCast->m_pMsgList.size() == 0)
        {
            LeaveCriticalSection(&pConnectBroadCast->m_MsgQueueSection);
            Sleep(10);
			continue;
        }
        struct BaseMsg* pBaseMsg = pConnectBroadCast->m_pMsgList.front();
        if(NULL == pBaseMsg)
		{
            LeaveCriticalSection(&pConnectBroadCast->m_MsgQueueSection);
			Sleep(10);
			continue;
		}
        pConnectBroadCast->m_pMsgList.pop_front();
        LeaveCriticalSection(&pConnectBroadCast->m_MsgQueueSection);

		pConnectBroadCast->m_OnMessageProc(pBaseMsg, pBaseMsg->iSockfd, pConnectBroadCast->m_Param);
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
	return 1;
}