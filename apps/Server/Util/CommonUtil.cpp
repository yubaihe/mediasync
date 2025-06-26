#include "CommonUtil.h"
#include <sys/stat.h>
#include <ifaddrs.h>
#include <set>
#include <net/if_arp.h>
#include <linux/wireless.h>
#include <linux/if_tun.h>
namespace Server
{
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
    string CCommonUtil::ExecuteCmd2(string strCmd)
    {
        char* pszRet = (char*)malloc(1);
        memset(pszRet, 0, 1);
        char szBuf[100] = {0};
        FILE *ptr;
        //printf("%s\n", pszCmd);
        if((ptr=popen(strCmd.c_str(), "r")) != NULL)
        {
            while(fgets(szBuf, 99, ptr) != NULL)
            {
                //printf("%s\n", szBuf);
                int iLen = strlen(pszRet) + strlen(szBuf) + 1;
                pszRet = (char*)realloc(pszRet, iLen);
                strcat(pszRet, szBuf);
                memset(szBuf, 0, 100);
            }
            pclose(ptr);
            ptr = NULL;
        }
        string strRet(pszRet);
        free(pszRet);
        pszRet = NULL;
        return strRet;
    }
    std::string CCommonUtil::ExecuteCmd(const char* pszFormat, ...)
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
        //printf("%s\n", pszCmd);
        va_end(vArgList);
        string strRet = Server::CCommonUtil::ExecuteCmd2(pszCmd);

        free(pszCmd);
        pszCmd = NULL;
        return strRet;
    }
    BOOL CCommonUtil::CheckStatus(pid_t status, const char* pszCommand)
    {
        if (-1 == status)
        {
            printf("system error!");
            return FALSE;
        }
        else
        {
            //printf("exit status value = [0x%x]\n", status);
            if (WIFEXITED(status))
            {
                if (0 == WEXITSTATUS(status))
                {
                    printf("run shell script [successfully] %s .\n", pszCommand);
                    return TRUE;
                }
                else
                {
                    printf("run shell script  [fail] %s, script exit code: %d\n", pszCommand, WEXITSTATUS(status));
                    return FALSE;
                }
            }
            else
            {
                printf("exit status = [%d]\n", WEXITSTATUS(status));
                return FALSE;
            }
        }
    }
    BOOL CCommonUtil::ExecuteCmdWithOutReplay(const char* pszFormat, ...)
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
       
        pid_t iStatus = system(pszCmd);
        BOOL bRet = CheckStatus(iStatus, pszCmd);
        free(pszCmd);
        pszCmd = NULL;
        return bRet;
    }
    string CCommonUtil::ListToString(list<string> List, string strSep)
    {
        if(List.size() == 0)
        {
            return "";
        }
        string strRet = "";
        for(list<string>::iterator itor = List.begin(); itor != List.end(); ++itor)
        {
            strRet = strRet.append(*itor);
            strRet = strRet.append(strSep);
        }
        if (strRet.size() >= strSep.length() && strRet.substr(strRet.size() - strSep.length()) == strSep)
        {
            strRet.resize(strRet.size() - strSep.length());
        }
        return strRet;
    }
    list<string> CCommonUtil::AllNetNames()
{
    std::list<std::string> retList;
    struct ifaddrs* pIfaddr;

    if(getifaddrs(&pIfaddr) == -1)
    {
        return retList;
    }
    std::set<std::string> namesSet; 
    for (struct ifaddrs* pItem = pIfaddr; pItem != NULL; pItem = pItem->ifa_next) 
    {
        if (NULL != pItem->ifa_name) 
        {
            namesSet.insert(pItem->ifa_name);
        }
    }
    for (string strName : namesSet) 
    {
        retList.push_back(strName);
    }
    freeifaddrs(pIfaddr);
    return retList;
}
BOOL CCommonUtil::ContainNetName(string strNetName)
{
    list<string> nameList = CCommonUtil::AllNetNames();
    for(list<string>::iterator itor = nameList.begin(); itor != nameList.end(); ++itor)
    {
        if(0 == itor->compare(strNetName))
        {
            return TRUE;
        }
    }
    return FALSE;
}
}


