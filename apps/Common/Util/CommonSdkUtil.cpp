#include "CommonSdkUtil.h"
#include <stdint.h>
#include <pthread.h>
#include <cstring>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
void CCommonSdkUtil::Log(LOGLEVEL eLogLevel, const char* pszFun, const char* pszFile, int iLine, const char* pszFormat, ...)
{
	//printf("CCommonUtil::Log\n");
	char* psz = NULL;
	int iLen = 0;
	va_list vArgList;
	va_start (vArgList, pszFormat);
	iLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
	va_end(vArgList);

	va_start (vArgList, pszFormat); 
	psz = (char*)malloc(iLen);
	memset(psz, 0, iLen);
	vsnprintf(psz, iLen, pszFormat, vArgList);
	va_end(vArgList);
	CommonSdk_WriteLog(eLogLevel, pszFun, pszFile, iLine, psz);
	free(psz);
	psz = NULL;
}
CCommonSdkUtil::CCommonSdkUtil(/* args */)
{
}

CCommonSdkUtil::~CCommonSdkUtil()
{
}