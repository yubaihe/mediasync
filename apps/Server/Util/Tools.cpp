#include "Tools.h"
#include <sys/statvfs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "./CommonUtil.h"
#include "md5.h"
#include "Base64Coding.h"
#include "../GpsManager.h"
namespace Server
{
    long long CTools::CurTimeMilSec()
    {
        struct timeval tv;    
        gettimeofday(&tv,NULL);    
        return ((long long)tv.tv_sec) * 1000 + tv.tv_usec / 1000; 
    }
    int CTools::GetYear(long iTimeSec)
    {
        struct tm * pInfo;
        pInfo = localtime((const time_t *)&iTimeSec);
        return pInfo->tm_year + 1900;
    }
    BOOL CTools::GetDiskInfo(const char* pszAddr, size_t* iTotal, size_t* iUsed)
    {
        struct statvfs64 stat;
        if (statvfs64(pszAddr, &stat) != 0) 
        {
            return FALSE;
        }
        *iTotal = (size_t)((stat.f_frsize * stat.f_blocks)/1024.0);
        size_t iFree = (size_t)((stat.f_frsize * stat.f_bfree)/1024.0);

        *iUsed = *iTotal - iFree;
        return TRUE;
    }
    void CTools::SecInfo(long iSec, uint16_t* piYear, uint8_t* piMonth, uint8_t* piDay)
    {
        struct tm* ptr;
        time_t lt;
        lt = iSec;
        ptr = localtime(&lt);
        *piYear = ptr->tm_year + 1900;
        *piMonth = ptr->tm_mon + 1;
        *piDay = ptr->tm_mday;
    }
    BOOL CTools::CheckGps(string strGps, GPSTYPE* piGpsType, float* pdLong, float* pdLat)
    {
        string strLongitude;
        string strLatitude;
        CGpsManager gpsManager;
        BOOL bRet = gpsManager.ParseGps(strGps, piGpsType, &strLongitude, &strLatitude);
        *pdLong = atof(strLongitude.c_str());
        *pdLat = atof(strLatitude.c_str());
        return bRet;
    }
    time_t CTools::CurTimeSec()
    {
        struct timeval tv;    
        gettimeofday(&tv,NULL);    
        return tv.tv_sec ; 
    }
    string CTools::ReplaceString(string strOriginal, string strOld, string strNew)
    {
        string strRet = strOriginal;
        size_t iPos = 0;
        while ((iPos = strRet.find(strOld, iPos)) != std::string::npos) 
        {
            // 执行替换并更新查找起点
            strRet.replace(iPos, strOld.length(), strNew);
            iPos += strNew.length(); // 跳过已替换部分
        }
        return strRet;
    }
    int CTools::Rfind(const char* psz, char sz)
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
    string CTools::GetMacAddr()
    {
        int iSocketMac = socket(AF_INET, SOCK_STREAM, 0);
        if( iSocketMac == -1)  
        {
            printf("socket create error\n");
            return "";
        }

        struct ifconf ifc;
        char szBuf[512] = {0};
        ifc.ifc_len = 512;
        ifc.ifc_buf = szBuf;
        ioctl(iSocketMac, SIOCGIFCONF, &ifc);    
        struct ifreq* pIfr = (struct ifreq*)szBuf;
        int iNum = (ifc.ifc_len/sizeof(struct ifreq));
        for(int i=0; i < iNum; ++i)
        {
            struct ifreq ifr2;
            strcpy(ifr2.ifr_name, pIfr->ifr_name);
            if (ioctl(iSocketMac, SIOCGIFHWADDR, &ifr2) < 0)
            {
                printf("%s no mac\n", pIfr->ifr_name);
            }
            else
            {
                unsigned char szMac[6] = {0};
                memcpy(szMac, ifr2.ifr_hwaddr.sa_data, sizeof(szMac));
                char szFormatMac[18] = {0};
                sprintf(szFormatMac, "%02X:%02X:%02X:%02X:%02X:%02X",
                szMac[0], szMac[1], szMac[2], szMac[3], szMac[4], szMac[5]);
                if(0 != strcmp(szFormatMac, "00:00:00:00:00:00"))
                {
                    string strMac(szFormatMac);
                    return strMac;
                }
            }
            pIfr++;
        }
        return "";
    }
    string CTools::GetMd5(const char* psz)
    {
        md5_state_t md5StateT;
        md5_init(&md5StateT);
        md5_append(&md5StateT, (const md5_byte_t *)psz, (int)strlen(psz));
        md5_byte_t digest[16];
        md5_finish(&md5StateT, digest);
        char szMd5[33] = {0}, szHexBuffer[5] = {0};
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
        
        string strRet(szBase64);
        return strRet;
    }
    BOOL CTools::HasRootPermission()
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

    BOOL CTools::UpdateTimeSec(long iSec)
    {
        if(FALSE == CTools::HasRootPermission())
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
    string CTools::GetSambaVersion()
    {
        //Version 4.3.11-Ubuntu
        string strVersion = CCommonUtil::ExecuteCmd("samba -V");
        if(strVersion.length() == 0)
        {
            return strVersion;
        }
        vector<string> vec = CCommonUtil::StringSplit(strVersion, " ");
        if(vec.size() != 2)
        {
            return strVersion;
        }
        vector<string> vec2 = CCommonUtil::StringSplit(vec[1], "-");
        if(vec.size() != 2)
        {
            return vec[1];
        }
        return vec2[0];
    }
    string CTools::Trim(const string& str)
    {
        // 找到第一个非空格字符位置
        size_t iStartPos = str.find_first_not_of(" \t\n\r\v\f");
        if (iStartPos == std::string::npos)
        {
            return "";
        }

        // 找到最后一个非空格字符位置
        size_t iEndPos = str.find_last_not_of(" \t\n\r\v\f");
        return str.substr(iStartPos, iEndPos - iStartPos + 1);
    }
}
