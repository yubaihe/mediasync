
#include "ConnectServer.h"
#include "Tools.h"
#include <arpa/inet.h>   // for sockaddr_in

DWORD ConnectServer_ServerProc(LPVOID* lpParameter);
DWORD ConnectServer_ConnRecvProc(LPVOID* lpParameter);
DWORD ConnectServer_TimerProc(LPVOID* lpParameter);
DWORD ConnectServer_MsgProc(LPVOID* lpParameter);
struct BaseMsg* ConnectServer_GetMsg(struct ConnectServer* pServer);
void ConnectServer_AddToMsgList(struct ConnectServer* pServer, char * pRecvBuff, SOCKET iSocketFd);
const int SOCKET_MAXCONNECT = 30;
int ConnectServer_SetConnNonBlock(SOCKET iSocketFd)
{
    int iOp;
    iOp = fcntl(iSocketFd,F_GETFL,0);
    fcntl(iSocketFd, F_SETFL, iOp|O_NONBLOCK);
    return iOp;
}

struct ConnectServer* ConnectServer_CreateSocket(int iPort)
{
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(iPort);

	SOCKET iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(iServerSocket <= 0)
    {
        printf("%s\n", "Create Socket Failed");
        //printf("Create Socket Failed!\n");
        return NULL;
    }
	int iValue = 1;
	setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, &iValue, sizeof(iValue));
	char* pBuffer = (char*)malloc(sizeof(struct ConnectServer));
	memset(pBuffer, 0, sizeof(struct ConnectServer));
	struct ConnectServer* pServer = (struct ConnectServer*)pBuffer;
	pServer->iServerSocket = iServerSocket;
	
	//端口复用
    int iOpt = 1;
    setsockopt(pServer->iServerSocket, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt));

	if(bind(pServer->iServerSocket,(const struct sockaddr*)&server_addr, (socklen_t)sizeof(server_addr)))
    {
        printf("Server Bind Port : Failed %d==>%d", iPort, errno);
		free(pBuffer);
		pBuffer = NULL;
		closesocket(pServer->iServerSocket);
		pServer->iServerSocket = -1;
        //printf("Server Bind Port : %d Failed!\n", iPort);
        return NULL;
    }

	if (listen(pServer->iServerSocket, SOCKET_MAXCONNECT) )
	{
		printf("Server Listen Failed!");
		free(pBuffer);
		pBuffer = NULL;
		closesocket(pServer->iServerSocket);
		pServer->iServerSocket = -1;
		return NULL;
	}

	//ConnectServer_SetConnNonBlock(pServer->iServerSocket);
	
	InitializeCriticalSection(&pServer->section);
	pServer->pConnMap = RelechMap_Init(20);
	
	InitializeCriticalSection(&pServer->MsgQueueSection);
	pServer->pMsgQueue = RelechQueue_Init();

	InitializeCriticalSection(&pServer->UnConnectQueueSection);
	pServer->pUnConnectQueue = RelechQueue_Init();
	
	return pServer;
}


struct ConnectServer* ConnectServer_Start(int iPort)
{
	struct ConnectServer* pConnectServer = ConnectServer_CreateSocket(iPort);
	if (NULL == pConnectServer)
    {
        //socket创建失败了
        printf("Create Server Failed\n");
        return NULL;
    }
	//printf("socket create success\n");

	pConnectServer->hServerHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectServer_ServerProc, pConnectServer, 0, NULL);
    pConnectServer->hTimerHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectServer_TimerProc, pConnectServer, 0, NULL);
	pConnectServer->hMessageHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectServer_MsgProc, pConnectServer, 0, NULL);
	return pConnectServer;
}

BOOL ConnectServer_GetLocalIpPort(struct ConnectServer* pConnectServer, char* pIpAddr, int* iPort)
{
    struct sockaddr addr;
    int iAddrLen = sizeof(addr);
    if (0 == getsockname(pConnectServer->iServerSocket, &addr, (socklen_t*)&iAddrLen))
    {
        if (addr.sa_family == AF_INET)
        {
            struct sockaddr_in*  pAddr = (struct sockaddr_in*)&addr;
            strcpy(pIpAddr, inet_ntoa(pAddr->sin_addr));
            if(0 == strcmp(pIpAddr, "0.0.0.0"))
            {
                strcpy(pIpAddr, "127.0.0.1");
            }
            *iPort = ntohs(pAddr->sin_port);
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void ConnectServer_Exit(char* pszIpAddr, int iPort)
{
    ////select模型不主动退出 作为客户端在连接下 那么select收到accept请求就能做退出处理了
    //printf("serveripaddr:%s port:%d", pszIpAddr, iPort);
    SOCKET TempSocket = socket(AF_INET, SOCK_STREAM, 0);
    int iVlaue = 1;
    setsockopt(TempSocket, SOL_SOCKET,SO_REUSEADDR, &iVlaue, sizeof(iVlaue));

    // connect server socket
    struct sockaddr_in addrServer;
    addrServer.sin_addr.s_addr = inet_addr(pszIpAddr);
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(iPort);

    //fcntl(TempSocket,F_SETFL,fcntl(TempSocket,F_GETFL,0)|O_NONBLOCK);//设置为非阻塞模式
    connect((SOCKET)TempSocket, (struct sockaddr *)&addrServer, sizeof(addrServer));
}


void ConnectServer_Stop(struct ConnectServer* pServer)
{
	if(NULL == pServer || pServer->iServerSocket == 0)
	{
		return;
	}

	char szIpAddr[16] = {0};
	int iPort = 0;
	ConnectServer_GetLocalIpPort(pServer, szIpAddr, &iPort);

	SOCKET iSocketFd = pServer->iServerSocket;
	pServer->iServerSocket = 0;

    ConnectServer_Exit(szIpAddr, iPort);
	shutdown(iSocketFd, SD_BOTH);
	closesocket(iSocketFd);
	if(NULL != pServer->hServerHandle)
	{
		WaitForSingleObject(pServer->hServerHandle, INFINITE);
		CloseHandle(pServer->hServerHandle);
		pServer->hServerHandle = NULL;
	}
	if(NULL != pServer->hTimerHandle)
	{
		WaitForSingleObject(pServer->hTimerHandle, INFINITE);
		CloseHandle(pServer->hTimerHandle);
		pServer->hTimerHandle = NULL;
	}
	if(NULL != pServer->hMessageHandle)
	{
		WaitForSingleObject(pServer->hMessageHandle, INFINITE);
		CloseHandle(pServer->hMessageHandle);
		pServer->hMessageHandle = NULL;
	}
	EnterCriticalSection(&pServer->section);
	while(pServer->pConnMap->iItemLen > 0)
	{
		RelechMapVector vc = RelechMap_GetHead(pServer->pConnMap);
		struct ConnectConn* pConn = (struct ConnectConn*)vc.pValue;
		EnterCriticalSection(&pConn->section);
		shutdown(pConn->iConnSocket, SD_BOTH);
		closesocket(pConn->iConnSocket);
		if(NULL != pConn->hConnRecvHandle)
		{
			WaitForSingleObject(pConn->hConnRecvHandle, INFINITE);
			CloseHandle(pConn->hConnRecvHandle);
			pConn->hConnRecvHandle = NULL;
		}
		RelechMap_EarseInt(pServer->pConnMap, pConn->iConnSocket);
	}
	LeaveCriticalSection(&pServer->section);
	RelechMap_ClearMap(&pServer->pConnMap);
	EnterCriticalSection(&pServer->UnConnectQueueSection);
	while(pServer->pUnConnectQueue->iItemLen > 0)
	{
		char* pszSocket = RelechQueue_Front(pServer->pUnConnectQueue);
		RelechQueue_PopFront(pServer->pUnConnectQueue);
		free(pszSocket);
		pszSocket = NULL;
	}
	RelechQueue_Clear(pServer->pUnConnectQueue);
	LeaveCriticalSection(&pServer->UnConnectQueueSection);
	
	DeleteCriticalSection(&pServer->section);
	DeleteCriticalSection(&pServer->MsgQueueSection);
	DeleteCriticalSection(&pServer->UnConnectQueueSection);
	
	free((char*)pServer);
	pServer = NULL;
}

DWORD ConnectServer_MsgProc(LPVOID* lpParameter)
{
	struct ConnectServer* pServer = (struct ConnectServer*)lpParameter;
	while(pServer->iServerSocket > 0)
	{
		struct BaseMsg* pBaseMsg = ConnectServer_GetMsg(pServer);
		if(NULL == pBaseMsg)
		{
			Sleep(10);
			continue;
		}

		pServer->OnMessageProc(pServer, pBaseMsg, pBaseMsg->iSockfd);
		free((char*)pBaseMsg->pBaseMessage);
		pBaseMsg->pBaseMessage = NULL;
		free((char*)pBaseMsg);
		pBaseMsg = NULL;
	}

	EnterCriticalSection(&pServer->MsgQueueSection);
	while(pServer->pMsgQueue->iItemLen > 0)
	{
		struct BaseMsg* pMsg = RelechQueue_Front(pServer->pMsgQueue);
		RelechQueue_PopFront(pServer->pMsgQueue);
		free((char*)pMsg);
		pMsg = NULL;
	}
	RelechQueue_Clear(pServer->pMsgQueue);
	LeaveCriticalSection(&pServer->MsgQueueSection);
	return 1;
}

DWORD ConnectServer_TimerProc(LPVOID* lpParameter)
{
	struct ConnectServer* pServer = (struct ConnectServer*)lpParameter;
	int iPrevTime = Tools_CurTimeSec();
	int iSpace = 5*60;
	while(pServer->iServerSocket > 0)
	{
		Sleep(2000);
		EnterCriticalSection(&pServer->UnConnectQueueSection);
		while(pServer->pUnConnectQueue->iItemLen > 0)
		{
			char* pszSocket = RelechQueue_Front(pServer->pUnConnectQueue);
			RelechQueue_PopFront(pServer->pUnConnectQueue);
			char* psz = NULL;
			uint64_t iSocketConn = strtoull(pszSocket, &psz, 10);
			free(pszSocket);
			pszSocket = NULL;

			EnterCriticalSection(&pServer->section);
			struct ConnectConn* pConn = RelechMap_LookUpInt(pServer->pConnMap, iSocketConn);
			if(NULL != pConn)
			{
				EnterCriticalSection(&pConn->section);
				shutdown(pConn->iConnSocket, SD_BOTH);
				closesocket(pConn->iConnSocket);
				if(NULL != pConn->hConnRecvHandle)
				{
					WaitForSingleObject(pConn->hConnRecvHandle, INFINITE);
					CloseHandle(pConn->hConnRecvHandle);
					pConn->hConnRecvHandle = NULL;
				}
	
				pServer->UnContectedConnProc(pServer, pConn->iConnSocket);
				RelechMap_EarseInt(pServer->pConnMap, pConn->iConnSocket);
			}

			LeaveCriticalSection(&pServer->section);
		}
		LeaveCriticalSection(&pServer->UnConnectQueueSection);

		if(Tools_CurTimeSec() - iPrevTime > iSpace)
		{
			iPrevTime = Tools_CurTimeSec();
			int iLastTime = Tools_CurTimeSec() - iSpace;
			EnterCriticalSection(&pServer->section);
			RelechMapVector vc = RelechMap_GetHead(pServer->pConnMap);
			do
			{		
				struct ConnectConn* pConn = (struct ConnectConn*)vc.pValue;
				if(NULL == pConn)
				{					
					break;
				}
				if(pConn->iLastAccessTime < iLastTime)
				{
					SOCKET iSocketFd = pConn->iConnSocket;
					//printf("remove con:%ld\n", iSocketFd);
					shutdown(iSocketFd, SD_BOTH);
					closesocket(iSocketFd);
					if(NULL != pConn->hConnRecvHandle)
					{
						WaitForSingleObject(pConn->hConnRecvHandle, INFINITE);
						CloseHandle(pConn->hConnRecvHandle);
						pConn->hConnRecvHandle = NULL;
					}
					RelechMap_EarseInt(pServer->pConnMap, pConn->iConnSocket);
					pServer->UnContectedConnProc(pServer, iSocketFd);
					
				}
			}
			while(RelechMap_GetNext(pServer->pConnMap, &vc) > 0);
			
			LeaveCriticalSection(&pServer->section);
		}
		
	}
	return 1;
}

DWORD ConnectServer_ServerProc(LPVOID* lpParameter)
{
	//pthread_detach(pthread_self());
	struct ConnectServer* pServer = (struct ConnectServer*)lpParameter;
	while(pServer->iServerSocket > 0)
	{
		struct sockaddr_in SocketConn;
        socklen_t iSocketLen = sizeof(SocketConn);
        SOCKET iSocketConn = accept(pServer->iServerSocket, (struct sockaddr *) &SocketConn, &iSocketLen);
		//printf("accept socket:%ld\n", iSocketConn);
		if(iSocketConn == SOCKET_ERROR)
		{
			//printf("%s\n", "accept error");
			continue;
		}
		if(pServer->iServerSocket == 0)
		{
			break;
		}
		struct ConnectConn conn;
		EnterCriticalSection(&pServer->section);
		RelechMap_SetAtInt(pServer->pConnMap, iSocketConn, &conn, sizeof(conn));
		struct ConnectConn* pConn = RelechMap_LookUpInt(pServer->pConnMap, iSocketConn);
		pConn->iConnSocket = iSocketConn;
		pConn->iLastAccessTime = Tools_CurTimeSec();
		InitializeCriticalSection(&pConn->section);
		pConn->pExtra = NULL;
		pConn->pNext = NULL;
		pConn->pServer = pServer;
		pServer->ContectedConnProc(pServer, iSocketConn);
		pConn->hConnRecvHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ConnectServer_ConnRecvProc, pConn, 0, NULL);
		LeaveCriticalSection(&pServer->section);
	}

	return 1;
}
void ConnectServer_CloseConn(struct ConnectConn* pConn)
{
	struct ConnectServer* pServer = pConn->pServer;
	EnterCriticalSection(&pServer->section);
	EnterCriticalSection(&pConn->section);
	shutdown(pConn->iConnSocket, SD_BOTH);
	closesocket(pConn->iConnSocket);
	if(NULL != pConn->hConnRecvHandle)
	{
		WaitForSingleObject(pConn->hConnRecvHandle, INFINITE);
		CloseHandle(pConn->hConnRecvHandle);
		pConn->hConnRecvHandle = NULL;
	}
	
	pServer->UnContectedConnProc(pServer, pConn->iConnSocket);
	RelechMap_EarseInt(pServer->pConnMap, pConn->iConnSocket);
	LeaveCriticalSection(&pServer->section);
}
DWORD ConnectServer_ConnRecvProc(LPVOID* lpParameter)
{
	//pthread_detach(pthread_self());
	struct ConnectConn* pConn = (struct ConnectConn*)lpParameter;
	char szRecvBuf[MAX_DATALEN] = { 0 };
	memset(szRecvBuf, 0, MAX_DATALEN);
	struct SocketLeftBuffer* pSocketLeft = NULL;
	//printf("conn connect:%ld\n", pConn->iConnSocket);
	while(pConn->iConnSocket > 0)
	{
		int iLen = recv(pConn->iConnSocket, szRecvBuf, MAX_DATALEN, 0);
		if (iLen <= 0)
		{
			
			if (NULL != pSocketLeft)
			{
				if (NULL != pSocketLeft->pszBuf)
				{
					free(pSocketLeft->pszBuf);
					pSocketLeft->pszBuf = NULL;
				}
				free(pSocketLeft);
				pSocketLeft = NULL;
			}
			break;
		}

		if (NULL != pSocketLeft)
		{
			DWORD dwTotal = iLen + pSocketLeft->dwDataLen;
			pSocketLeft->pszBuf = (char*)realloc(pSocketLeft->pszBuf, dwTotal);
			memcpy(pSocketLeft->pszBuf + pSocketLeft->dwDataLen, szRecvBuf, iLen);
			pSocketLeft->dwDataLen = dwTotal;
		}
		else
		{
			pSocketLeft = (struct SocketLeftBuffer*)malloc(sizeof(struct SocketLeftBuffer));

			pSocketLeft->pszBuf = (char*)malloc(iLen);
			memcpy(pSocketLeft->pszBuf, szRecvBuf, iLen);
			pSocketLeft->dwDataLen = iLen;
		}

		struct BaseMessage* pMsg = (struct BaseMessage*)pSocketLeft->pszBuf;
		if (pSocketLeft->dwDataLen == sizeof(struct BaseMessage) + pMsg->iDataLen - 1)
		{
			
			//就是一条消息
			char* pBuffer = (char*)malloc(pSocketLeft->dwDataLen);
			memcpy(pBuffer, pSocketLeft->pszBuf, pSocketLeft->dwDataLen);
			ConnectServer_AddToMsgList(pConn->pServer, pBuffer, pConn->iConnSocket);

			pSocketLeft->dwDataLen = 0;
			free(pSocketLeft->pszBuf);
			pSocketLeft->pszBuf = NULL;

			//free((char*)pSocketLeft);
			//pSocketLeft = NULL;
		}
		else
		{
			//有粘包的情况
			if (pSocketLeft->dwDataLen >= sizeof(struct BaseMessage))
			{
				DWORD iStartPos = 0;
				while (pSocketLeft->dwDataLen - iStartPos >= sizeof(struct BaseMessage))
				{
					struct BaseMessage* pMsgs = (struct BaseMessage*)(pSocketLeft->pszBuf + iStartPos);
					DWORD iNewMsgLen = sizeof(struct BaseMessage) - 1 + pMsgs->iDataLen;
					if (pSocketLeft->dwDataLen - iStartPos >= iNewMsgLen)
					{
						char* pBuffer = (char*)malloc(iNewMsgLen);
						memcpy(pBuffer, pSocketLeft->pszBuf + iStartPos, iNewMsgLen);
						ConnectServer_AddToMsgList(pConn->pServer, pBuffer, pConn->iConnSocket);
						iStartPos += iNewMsgLen;
					}
					else
					{
						//这里标识消息头够了 但是不足一条消息了
						break;
					}

				}

				if (pSocketLeft->dwDataLen - iStartPos == 0)
				{
					pSocketLeft->dwDataLen = 0;
					free(pSocketLeft->pszBuf);
					pSocketLeft->pszBuf = NULL;

					//free((char*)pSocketLeft);
					//pSocketLeft = NULL;
				}
				else if (pSocketLeft->dwDataLen - iStartPos > 0 && iStartPos > 0)
				{
					//消息不足
					int iDataLen = pSocketLeft->dwDataLen - iStartPos;
					char* pBuffer = (char*)malloc(iDataLen);
					memcpy(pBuffer, pSocketLeft->pszBuf + iStartPos, iDataLen);

					free(pSocketLeft->pszBuf);
					pSocketLeft->pszBuf = NULL;

					pSocketLeft->dwDataLen = iDataLen;
					pSocketLeft->pszBuf = pBuffer;
				}
			}
		}
	}

	if (NULL != pSocketLeft)
	{
		if (NULL != pSocketLeft->pszBuf)
		{
			free(pSocketLeft->pszBuf);
			pSocketLeft->pszBuf = NULL;
		}
		free(pSocketLeft);
		pSocketLeft = NULL;
	}
	if(pConn->pServer->iServerSocket > 0)
	{
		//ConnectServer_CloseConn(pConn);
		// EnterCriticalSection(&pConn->pServer->section);
		// pConn->iLastAccessTime = 0;
		// LeaveCriticalSection(&pConn->pServer->section);
		EnterCriticalSection(&pConn->pServer->UnConnectQueueSection);
		char* pszSocketFd = (char*)malloc(25);
		memset(pszSocketFd, 0, 25);
		sprintf(pszSocketFd, "%lul", pConn->iConnSocket); 
		RelechQueue_PushBack(pConn->pServer->pUnConnectQueue, pszSocketFd);
		LeaveCriticalSection(&pConn->pServer->UnConnectQueueSection);
	}
	
	return 1;
}

struct ConnectConn ConnectServer_GetConn(struct ConnectServer* pServer, SOCKET iSocket)
{
	EnterCriticalSection(&pServer->section);
	struct ConnectConn* pConn = RelechMap_LookUpInt(pServer->pConnMap, iSocket);
	struct ConnectConn conn;
	
	if(NULL != pConn)
	{
		memcpy(&conn, pConn, sizeof(struct ConnectConn));
	}
	else
	{
		conn.iConnSocket = 0;
		conn.iLastAccessTime = 0;
		conn.hConnRecvHandle = NULL;
		conn.pServer = NULL;
		conn.pNext = NULL;
		conn.pExtra = NULL;
	}
	LeaveCriticalSection(&pServer->section);
	return conn;
}


void ConnectServer_AddToMsgList(struct ConnectServer* pServer, char * pRecvBuff, SOCKET iSocketFd)
{
    if (NULL == pRecvBuff )
    {
        return;
    }

    struct BaseMessage* pBaseMsg = (struct BaseMessage*)pRecvBuff;

    int iMsgLen = sizeof(struct BaseMsg);
    char* pBuffer = (char*)malloc(iMsgLen);
    memset(pBuffer, 0, iMsgLen);
    struct BaseMsg* pMsg = (struct BaseMsg*)pBuffer;
    pMsg->iVersion = pBaseMsg->iVersion;
    pMsg->iDataLen = pBaseMsg->iDataLen;
    pMsg->iSockfd = iSocketFd;
    pMsg->pBaseMessage = pBaseMsg;
    pMsg->pszBuf = pBaseMsg->szBuf;

	EnterCriticalSection(&pServer->MsgQueueSection);
	RelechQueue_PushBack(pServer->pMsgQueue, pMsg);
	LeaveCriticalSection(&pServer->MsgQueueSection);
}

struct BaseMsg* ConnectServer_GetMsg(struct ConnectServer* pServer)
{
	EnterCriticalSection(&pServer->MsgQueueSection);
	struct BaseMsg* pMsg = RelechQueue_Front(pServer->pMsgQueue);
	RelechQueue_PopFront(pServer->pMsgQueue);
	LeaveCriticalSection(&pServer->MsgQueueSection);
	return pMsg;
}

void ConnectServer_UpdateAccessTime(struct ConnectServer* pServer, SOCKET iSocket)
{
	EnterCriticalSection(&pServer->section);
	struct ConnectConn* pConnItem = RelechMap_LookUpInt(pServer->pConnMap, iSocket);
	if(NULL != pConnItem)
	{
		pConnItem->iLastAccessTime = Tools_CurTimeSec();
	}
	LeaveCriticalSection(&pServer->section);
}

BOOL ConnectServer_SendMessage(struct ConnectServer* pServer, SOCKET iSocketFd, const char* pSendBuff, DWORD dwLen)
{
    EnterCriticalSection(&pServer->section);
	struct ConnectConn* pConn =  RelechMap_LookUpInt(pServer->pConnMap, iSocketFd);
    if (pConn == NULL)
    {
        //LOGI("连接不合法");
        LeaveCriticalSection(&pServer->section);
        return FALSE;
    }

    EnterCriticalSection(&pConn->section);
    LeaveCriticalSection(&pServer->section);
    DWORD dwMsgLen = sizeof(struct BaseMessage) - 1 + dwLen;
    //LOG(INFO) << dwMsgLen;
    char* pBuffer = (char*)malloc(dwMsgLen);
    memset(pBuffer, 0, dwMsgLen);
    struct BaseMessage* pMsg = (struct BaseMessage*)pBuffer;
    pMsg->iDataLen = dwLen;
    memcpy(pMsg->szBuf, pSendBuff, pMsg->iDataLen);
    //防止多线程发送数据错乱
    DWORD dwLeft = dwMsgLen;
	time_t iStartTime = 0;
    while (1)
    {
		if(iStartTime > 0 && (time(NULL) - iStartTime >= 3))
		{
			//3S time out
			break;
		}
        const char* pTmpBuffer = pBuffer + (dwMsgLen - dwLeft);
        int iLen = send(iSocketFd, pTmpBuffer, dwLeft, MSG_NOSIGNAL);
        if (iLen == -1)
        {
			if(iStartTime == 0)
			{
				iStartTime = time(NULL);
			}
            if(errno == EAGAIN  || errno == EWOULDBLOCK || errno == EINTR)//发送缓冲区已满，或者被中断了
            {
                //LOG(INFO)<<"send buffer full";
                continue;
            }
            else if (errno == ECONNRESET||errno == EPIPE||errno==ETIMEDOUT)
            {
                //对方已经断开了！！
                break;
            }
			else
			{
				break;
			}
        }
		else
		{
			iStartTime = 0;
		}
        dwLeft -= iLen;
        if(dwLeft == 0)
        {
            break;
        }

    }
    LeaveCriticalSection(&pConn->section);

    if (dwLeft != 0)
    {
        //LOGI("CBaseServer::SendMessage CloseConn");
        closesocket(iSocketFd);
        return FALSE;
    }

    free(pBuffer);
    pBuffer = NULL;
    return TRUE;
}

