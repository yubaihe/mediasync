#include "stdafx.h"
#include "Tools.h"
#include <stdio.h> 
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <sys/stat.h>
#include <time.h>
#include<mntent.h>

long long Tools_CurTimeMilSec()
{
	struct timeval tv;    
	gettimeofday(&tv,NULL);    
	return ((long long)tv.tv_sec) * 1000 + tv.tv_usec / 1000; 
}

int Tools_GetYear(long iTimeSec)
{
    struct tm * pInfo;
    pInfo = localtime((const time_t *)&iTimeSec);
    return pInfo->tm_year + 1900;
}

long Tools_CurTimeSec()
{
	struct timeval tv;    
	gettimeofday(&tv,NULL);    
	return tv.tv_sec ; 
}

char* Tools_GetMacAddr()
{
	int iSocketMac = socket(AF_INET, SOCK_STREAM, 0);
	if( iSocketMac == -1)  
	{
		printf("socket create error\n");
		char* pBuffer = (char*)malloc(1);
		memset(pBuffer, 0, 1);
		return pBuffer;
	}

	struct ifconf ifc;
	char szBuf[512] = {0};
	struct ifreq *ifr;

	//��ʼ��ifconf
	ifc.ifc_len = 512;
	ifc.ifc_buf = szBuf;
	ioctl(iSocketMac, SIOCGIFCONF, &ifc);    
	ifr = (struct ifreq*)szBuf;
	int iNum = (ifc.ifc_len/sizeof(struct ifreq));
	for(int i=0; i < iNum; ++i)
	{
		struct ifreq ifr2;
		strcpy(ifr2.ifr_name, ifr->ifr_name);
		if (ioctl(iSocketMac, SIOCGIFHWADDR, &ifr2) < 0)
		{
			printf("%s no mac\n", ifr->ifr_name);
		}
		else
		{
			unsigned char szMac[6] = {0};
			memcpy(szMac, ifr2.ifr_hwaddr.sa_data, sizeof(szMac));
			char* pszMac = (char*)malloc(18);
			memset(pszMac, 0, 18);
			sprintf(pszMac, "%02X:%02X:%02X:%02X:%02X:%02X",
	          szMac[0], szMac[1], szMac[2], szMac[3], szMac[4], szMac[5]);
			if(0 == strcmp(pszMac, "00:00:00:00:00:00"))
			{
				free(pszMac);
				pszMac = NULL;
			}
			else
			{
				return pszMac;
			}
		}
		ifr++;
	}
	char* pBuffer = (char*)malloc(1);
	memset(pBuffer, 0, 1);
	return pBuffer;
}

BOOL Tools_IsIpFormatRight(char* pIpAddress)
{
	int a, b, c, d;
	if ((sscanf(pIpAddress, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
		&& (a >= 0 && a <= 255)
		&& (b >= 0 && b <= 255)
		&& (c >= 0 && c <= 255)
		&& (d >= 0 && d <= 255))
	{
		return TRUE;
	}
	return FALSE;
}

char* Tools_CurrDir()
{
	char szDir[255] = {0};
	int iLen = readlink("/proc/self/exe", szDir, 255);
	char *psz = NULL;
    if (NULL != (psz = strrchr(szDir,'/')))
    {
        *psz = '\0';
    }
	char*pBuffer = (char*)malloc(iLen + 2);
	memset(pBuffer, 0, iLen + 2);
	strcpy(pBuffer, szDir);
	return pBuffer;
}

BOOL Tools_IsFile(const char* pszPath)
{
	struct stat _stat;
	_stat.st_mode = 0;
	stat(pszPath, &_stat);
	if ((_stat.st_mode&S_IFREG) == S_IFREG)
	{
		return TRUE;
	}
	return FALSE;
}

long Tools_GetFileLen(char* pPathFile)
{
	struct stat _stat;
	stat(pPathFile, &_stat);
	return _stat.st_size;
}


int Tools_Rfind(const char* psz, char sz)
{
	int iLen = strlen(psz);
	while(psz[iLen] != sz)
	{
		iLen--;
		if(iLen < 0)
		{
			break;
		}
	}

	return iLen;
}

int Tools_RandomValue(int iMaxValue)
{
	srand((unsigned int)time(0));
    return rand()%iMaxValue + 1;
}

int Tools_RandomRangeValue(int iMinValue, int iMaxValue)
{
	srand((unsigned int)time(0));
    return rand()%(iMaxValue - iMinValue) + iMinValue;
	//return 7567;
}

int Tools_YearStartSec(int iYear)
{
	time_t curTime = time(0);
	struct tm* pTimeNow;
	pTimeNow = localtime(&curTime);
	pTimeNow->tm_year = iYear - 1900;
	pTimeNow->tm_mon = 0;
	pTimeNow->tm_mday = 1;
	pTimeNow->tm_hour = 0;
	pTimeNow->tm_min = 0;
	pTimeNow->tm_sec = 0;
	time_t iNow = mktime(pTimeNow);
	return iNow;
}

int Tools_YearEndSec(int iYear)
{
	time_t curTime = time(0);
	struct tm* pTimeNow;
	pTimeNow = localtime(&curTime);
	pTimeNow->tm_year = iYear - 1900;
	pTimeNow->tm_mon = 11;
	pTimeNow->tm_mday = 31;
	pTimeNow->tm_hour = 23;
	pTimeNow->tm_min = 59;
	pTimeNow->tm_sec = 59;
	time_t iNow = mktime(pTimeNow);
	return iNow;
}

char* Tools_ReplaceString(const char* pSrc, char* pSep, char* pNewSep)
{
	if(NULL == pSrc || strlen(pSrc) == 0)
	{
		char* pBuffer = malloc(1);
		memset(pBuffer, 0, 1);
		return pBuffer;
	}
    int iDestLen = strlen(pSrc) * strlen(pNewSep) + 1;
    char* pszDest = malloc(iDestLen);
    memset(pszDest, 0, iDestLen);
    for(int i = 0; pSrc[i]; ++i) 
    {
        if(pSrc[i] == pSep[0]) 
        {
            const char* psz = pSrc + i;
            char* qsz = pSep;
            BOOL bFind = TRUE;
            for (int j = 0; j < strlen(pSep); ++j)
            {
                if (psz[j] != qsz[j])
                {
                    bFind = FALSE;
                    break;
                }
                
            }
            if(bFind)
            {
                strcat(pszDest, pNewSep);
                i += strlen(pSep) - 1;
            }
        }
        else
        {
            char sz[2] = {0};
            sz[0] = pSrc[i];
            strcat(pszDest, sz);
        }
    }
    if(NULL == pszDest)
    {
        pszDest = malloc(1);
		memset(pszDest, 0, 1);
    }
    return pszDest;
}

void* __rawmemchr (const void *s, int c)
{
  if (c != '\0')
    return memchr (s, c, (size_t)-1);
  return (char *)s + strlen (s);
}

// char * strtok_r2 (char *s, const char *delim, char **save_ptr)
// {
//   char *token;
//   if (s == NULL)
//     s = *save_ptr;
//   /* Scan leading delimiters. */
//   s += strspn (s, delim);
//   if (*s == '\0')
//     {
//       *save_ptr = s;
//       return NULL;
//     }
//   /* Find the end of the token. */
//   token = s;
//   s = strpbrk (token, delim);
//   if (s == NULL)
//     /* This token finishes the string. */
//     *save_ptr = __rawmemchr (token, '\0');
//   else
//     {
//       /* Terminate the token and make *SAVE_PTR point past it. */
//       *s = '\0';
//       *save_ptr = s + 1;
//     }
//   return token;
// }
// char* Tools_SortYear(char* pYears)
// {
// 	int iLen = strlen(pYears) + 1;
//     char* pBuffer = (char*)malloc(iLen);
// 	memset(pBuffer, 0, iLen);
// 	memcpy(pBuffer, pYears, iLen - 1);
//     int iCount = 0;
//     int* pYearBuffer = NULL;
//     char* pTmp = pBuffer;
// 	char* pOut = NULL;
//     while((pTmp = strtok_r(pTmp, "&", &pOut)) != NULL)
//     {
//         if(iCount == 0)
//         {
//             pYearBuffer = malloc(sizeof(int));
//             pYearBuffer[0] = atoi(pTmp);
//         }
// 		else
// 		{
// 			pYearBuffer = realloc(pYearBuffer, (iCount + 1)*sizeof(int));
//         	pYearBuffer[iCount] = atoi(pTmp);
// 		}
		
//         iCount++;

// 		pTmp = NULL;
//     }
// 	if(pBuffer != NULL)
// 	{
// 		free(pBuffer);
// 		pBuffer = NULL;
// 	}
	
//     if(iCount == 0)
//     {
//         char* pBuffer = malloc(1);
//         memset(pBuffer, 0, 1);
//         return pBuffer;
//     }
    
//    char* pRetBuffer = NULL;
//     while(1)
//     {
//         int iMax = 0;
//         int iMaxIndex = 0;
//         for(int i = 0; i < iCount; ++i)
//         {
//             if(iMax < pYearBuffer[i])
//             {
//                 iMax = pYearBuffer[i];
//                 iMaxIndex = i;
//             }
//         }
        
//         if(iMax == 0)
//         {
//             break;
//         }
//         pYearBuffer[iMaxIndex] = 0;
//         char sz[20] = {0};
//         sprintf(sz, "%d", iMax);
//         int iSzLen = strlen(sz);
//         if(pRetBuffer == NULL)
//         {
//             pRetBuffer = (char*)malloc(iSzLen + 1);
//             memset(pRetBuffer, 0, iSzLen + 1);
//             strcpy(pRetBuffer, sz);
//         }
//         else
//         {
//             pRetBuffer = realloc(pRetBuffer, strlen(pRetBuffer) + iSzLen + 2);
//             strcat(pRetBuffer, "&");
//             strcat(pRetBuffer, sz);
//         }
//     }
    
//     free(pYearBuffer);
//     pYearBuffer = NULL;
//     if(NULL == pRetBuffer)
//     {
//         char* pBuffer = malloc(1);
//         memset(pBuffer, 0, 1);
//         return pBuffer;
//     }
//     return pRetBuffer;
// }

char* Tools_ExecuteCmd(const char* pszCmd)
{
	char* pszRet = malloc(1);
	memset(pszRet, 0, 1);
    char szBuf[100] = {0};
    FILE *ptr;
    //printf("%s\n", pszCmd);
    if((ptr=popen(pszCmd, "r")) != NULL)
    {
        while(fgets(szBuf, 99, ptr) != NULL)
        {
            int iLen = strlen(pszRet) + strlen(szBuf) + 1;
            pszRet = realloc(pszRet, iLen);
			strcat(pszRet, szBuf);
            memset(szBuf, 0, 100);
        }
        pclose(ptr);
        ptr = NULL;
    }
    //printf("%s\n", pszRet);
	return pszRet;
}
void Tools_ExecuteCmd2(const char* pszCmd)
{
    char szBuf[100] = {0};
    FILE *ptr;
    if((ptr=popen(pszCmd, "r")) != NULL)
    {
        while(fgets(szBuf, 99, ptr) != NULL)
        {
            //printf("%s\n", szBuf);
            memset(szBuf, 0, 100);
        }
        pclose(ptr);
        ptr = NULL;
    }
}

char* Tools_ExecuteCmdWithFormat(const char* pszFormat, ...)
{
    char* pszCmd = NULL;
    int iCmdLen = 0;
    va_list vArgList;
    va_start (vArgList, pszFormat);
    iCmdLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
    va_end(vArgList);

    va_start (vArgList, pszFormat); 
    pszCmd = (char*)malloc(iCmdLen);
    memset(pszCmd, 0, iCmdLen);
    vsnprintf(pszCmd, iCmdLen, pszFormat, vArgList);
    va_end(vArgList);
    char* pszRet = Tools_ExecuteCmd(pszCmd);
    free(pszCmd);
    pszCmd = NULL;
    return pszRet;
}


void Tools_ExecuteCmdWithFormatNoReplay(const char* pszFormat, ...)
{
    char* pszCmd = NULL;
    int iCmdLen = 0;
    va_list vArgList;
    va_start (vArgList, pszFormat);
    iCmdLen = vsnprintf(NULL, 0, pszFormat, vArgList) + 1;
    va_end(vArgList);

    va_start (vArgList, pszFormat); 
    pszCmd = (char*)malloc(iCmdLen);
    memset(pszCmd, 0, iCmdLen);
    vsnprintf(pszCmd, iCmdLen, pszFormat, vArgList);
    va_end(vArgList);
    
    system(pszCmd);

    free(pszCmd);
    pszCmd = NULL;
}

BOOL Tools_GetDiskInfo(long* iTotal, long* iUsed)
{
    char* pMntPoint = getenv("SYNC_MOUNT");
    if(NULL == pMntPoint)
    {
        return FALSE;
    }
    char szBuffer[1024] = {0};
    sprintf(szBuffer, "df %s", pMntPoint);
    char* pszRet = Tools_ExecuteCmd(szBuffer);
    
    if(NULL == pszRet)
    {
        return FALSE;
    }
    char* pTmp = pszRet;
	char* pOut = NULL;
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        if(strlen(pTmp) < strlen(pMntPoint))
        {
            continue;
        }
        if(0 == memcmp(pTmp, pMntPoint, strlen(pMntPoint)))
        {
            char szTotal[50] = {0};
            char szUsed[50] = {0};
            int iCount = 0;
            int i = 0;
            BOOL bFindSpace = FALSE;
            while(pTmp[i] != '\0')
            {
                if(pTmp[i] == ' ')
                {
                    if(FALSE == bFindSpace)
                    {
                        bFindSpace = TRUE;
                        iCount++;
                    }
                }
                else
                {
                    bFindSpace = FALSE;
                    char sz[2] = {0};
                    sz[0] = pTmp[i];
                    if(iCount == 1)
                    {
                        strcat(szTotal, sz);
                    }
                    else if(iCount == 2)
                    {
                        strcat(szUsed, sz);
                    }
                }
                
                i++;
            }
            if(strlen(szTotal) == 0)
            {
                free(pszRet);
                pszRet = NULL;
                return FALSE;
            }
            *iTotal = atol(szTotal);
            if(0 == strlen(szUsed))
            {
                *iUsed = 0;
            }
            else
            {
                *iUsed = atol(szUsed);
            }
        }
		pTmp = NULL;
    }
    free(pszRet);
    pszRet = NULL;
    return TRUE;
}

char* Tools_GetMd5(const char* psz)
{
    md5_state_t md5StateT;
    md5_init(&md5StateT);
    md5_append(&md5StateT, (const md5_byte_t *)psz, (int)strlen(psz));
    md5_byte_t digest[16];
    md5_finish(&md5StateT, digest);
    char szMd5[33] = {0}, szHexBuffer[3] = {0};
    for (size_t i = 0; i != 16; ++i)
    {
        if (digest[i] < 16)
            sprintf(szHexBuffer, "0%x", digest[i]);
        else
            sprintf(szHexBuffer, "%x", digest[i]);
        strcat(szMd5, szHexBuffer);
    }
    //printf("%s\n", szMd5String);
    char szBase64[1024] = {0};
    int iBufferLen = 1024;
    base64_encode((unsigned char*)szMd5, 32, (unsigned char*)szBase64, &iBufferLen);
    
    int iMd5Len = strlen(szBase64) + 1;
    char* pszRet = malloc(iMd5Len);
    memset(pszRet, 0, iMd5Len);
    strcpy(pszRet, szBase64);
    return pszRet;
}

char* Tools_SecToTime(long iSec)
{
	struct tm* ptr;
	time_t lt;
	lt = iSec;
	ptr = localtime(&lt);
    char* pRetBuffer = (char*)malloc(100);
    memset(pRetBuffer, 0, 100);
	sprintf(pRetBuffer, "%04d-%02d-%02d %02d:%02d:%02d", ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday, ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
    return pRetBuffer;
}

void Tools_SecInfo(long iSec, int* piYear, int* piMonth, int* piDay)
{
    struct tm* ptr;
	time_t lt;
	lt = iSec;
	ptr = localtime(&lt);
    *piYear = ptr->tm_year + 1900;
    *piMonth = ptr->tm_mon + 1;
    *piDay = ptr->tm_mday;
}

BOOL Tools_UpdateTimeSec(long iSec)
{
    if(FALSE == Tools_HasRootPermission())
    {
        return FALSE;
    }
    struct timeval stime;
    gettimeofday(&stime,NULL);
    stime.tv_sec = iSec;
    if(0 == settimeofday(&stime,NULL))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL Tools_GetMountInfo(const char* pszDevName, char* pszMountAddr, char* pszOutDevName, char* pszOutMountPoint, char* pszFsType, int* piReadOnly)
{
	FILE* pFile = NULL;
	struct mntent* pMntent = NULL;
	
	if((pszDevName == NULL)&&(pszMountAddr == NULL))
	{
		return FALSE;
	}
	pFile = setmntent("/proc/mounts", "r");
	if(pFile == NULL)
    {
        return FALSE;
    }
	while((pMntent = getmntent(pFile)) != NULL)
	{
		// 找到一个匹配的设备名或挂载点就退出
		if ((pszDevName != NULL && strcmp(pszDevName, pMntent->mnt_fsname) == 0) 
			||(pszMountAddr != NULL && strcmp(pszMountAddr, pMntent->mnt_dir) == 0))
		{
			break;
		}
	}	
	endmntent(pFile);
	
	if(pMntent == NULL)
    {
        return FALSE;
    }
	if(NULL != pszOutDevName)
    {
        strcpy(pszOutDevName, pMntent->mnt_fsname);
    }
	if(NULL != pszOutMountPoint)
    {
        strcpy(pszOutMountPoint, pMntent->mnt_dir);
    }
    if(NULL != pszFsType)
    {
        strcpy(pszFsType, pMntent->mnt_type);
    }
	if(NULL != piReadOnly)
	{
		if(strstr(pMntent->mnt_opts, "rw"))
        {
            *piReadOnly = 0;
        }
        else
        {
            *piReadOnly = 1;
        }
	}
	
	return TRUE;
}

BOOL Tools_CheckGps(const char* pszGps, GPSTYPE* piGpsType, float* pdLong, float* pdLat)
{
    printf("Tools_CheckGps:%s\n", pszGps);
    if(strlen(pszGps) < 3)
    {
        return FALSE;
    }
    *piGpsType = GPSTYPE_NORMAL;
    const char* pszGpsStart = pszGps;
    if(0 == memcmp(pszGpsStart, "B-", 2))
    {
        pszGpsStart += 2;
        *piGpsType = GPSTYPE_BAIDU;
    }
    int i = 0;
    BOOL bFind = FALSE;
    while (pszGpsStart[i] != '\0')
    {
        if(pszGpsStart[i] == '&')
        {
            bFind = TRUE;
            break;
        }
        i++;
    }
    if(FALSE == bFind)
    {
        *piGpsType = GPSTYPE_UNKNOW;
        return FALSE;
    }
    int iDestLen = i + 1;
    if((pszGpsStart + iDestLen)[0] == '\0')
    {
        //&后面如果没有东西了就是错的
        *piGpsType = GPSTYPE_UNKNOW;
        return FALSE;
    }
    char* psz = malloc(iDestLen);
    memset(psz, 0, iDestLen);
    memcpy(psz, pszGpsStart, iDestLen - 1);
    *pdLong = atof(psz);

    iDestLen = strlen(pszGpsStart) - i + 1;
    psz = realloc(psz, iDestLen);
    memset(psz, 0, iDestLen);
    memcpy(psz, pszGpsStart + i + 1, iDestLen - 1);
    *pdLat = atof(psz);
    free(psz);
    psz = NULL;
    printf("Tools_CheckGps:%f&%f\n", *pdLong, *pdLat);
    return TRUE;
}

BOOL Tools_HasRootPermission()
{
    uid_t iUid = geteuid();
    if (iUid != 0) 
    {
        return FALSE;
   	} 
    else
    {
   		return TRUE;
  	}
}

BOOL Tools_GetMountPointFromDevName(const char* pszDevName, char* pszOutMountPoint)
{
#ifdef ARM
    BOOL bRet = Tools_GetMountInfo(pszDevName, NULL, NULL, pszOutMountPoint, NULL, NULL);
    return bRet;
#else
    char* pszCurDir = Tools_CurrDir();
    sprintf(pszOutMountPoint, "%s/storage", pszCurDir);
    printf("~~~~%s\n", pszOutMountPoint);
     if(access(pszOutMountPoint, F_OK) < 0)
     {
         Tools_ExecuteCmdWithFormatNoReplay("mkdir %s -p", pszOutMountPoint);
     }
    free(pszCurDir);
    pszCurDir = NULL;
    return TRUE;
#endif
}
