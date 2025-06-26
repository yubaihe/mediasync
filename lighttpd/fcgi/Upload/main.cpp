#include "stdafx.h"
#include <fcgiapp.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <alloca.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <json.hpp>
#include "Dbus/libdbus.h"
#include "Common.h"
#define MAX_PATH 1024
#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0
void CommonSdk_WriteLog(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszLog)
{
    printf("%s\n", pszLog);
}
std::string g_strRootDir("");
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

BOOL PraseFile(char* pszBound, const char* pszDestFold, FCGX_Stream* pStream)
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
		char* pszboundaryIdentify = (char*)malloc(iBoundAryLen + 1);
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
	::FILE* pDestFile = NULL;
	int iStatus = -1;
	int iLeftLen = 0;
	
	while (1)
	{
		if (iStatus >= 0)
		{
			break;
		}
		memset(pszRecvBuffer, 0, iMinPackageLen);
		// int iRecvLen = recv((int)iSocketFd, pszRecvBuffer, iMinPackageLen, 0);
        int iRecvLen = FCGX_GetStr(pszRecvBuffer, iMinPackageLen, pStream);
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
		char* pszTmp = (char*)malloc(iTotalLen);
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
				char* pszboundaryIdentify = (char*)malloc(iBoundAryLen + 1);
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

FCGX_Request cgi;
void ResponseJson(const char* pBuffer)
{
	int iSendLen = strlen(pBuffer) + 100;
	char* pRetBuffer = (char*) malloc(iSendLen);
	memset(pRetBuffer, 0, iSendLen);
	strcpy(pRetBuffer, "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n");
	strcat(pRetBuffer, pBuffer);
	strcat(pRetBuffer, "\r\n");
	FCGX_PutStr(pRetBuffer, strlen(pRetBuffer), cgi.out);
	free(pRetBuffer);
	pRetBuffer = NULL;
}
std::string GetStoreAddr()
{
    char szRetBuffer[65535] = {0};
    int iDbusLen = 65535;
    BOOL bRet = LibDbus_SendSync("Server", 0x5007, (const char*)"{\"action\":\"store\"}", strlen("{\"action\":\"store\"}") + 1, szRetBuffer, &iDbusLen);
    if(FALSE == bRet||0 == iDbusLen)
    {
        return "";
    }
	try
	{
		nlohmann::json json = nlohmann::json::parse(szRetBuffer);
		string strToAddr = json["tempaddr"];
		if(strToAddr[strToAddr.length() - 1] != '/')
		{
			strToAddr.append("/");
		}
		printf("%s\n", strToAddr.c_str());
		syslog (LOG_INFO, "Upload start"); 
		return strToAddr;
	}
	catch (...)
	{
		return "";
	}
}
int main(int argc, char** argv) 
{
	//日志会输出到/var/log/syslog
    openlog("Upload", LOG_CONS|LOG_NDELAY, LOG_USER);
    syslog (LOG_INFO, "Upload start"); 
	int iError = FCGX_Init(); 
	if (iError) 
    { 
        return 1; 
    }
	
	iError = FCGX_InitRequest(&cgi, LISTENSOCK_FILENO, LISTENSOCK_FLAGS);
	if (iError) 
    { 
        return 2; 
    }
	while (1) 
    {
	    iError = FCGX_Accept_r(&cgi);
		if (iError) 
        { 
            syslog(LOG_INFO, "FCGX_Accept_r stopped: %d", iError); 
            break; 
        }
		char szBuffer[100] = {0};
        char* pszContentLen = FCGX_GetParam("CONTENT_LENGTH", cgi.envp);
        char* pszContentType = FCGX_GetParam("CONTENT_TYPE", cgi.envp);
		if(!strncmp("POST", FCGX_GetParam("REQUEST_METHOD", cgi.envp), strlen("POST")))
        {
	        char* psz = strstr(pszContentType, "boundary=");
	        psz += strlen("boundary=");
	        char szBoundary[100] = { 0 };
			
	        memcpy(szBoundary, psz, strlen(psz));
			if(g_strRootDir.length() == 0)
			{
				g_strRootDir = GetStoreAddr();
				if(g_strRootDir.length() == 0)
				{
					ResponseJson("failed");
					break;
				}
				if(g_strRootDir[g_strRootDir.length() - 1] != '/')
				{
					g_strRootDir.append("/");
				}
			}
            BOOL bRet = PraseFile(szBoundary, g_strRootDir.c_str(), cgi.in);
            if(bRet)
            {
                ResponseJson("success");
            }
            else
            {
                ResponseJson("failed");
            }
		}
	}
	return 1;
}
