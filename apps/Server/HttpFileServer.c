#include "HttpFileServer.h"


void HttpFileServer_SendResult(SOCKET iSocket, BOOL bSuccess)
{
    if (bSuccess)
    {
        char szBuffer[] = "HTTP/1.1 200\r\n" \
            "Content - Length: 0\r\n" \
            "Connection : keep - alive";
        send(iSocket, szBuffer, strlen(szBuffer), 0);
    }
    else
    {
        char szBuffer[] = "HTTP/1.1 500\r\n" \
            "Content - Length: 0\r\n" \
            "Connection : keep - alive";
        send(iSocket, szBuffer, strlen(szBuffer), 0);
    }
}
char* HttpFileServer_BinaryFind(char* pszDest, int iDestLen, char* pszSrc, int iSrcLen)
{
	int j = 0;
	for (int i = 0; i < iDestLen; ++i)
	{
		if(iDestLen - i < iSrcLen)
		{
			break;
		}
		for (j = 0; j < iSrcLen; ++j)
		{
			if (pszDest[i + j] != pszSrc[j])
			{
				break;
			}
		}
		if (j == iSrcLen)
		{
			return pszDest + i;
		}
	}
	return NULL;
}

BOOL HttpFileServer_PraseFile(char* pszBound, char* pszDestFold, SOCKET iSocketFd)
{
	//--pszboundaryIdentify\r\n
	//\r\n--pszboundaryIdentify--\r\n
	char* pszStartBoundary = NULL;
    char* pszEndBoundary = NULL;
    int iBoundAryLen = 0;
    int iBoundAryEndLen = 0;
	if(NULL != pszBound)
	{
		iBoundAryLen = strlen(pszBound);
		char* pszboundaryIdentify = malloc(iBoundAryLen + 1);
		memset(pszboundaryIdentify, 0, iBoundAryLen + 1);
		memcpy(pszboundaryIdentify, pszBound, iBoundAryLen);

		pszStartBoundary = (char*)malloc(iBoundAryLen + 10);
		memset(pszStartBoundary, 0, iBoundAryLen + 10);
		strcpy(pszStartBoundary, "--");
		strcat(pszStartBoundary, pszboundaryIdentify);	
		strcat(pszStartBoundary, "\r\n");
		iBoundAryLen = strlen(pszStartBoundary);

		pszEndBoundary = (char*)malloc(iBoundAryLen + 10);
		memset(pszEndBoundary, 0, iBoundAryLen + 10);
		strcpy(pszEndBoundary, "\r\n");
		strcat(pszEndBoundary, "--");
		strcat(pszEndBoundary, pszboundaryIdentify);
		strcat(pszEndBoundary, "--\r\n");
		iBoundAryEndLen = strlen(pszEndBoundary);

		free(pszboundaryIdentify);
		pszboundaryIdentify = NULL;
	}
    
    int iMinPackageLen = 100;
	char* pszRecvBuffer = (char*)malloc(iMinPackageLen);
	memset(pszRecvBuffer, 0, iMinPackageLen);

	int iDealBufferLen = iMinPackageLen * 100;
	char* pszDealBuffer = (char*)malloc(iDealBufferLen);
	memset(pszDealBuffer, 0, iDealBufferLen);
	char szFileName[MAX_PATH] = { 0 };
	FILE* pDestFile = NULL;
	int iStatus = -1;
	int iLeftLen = 0;
	
	while (1)
	{
		if (iStatus >= 0)
		{
			break;
		}
		memset(pszRecvBuffer, 0, iMinPackageLen);
		int iRecvLen = recv((int)iSocketFd, pszRecvBuffer, iMinPackageLen, 0);
        // int iRecvLen = FCGX_GetStr(pszRecvBuffer, iMinPackageLen, pStream);
		if (iRecvLen <= 0)
		{
			iStatus = 0;
			break;
		}
		
		if ((iLeftLen + iRecvLen) >= iDealBufferLen)
		{
			printf("go here iLeftLen:%d iRecvLen:%d iDealBufferLen:%d\n", iLeftLen, iRecvLen, iDealBufferLen);
			iStatus  = 0;
			break;
		}
		int iTotalLen = iLeftLen + iRecvLen;
		char* pszTmp = malloc(iTotalLen);
		if(iLeftLen > 0)
		{
			memcpy(pszTmp, pszDealBuffer, iLeftLen);
		}
		memcpy(pszTmp + iLeftLen, pszRecvBuffer, iRecvLen);
		memset(pszDealBuffer, 0, iDealBufferLen);
		memcpy(pszDealBuffer, pszTmp, iTotalLen);
		free(pszTmp);
		pszTmp = NULL;
		iLeftLen = iTotalLen;
		int iStartPos = 0;
		while (1)
		{
			if(NULL == pszStartBoundary)
			{
				//没有bound自己找
				char* pTitleEndPos = strstr(pszDealBuffer, "\r\n\r\n");
				if (NULL == pTitleEndPos)
				{
					break;
				}
				char* pszBoundary = strstr(pszDealBuffer, "boundary=");
				if (NULL == pszBoundary)
				{
					break;
				}
				pszBoundary += strlen("boundary=");
				char* pszBoundaryEnd = strstr(pszBoundary, "\r\n");
				if (NULL == pszBoundaryEnd)
				{
					break;
				}
				iBoundAryLen = pszBoundaryEnd - pszBoundary;
				char* pszboundaryIdentify = malloc(iBoundAryLen + 1);
				memset(pszboundaryIdentify, 0, iBoundAryLen + 1);
				memcpy(pszboundaryIdentify, pszBoundary, iBoundAryLen);

				pszStartBoundary = (char*)malloc(iBoundAryLen + 10);
				memset(pszStartBoundary, 0, iBoundAryLen + 10);
				strcpy(pszStartBoundary, "--");
				strcat(pszStartBoundary, pszboundaryIdentify);	
				strcat(pszStartBoundary, "\r\n");
				iBoundAryLen = strlen(pszStartBoundary);

				pszEndBoundary = (char*)malloc(iBoundAryLen + 10);
				memset(pszEndBoundary, 0, iBoundAryLen + 10);
				strcpy(pszEndBoundary, "\r\n");
				strcat(pszEndBoundary, "--");
				strcat(pszEndBoundary, pszboundaryIdentify);
				strcat(pszEndBoundary, "--\r\n");
				iBoundAryEndLen = strlen(pszEndBoundary);

				free(pszboundaryIdentify);
				pszboundaryIdentify = NULL;
				// printf("pszStartBoundary: %s\n", pszStartBoundary);
				// printf("pszEndBoundary: %s\n", pszEndBoundary);
				iStartPos = (pTitleEndPos - pszDealBuffer) + 4;
			}
			int iWriteLen = iTotalLen - iStartPos - iBoundAryEndLen;
			if(iWriteLen <= 0)
			{
				break;
			}
			char* pTmpBuffer = pszDealBuffer + iStartPos;
			// printf("\n==========start===========\n");
			// printf("%s", pTmpBuffer);
			// printf("\n==========end===========\n");
			char* pszPos = HttpFileServer_BinaryFind(pTmpBuffer, iTotalLen - iStartPos, pszStartBoundary, iBoundAryLen);
			if(NULL != pszPos)
			{
				//find start
				iWriteLen = (pszPos - pTmpBuffer) - 2;
				if(iWriteLen > 0 && NULL != pDestFile)
				{
					if(iWriteLen > 0)
					{
						int iRet = fwrite(pTmpBuffer, 1, iWriteLen, pDestFile);
						fflush(pDestFile);
						fclose(pDestFile);
						pDestFile = NULL;
						if (iRet != iWriteLen)
						{
							iStatus = 0;
						}
						else
						{
							iStartPos += iWriteLen + 2;
						}
						continue;
					}
				}
				else
				{
					char* pTitleEndPos = strstr(pTmpBuffer, "\r\n\r\n");
					if (NULL != pTitleEndPos)
					{
						char* pszFileName = strstr(pTmpBuffer, "filename=\"");
						if (NULL == pszFileName)
						{
							pTitleEndPos += 4;
							char* pszStartPos = HttpFileServer_BinaryFind(pTitleEndPos, iTotalLen - (pTitleEndPos - pszDealBuffer) - 1, pszStartBoundary, iBoundAryLen);
							if(NULL != pszStartPos)
							{
								iStartPos = pszStartPos - pszDealBuffer;
							}
							else
							{
								break;
							}
							continue;
						}
						pszFileName += strlen("filename=\"");
						char* pszFileNameEnd = strstr(pszFileName, "\"\r\n");
						if (NULL == pszFileNameEnd)
						{
							char* pszStartPos = HttpFileServer_BinaryFind(pTitleEndPos, iTotalLen - (pTitleEndPos - pszDealBuffer) - 1, pszStartBoundary, iBoundAryLen);
							if(NULL != pszStartPos)
							{
								iStartPos = pszStartPos - pszDealBuffer;
							}
							else
							{
								break;
							}
							continue;
						}
						memset(szFileName, 0, MAX_PATH);
						memcpy(szFileName, pszFileName, pszFileNameEnd - pszFileName);
						if (NULL != pDestFile)
						{
							fflush(pDestFile);
							fclose(pDestFile);
							pDestFile = NULL;
						}
						char szFile[MAX_PATH * 2] = { 0 };
						sprintf(szFile, "%s/%s", pszDestFold, szFileName);
						// printf("pfile:%s\n", szFile);
						pDestFile = fopen(szFile, "wb");
						if(NULL == pDestFile)
						{
							iStatus = 0;
							break;
						}

						// printf("szFileName:%s\n", szFileName);
						iStartPos += (pTitleEndPos - pTmpBuffer) + 4;
						continue;
					}
					else
					{
						break;
					}
				}
			}
			if (iWriteLen > 0)
			{
				if(NULL != pDestFile)
				{
					int iRet = fwrite(pTmpBuffer, 1, iWriteLen, pDestFile);
					if (iRet != iWriteLen)
					{
						iStatus = 0;
					}
				}
				iStartPos += iWriteLen;
			}
			if(iStatus >= 0)
			{
				break;
			}
		}
		if(NULL != pszEndBoundary)
		{
			char* pszFindEndPos = HttpFileServer_BinaryFind(pszDealBuffer + iStartPos, iTotalLen - iStartPos, pszEndBoundary, iBoundAryEndLen);
			if (NULL != pszFindEndPos)
			{
				//找到结束标识了
				printf("get end tag\n");
				iStatus = 1;
			}
		}
		
		if(iStartPos > 0)
		{
			iLeftLen = iTotalLen - iStartPos;
			char* pszTmpBuffer = (char*)malloc(iLeftLen);
			memset(pszTmpBuffer, 0, iLeftLen);
			memcpy(pszTmpBuffer, pszDealBuffer + iStartPos, iLeftLen);
			memset(pszDealBuffer, 0, iDealBufferLen);
			memcpy(pszDealBuffer, pszTmpBuffer, iLeftLen);
			free(pszTmpBuffer);
			pszTmpBuffer = NULL;
		}
	}
	if (NULL != pDestFile)
	{
		fflush(pDestFile);
		fclose(pDestFile);
		pDestFile = NULL;
	}
    if (NULL != pszDealBuffer)
	{
		free(pszDealBuffer);
		pszDealBuffer = NULL;
	}
	if (NULL != pszStartBoundary)
	{
		free(pszStartBoundary);
		pszStartBoundary = NULL;
	}
	if (NULL != pszEndBoundary)
	{
		free(pszEndBoundary);
		pszEndBoundary = NULL;
	}
	if (NULL != pszRecvBuffer)
	{
		free(pszRecvBuffer);
		pszRecvBuffer = NULL;
	}
	return iStatus;
}

DWORD HttpFileServer_ClientProc(LPVOID* lpParameter)
{
    // pthread_detach(pthread_self());
    struct HttpFileClient* pClient = (struct HttpFileClient*)lpParameter;
    SOCKET iClientSocket = (SOCKET)pClient->iClientSocket;
    free(lpParameter);
    lpParameter = NULL;
    if (iClientSocket == 0)
    {
        return 0;
    }
    BOOL bRet = FALSE;

    bRet = HttpFileServer_PraseFile(NULL, TMPFILEPATH, iClientSocket);
    HttpFileServer_SendResult(iClientSocket, bRet);
    
    closesocket(iClientSocket);
    iClientSocket = -1;
    return 1;
}

DWORD HttpFileServer_RecvProc(LPVOID* lpParameter)
{
    //pthread_detach(pthread_self());
    struct HttpFileServer* pSocketServer = (struct HttpFileServer*)lpParameter;
    struct sockaddr_in SocketConn;
    socklen_t iSocketLen = sizeof(SocketConn);
    if (pSocketServer->iServerSocket == 0)
    {
        printf("%s\n", "server error");
        return 0;
    }
    while (FALSE == pSocketServer->bQuit)
    {
        SOCKET iSocketConn = accept(pSocketServer->iServerSocket, (struct sockaddr*) & SocketConn, &iSocketLen);
        if (iSocketConn == SOCKET_ERROR)
        {
            printf("%s\n", "accept error ");
            continue;
        }

        int iTimeOut = 20*1000;
        setsockopt(iSocketConn, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
        setsockopt(iSocketConn, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeOut, sizeof(iTimeOut));
        int iBufferLen = sizeof(struct HttpFileClient);
        char* pBuffer = (char*)malloc(iBufferLen);
        memset(pBuffer, 0, iBufferLen);
        struct HttpFileClient* pClient = (struct HttpFileClient*)pBuffer;
        pClient->bConnect = TRUE;
        pClient->iClientSocket = iSocketConn;
        pClient->pServer = pSocketServer;
        HANDLE hClientHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HttpFileServer_ClientProc, pBuffer, 0, NULL);
		pthread_detach((pthread_t)hClientHandle);
    }
    
    if (0 != pSocketServer->iServerSocket)
    {
        shutdown(pSocketServer->iServerSocket, SD_BOTH);
        closesocket(pSocketServer->iServerSocket);
        pSocketServer->iServerSocket = 0;
    }
    
    return 1;
}

void HttpFileServer_Stop(struct HttpFileServer* pServer)
{

    if (NULL == pServer)
    {
        return;
    }
    pServer->bQuit = TRUE;
    shutdown(pServer->iServerSocket, SD_BOTH);
	closesocket(pServer->iServerSocket);
    if (NULL != pServer->hAccept)
    {
        WaitForSingleObject(pServer->hAccept, INFINITE);
        CloseHandle(pServer->hAccept);
        pServer->hAccept = NULL;
    }
    free((char*)pServer);
    pServer = NULL;
   
}

struct HttpFileServer* HttpFileServer_Start(int iPort)
{
    //printf("%s port:%d\n", "open socket", iPort);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(iPort);

    SOCKET iServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (iServerSocket <= 0)
    {
        printf("%s\n", "Create Socket Failed");
        return NULL;
    }
    int iValue = 1;
    setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iValue, sizeof(iValue));
    int iOpt = 1;
    setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iOpt, sizeof(iOpt));

    if (bind(iServerSocket, (const struct sockaddr*) & server_addr, (socklen_t)sizeof(server_addr)))
    {
        printf("Server Bind Port : Failed %d==>%d\n", iPort, errno);
        closesocket(iServerSocket);
        iServerSocket = 0;
        return NULL;
    }

    if (listen(iServerSocket, 10))
    {
        printf("Server Listen Failed!");
        closesocket(iServerSocket);
        iServerSocket = -1;
        return NULL;
    }
    int iBufferLen = sizeof(struct HttpFileServer);
    char* pBuffer = (char*)malloc(iBufferLen);
    memset(pBuffer, 0, iBufferLen);
    struct HttpFileServer* pHttpFileServer = (struct HttpFileServer*)pBuffer;
    pHttpFileServer->bQuit = FALSE;
    pHttpFileServer->iServerSocket = iServerSocket;
    pHttpFileServer->hAccept = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HttpFileServer_RecvProc, pHttpFileServer, 0, NULL);
    return pHttpFileServer;
}