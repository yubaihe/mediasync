#include "CommonUtil.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/timeb.h>  
#include <sys/stat.h>
CCommonUtil::CCommonUtil()
{

}

CCommonUtil::~CCommonUtil()
{

}

std::vector<std::string> CCommonUtil::StringSplit(const  std::string& s, const std::string& delim/* = ","*/)
{
	std::vector<std::string> elems;
	size_t pos = 0;
	size_t len = s.length();
	size_t delim_len = delim.length();
	if (delim_len == 0) return elems;
	while (pos < len)
	{
		int find_pos = s.find(delim, pos);
		if (find_pos < 0)
		{
			elems.push_back(s.substr(pos, len - pos));
			break;
		}
		elems.push_back(s.substr(pos, find_pos - pos));
		pos = find_pos + delim_len;
	}
	return elems;
}

std::string CCommonUtil::TimeToSec(std::string strTime)
{
	std::vector<std::string> timeVec = CCommonUtil::StringSplit(strTime, " ");
	if (timeVec.size() != 2)
	{
		return "";
	}
	for (uint32_t i = 0; i < timeVec.size(); ++i)
	{
		std::vector<std::string> timeVec2;
		if (i == 0)
		{
			timeVec2  = CCommonUtil::StringSplit(timeVec[i], "-");
		} 
		else
		{
			timeVec2 = CCommonUtil::StringSplit(timeVec[i], ":");
		}
		if (timeVec2.size() != 3)
		{
			return "";
		}
	}
	const char* pszTime = strTime.c_str();
	struct tm tt;
	memset(&tt, 0, sizeof(tt));
	tt.tm_year = atoi(pszTime) - 1900;
	tt.tm_mon = atoi(pszTime + 5) - 1;
	tt.tm_mday = atoi(pszTime + 8);
	tt.tm_hour = atoi(pszTime + 11);
	tt.tm_min = atoi(pszTime + 14);
	tt.tm_sec = atoi(pszTime + 17);
	std::string str = CCommonUtil::StringFormat("%lld", mktime(&tt));
	return str;
}

std::string CCommonUtil::StringFormat(const char* pszFmt, ...)
{
	std::string str;
	va_list args;
	va_start(args, pszFmt);
    int nLength = vsnprintf(NULL, 0, pszFmt, args);
	nLength += 1;  //上面返回的长度是包含\0，这里加上
    va_end(args);
    va_start(args, pszFmt);
    std::vector<char> vectorChars(nLength);
	vsnprintf(vectorChars.data(), nLength, pszFmt, args);

	str.assign(vectorChars.data());
	va_end(args);
	return str;
}

uint64_t CCommonUtil::CurTimeSec()
{
	//long long iTimeLast;
	//iTimeLast = time(NULL);     //总秒数  
	struct timeb t1;
	ftime(&t1);
	return t1.time;
}

std::string& CCommonUtil::ReplaceStr(std::string& str, const std::string& strFrom, const std::string& strTo)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += strTo.length())
	{
		pos = str.find(strFrom, pos);
		if (pos != std::string::npos)
		{
			str.replace(pos, strFrom.length(), strTo);
		}
		else
		{
			break;
		}
	}
	return str;
}

std::string CCommonUtil::SecToTime(long iSec)
{
	struct tm* ptr;
	time_t lt;
	lt = iSec;
	ptr = localtime(&lt);
	return CCommonUtil::StringFormat("%04d-%02d-%02d %02d:%02d:%02d", ptr->tm_year + 1900, ptr->tm_mon + 1, ptr->tm_mday, ptr->tm_hour, ptr->tm_min, ptr->tm_sec);
}

std::string CCommonUtil::ExecuteCmd(const char* pszCmd)
{
	char* pszRet = (char*)malloc(1);
	memset(pszRet, 0, 1);
    char szBuf[100] = {0};
    FILE *ptr;
    //printf("%s\n", pszCmd);
    if((ptr=popen(pszCmd, "r")) != NULL)
    {
        while(fgets(szBuf, 99, ptr) != NULL)
        {
            int iLen = strlen(pszRet) + strlen(szBuf) + 1;
            pszRet = (char*)realloc(pszRet, iLen);
			strcat(pszRet, szBuf);
            memset(szBuf, 0, 100);
        }
        pclose(ptr);
        ptr = NULL;
    }
    std::string strRet(pszRet);
	free(pszRet);
	pszRet = NULL;
	return strRet;
}

std::string CCommonUtil::TrimStr(std::string str)
{
	if (str.empty())
	{
		return str;
	}

	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

std::string CCommonUtil::GetMime(const char* pszFile)
{
    char szCmd[300] = {0};
    sprintf(szCmd, "file -i %s", pszFile);
    //printf("%s\n", szCmd);
    std::string strMime = CCommonUtil::ExecuteCmd(szCmd);
    strMime = strMime.substr(strlen(pszFile) + 1, strMime.length() - strlen(pszFile) - 1);
    strMime.erase(0, strMime.find_first_not_of(" "));
    strMime.erase(strMime.find_last_not_of(" ") + 1);
    std::vector<std::string> vec = CCommonUtil::StringSplit(strMime, ";");
    if(vec.size() == 1)
    {
        return "";
    }

    return vec[0];
}

BOOL CCommonUtil::IsMimeTypeImage(const char* pszMime)
{
	char szImage[] = "image";
	if(strlen(pszMime) > strlen(szImage) && 0 == memcmp(pszMime, szImage, strlen(szImage)))
    {
        return TRUE;
    }
	return FALSE;
}

BOOL CCommonUtil::IsMimeTypeVideo(const char* pszMime)
{
	char szVideo[] = "video";
	if(strlen(pszMime) > strlen(szVideo) && 0 == memcmp(pszMime, szVideo, strlen(szVideo)))
    {
        return TRUE;
    }
	return FALSE;
}



BOOL CCommonUtil::CheckFileExist(const char* pszFile)
{
    if((access(pszFile, F_OK)) != -1)   
    {   
        return TRUE;  
    }   
    else  
    {   
        return FALSE; 
    }   
}

BOOL CCommonUtil::CheckMagicFileExist(const char* pszPath)
{
    int iLen = strlen(pszPath);
    if(iLen == 0)
    {
        return FALSE;
    }
    char* psz = (char*)malloc(iLen);
    memset(psz, 0, iLen);
    int i = 0;
    int iIndex = 0;
    BOOL bExist = FALSE;
    while (pszPath[i] != '\0')
    {
        if(pszPath[i] == ':')
        {
            BOOL bRet = CCommonUtil::CheckFileExist(psz);
            if(TRUE == bRet)
            {
                bExist = TRUE;
                break;
            }
            iIndex = 0;
            memset(psz, 0, iLen);
        }
        else
        {
            psz[iIndex++] = pszPath[i];
        }
        
        i++;
    }
    if(FALSE == bExist && iIndex > 0)
    {
        bExist = CCommonUtil::CheckFileExist(psz);
    }
    free(psz);
    psz = NULL;
    return bExist;
    
}