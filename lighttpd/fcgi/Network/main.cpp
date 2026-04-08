#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <alloca.h>
#include <fcgiapp.h>
#include <stdio.h>
#include "MessageHandler.h"

#define LISTENSOCK_FILENO 0
#define LISTENSOCK_FLAGS 0
char* GetParam(FCGX_Request* pCgi)
{
    //获取POST请求携带的参数长度
    char *pLenstr = FCGX_GetParam("CONTENT_LENGTH", pCgi->envp);
    int iDataLen = atoi(pLenstr);
    if(iDataLen == 0)
    {
        return NULL;
    }
    char* pRecvBuffer = (char*)malloc(iDataLen + 1);
    memset(pRecvBuffer, 0, iDataLen + 1);
    FCGX_GetStr(pRecvBuffer, iDataLen, pCgi->in);
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
    //日志会输出到/var/log/syslog
    openlog("MediaParse", LOG_CONS|LOG_NDELAY, LOG_USER);
    syslog (LOG_INFO, "MediaParse start"); 
	int iError = FCGX_Init(); /* call before Accept in multithreaded apps */
	if (iError) 
    { 
        syslog (LOG_INFO, "FCGX_Init failed: %d", iError); 
        closelog();
        return 1; 
    }
	FCGX_Request cgi;
	iError = FCGX_InitRequest(&cgi, LISTENSOCK_FILENO, LISTENSOCK_FLAGS);
	if (iError) 
    { 
        syslog(LOG_INFO, "FCGX_InitRequest failed: %d", iError); 
        closelog();
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
        //syslog (LOG_INFO, "MediaParse get request:%s", FCGX_GetParam("REQUEST_METHOD", cgi.envp)); 
		if(!strncmp("POST", FCGX_GetParam("REQUEST_METHOD", cgi.envp), strlen("POST")))
        {
            char* pParam = GetParam(&cgi);
            if(NULL == pParam)
            {
                syslog(LOG_INFO, "GetParam empty");
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
    closelog();
	return 1;
}
