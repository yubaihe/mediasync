#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <alloca.h>
#include <fcgiapp.h>
#include <stdio.h>
#include "stdafx.h"
#include "Dbus/libdbus.h"
#include "MessageHandler.h"
#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0
char* GetParam(FCGX_Request cgi)
{
    //获取POST请求携带的参数长度
    char *pLenstr = FCGX_GetParam("CONTENT_LENGTH", cgi.envp);
    int iDataLen = atoi(pLenstr);
    if(iDataLen == 0)
    {
        return NULL;
    }
    char* pRecvBuffer = (char*)malloc(iDataLen + 1);
    memset(pRecvBuffer, 0, iDataLen + 1);
    FCGX_GetStr(pRecvBuffer, iDataLen, cgi.in);
    return pRecvBuffer;
}

void ResponseJson(const char* pBuffer, FCGX_Request* pCgi)
{
    int iSendLen = strlen(pBuffer) + 100;
    char* pRetBuffer = (char*) malloc(iSendLen);
    memset(pRetBuffer, 0, iSendLen);
    strcpy(pRetBuffer, "Status: 200 OK\r\nContent-Type: text/html\r\n\r\n");
    strcat(pRetBuffer, pBuffer);
    strcat(pRetBuffer, "\r\n");
    FCGX_PutStr(pRetBuffer, strlen(pRetBuffer), pCgi->out);
    free(pRetBuffer);
    pRetBuffer = NULL;
}
int main(int argc, char** argv) 
{
    openlog("Server", LOG_CONS|LOG_NDELAY, LOG_USER);
    syslog (LOG_INFO, "Server start"); 
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
    CMessageHandler messageHandler;
    char szRetBuffer[MAX_MESSAGE_LEN] = {0};
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

            memset(szRetBuffer, 0, sizeof(szRetBuffer));
            messageHandler.OnMessage(pParam, szRetBuffer);
            ResponseJson(szRetBuffer, &cgi);
            
            free(pParam);
            pParam = NULL;
        }
        else
        {
            messageHandler.OnMessage("", szRetBuffer);
            ResponseJson(szRetBuffer, &cgi);
        }
	}
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