#include "MultipartParser.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <syslog.h>
#include "fcgiapp.h"
#include "Dbus/libdbus.h"
#include "nlohmann/json.hpp"


CMultipartParser::CMultipartParser(const std::string& strBoundary, const std::string& strSaveDir) 
{
    m_strStartBoundary = "--" + strBoundary + "\r\n";
    m_strEndBoundary = "\r\n--" + strBoundary + "--\r\n";
    m_strSaveDir = strSaveDir;
    m_pOutFile = NULL;
    m_iTotalWritten = 0;
    m_iState = 0;
    m_bFinished = FALSE;
}

CMultipartParser::~CMultipartParser()
{
    if (NULL != m_pOutFile)
    {
        fclose(m_pOutFile);
        m_pOutFile = NULL;
    } 
}
BOOL CMultipartParser::FeedData(const char* pszData, size_t iDataLen)
{
    m_strBuffer.append(pszData, iDataLen);
    return ProcessBuffer();
}
BOOL CMultipartParser::IsFinished() const 
{ 
    return m_bFinished;
}
std::string CMultipartParser::ExtractFileName(const std::string& strHeaders) 
{
    static const std::string strToken = "filename=\"";
    size_t iPos = strHeaders.find(strToken);
    if (iPos == std::string::npos)
    {
        return "";
    }
    iPos += strToken.size();
    size_t iEnd = strHeaders.find('"', iPos);
    if (iEnd == std::string::npos)
    {
        return "";
    }
    return strHeaders.substr(iPos, iEnd - iPos);
}
std::string CMultipartParser::SanitizeFileName(const std::string& strName)
{
    size_t iPos = strName.find_last_of("/\\");
    if (iPos != std::string::npos)
    {
        return strName.substr(iPos + 1);
    }
    return strName;
}
BOOL CMultipartParser::ProcessBuffer()
{
    while (TRUE)
    {
        if (m_iState == 0)
        {
            // 查找第一个开始边界
            size_t iPos = m_strBuffer.find(m_strStartBoundary);
            if (iPos == std::string::npos)
            {
                break;
            }
            m_strBuffer.erase(0, iPos + m_strStartBoundary.size());
            m_iState = 1;
            continue;
        }
        if (m_iState == 1) 
        {
            // 查找头部结束标志 "\r\n\r\n"
            size_t iPos = m_strBuffer.find("\r\n\r\n");
            if (iPos == std::string::npos)
            {
                break;
            }
            std::string strHeaders = m_strBuffer.substr(0, iPos);
            m_strBuffer.erase(0, iPos + 4);
            std::string strFileName = ExtractFileName(strHeaders);
            if (strFileName.empty())
            {
                // 没有文件名的part，跳过整个part
            }
            else
            {
                std::string strSafeName = SanitizeFileName(strFileName);
                if (strSafeName.empty())
                {
                    syslog(LOG_WARNING, "Empty or invalid filename");
                } 
                else 
                {
                    char szPath[1024];
                    snprintf(szPath, sizeof(szPath), "%s/%s", m_strSaveDir.c_str(), strSafeName.c_str());
                    m_pOutFile = fopen(szPath, "wb");
                    if (!m_pOutFile) 
                    {
                        syslog(LOG_ERR, "Failed to open file %s", szPath);
                        return FALSE;
                    }
                }
            }
            m_iState = 2;
            continue;
        }
        if (m_iState == 2)
        {
            // 查找结束边界或下一个开始边界
            size_t iEndPos = m_strBuffer.find(m_strEndBoundary);
            size_t iNextStartPos = m_strBuffer.find(m_strStartBoundary);
            size_t iWriteEnd = std::min(iEndPos, iNextStartPos);
            if (iWriteEnd == std::string::npos)
            {
                // 未找到任何边界，全部作为内容写入
                if (NULL != m_pOutFile)
                {
                    size_t iWritten = fwrite(m_strBuffer.data(), 1, m_strBuffer.size(), m_pOutFile);
                    if (iWritten != m_strBuffer.size())
                    {
                        syslog(LOG_ERR, "Write error");
                        return FALSE;
                    }
                    m_iTotalWritten += iWritten;
                }
                m_strBuffer.clear();
                break;
            }
            // 写入直到 iWriteEnd
            if (m_pOutFile && iWriteEnd > 0)
            {
                size_t iWritten = fwrite(m_strBuffer.data(), 1, iWriteEnd, m_pOutFile);
                if (iWritten != iWriteEnd)
                {
                    syslog(LOG_ERR, "Write error");
                    return FALSE;
                }
                m_iTotalWritten += iWritten;
            }
            m_strBuffer.erase(0, iWriteEnd);
            // 检查是否是结束边界
            if (iEndPos == 0)
            {
                m_strBuffer.erase(0, m_strEndBoundary.size());
                m_bFinished = TRUE;
                if (NULL != m_pOutFile)
                {
                    fclose(m_pOutFile);
                    m_pOutFile = NULL;
                }
                break;
            }
            if (iNextStartPos == 0)
            {
                // 遇到了下一个part的开始，关闭当前文件，重置状态
                if (NULL != m_pOutFile)
                {
                    fclose(m_pOutFile);
                    m_pOutFile = NULL;
                }
                m_strBuffer.erase(0, m_strStartBoundary.size());
                m_iState = 1;
                continue;
            }
        }
    }
    return true;
}

    