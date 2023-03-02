#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <alloca.h>
#include <fcgiapp.h>
#include <stdio.h>
#include "stdafx.h"
#include "MessageHandler.h"

int SERVERPORT = 0;
char SERVERIP[16] = "";

#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0

void InitParam()
{
    int iFileContentLen = 1;
    char* pFileContent = malloc(1);
    memset(pFileContent, 0, iFileContentLen);

    char szBuffer[100] = {0};
    int iBufferLen = sizeof(szBuffer);
    FILE* pFile = fopen("./Config.json", "rb");
    if(NULL == pFile)
    {
        printf("%s\n", "No Config.json find");
        exit(0);
    }

    while(1)
    {
        int iReadLen = fread(szBuffer, sizeof(char), iBufferLen, pFile);
        char* pTmpBuffer = pFileContent;
        pFileContent = realloc(pTmpBuffer, iFileContentLen + iReadLen);
        memset(pFileContent + iFileContentLen, 0, iReadLen);
        memcpy(pFileContent + iFileContentLen - 1, szBuffer, iReadLen);
        iFileContentLen += iReadLen;
        if(iReadLen != iBufferLen)
        {
            break;
        }
    }

    fclose(pFile);

    printf("%s", pFileContent);
    json_object* pJsonRoot = json_tokener_parse(pFileContent);
    json_object* pServerObj = json_object_object_get(pJsonRoot, "server");
    char* pszIp = (char*)json_object_get_string(json_object_object_get(pServerObj, "ip"));
    int iPort = json_object_get_int(json_object_object_get(pServerObj, "port"));

    SERVERPORT = iPort;
    strcpy(SERVERIP, pszIp);

    json_object_put(pJsonRoot);
    free(pFileContent);
    pFileContent = NULL;
}

int main(int argc, char** argv) 
{
    //openlog("testfastcgi", LOG_CONS|LOG_NDELAY, LOG_USER);
	int iError = FCGX_Init(); /* call before Accept in multithreaded apps */
	if (iError) 
    { 
        //syslog (LOG_INFO, "FCGX_Init failed: %d", iError); 
        return 1; 
    }
	FCGX_Request cgi;
	iError = FCGX_InitRequest(&cgi, LISTENSOCK_FILENO, LISTENSOCK_FLAGS);
	if (iError) 
    { 
        //syslog(LOG_INFO, "FCGX_InitRequest failed: %d", iError); 
        return 2; 
    }

    char* GetParam(FCGX_Request cgi)
    {
        //获取POST请求携带的参数长度
        char *pLenstr = FCGX_GetParam("CONTENT_LENGTH", cgi.envp);
        int iDataLen = atoi(pLenstr);
        if(iDataLen == 0)
        {
            return NULL;
        }
        char* pRecvBuffer = malloc(iDataLen + 1);
        memset(pRecvBuffer, 0, iDataLen + 1);
        FCGX_GetStr(pRecvBuffer, iDataLen, cgi.in);
        return pRecvBuffer;
    }

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

    InitParam();
    MessageHandler* pMessageHandler = MessageHandler_Init();
	while (1) 
    {
	    iError = FCGX_Accept_r(&cgi);
		if (iError) 
        { 
            syslog(LOG_INFO, "FCGX_Accept_r stopped: %d", iError); 
            break; 
        }
		if(!strncmp("POST", FCGX_GetParam("REQUEST_METHOD", cgi.envp), strlen("POST")))
        {
            char* pParam = GetParam(cgi);
            if(NULL == pParam)
            {
                continue;
            }

            char* pRetBuffer = NULL;
            OnMessage(pMessageHandler->pHanlerMap, pParam, &pRetBuffer);
            
            ResponseJson(pRetBuffer);
            
            free(pParam);
            pParam = NULL;
            if(NULL != pRetBuffer)
            {
                free(pRetBuffer);
                pRetBuffer = NULL;
            }
        }
	}
    MessageHandler_UnInit(pMessageHandler);
	return 1;
}

// int main() 
// {
//     MessageHandler* pMessageHandler = MessageHandler_Init();
//     char* pRetBuffer = NULL;
     
//     OnMessage(pMessageHandler->pHanlerMap, "{\"action\":\"devtesdevtestdevtestt\"}", &pRetBuffer);
//     MessageHandler_UnInit(pMessageHandler);
//     free(pRetBuffer);
//     pRetBuffer = NULL;
//     return 1;
// }