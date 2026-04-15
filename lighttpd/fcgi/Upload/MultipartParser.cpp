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
    // 最长边界长度（开始边界与结束边界中的较大值）
    const size_t MAX_BOUNDARY_LEN = std::max(m_strStartBoundary.size(), m_strEndBoundary.size());

    while (TRUE)
    {
        if (m_iState == 0)
        {
            // 查找第一个开始边界（没有前置 CRLF）
            size_t iPos = m_strBuffer.find(m_strStartBoundary);
            if (iPos == std::string::npos)
                break;
            m_strBuffer.erase(0, iPos + m_strStartBoundary.size());
            m_iState = 1;
            continue;
        }

        if (m_iState == 1)
        {
            // 查找头部结束标志 "\r\n\r\n"
            size_t iPos = m_strBuffer.find("\r\n\r\n");
            if (iPos == std::string::npos)
                break;
            std::string strHeaders = m_strBuffer.substr(0, iPos);
            m_strBuffer.erase(0, iPos + 4);

            std::string strFileName = ExtractFileName(strHeaders);
            if (!strFileName.empty())
            {
                std::string strSafeName = SanitizeFileName(strFileName);
                if (!strSafeName.empty())
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
            // 查找结束边界（包含前置 CRLF）和下一个开始边界（可能包含前置 CRLF）
            size_t iEndPos = m_strBuffer.find(m_strEndBoundary);
            
            // 查找开始边界时，允许其前面有可选的 "\r\n"（即找到 "\r\n--boundary\r\n" 或 "--boundary\r\n"）
            size_t iNextStartPos = std::string::npos;
            size_t iTempPos = m_strBuffer.find(m_strStartBoundary);
            if (iTempPos != std::string::npos)
            {
                iNextStartPos = iTempPos;
            }
            else
            {
                // 尝试查找带前置 CRLF 的边界（用于 part 之间）
                std::string strStartWithCRLF = "\r\n" + m_strStartBoundary;
                iTempPos = m_strBuffer.find(strStartWithCRLF);
                if (iTempPos != std::string::npos)
                    iNextStartPos = iTempPos;
            }

            size_t iWriteEnd = std::min(iEndPos, iNextStartPos);

            if (iWriteEnd == std::string::npos)
            {
                // 未找到任何边界 → 只写入绝对不属于边界前缀的部分
                if (m_strBuffer.size() > MAX_BOUNDARY_LEN)
                {
                    size_t iSafeWriteLen = m_strBuffer.size() - MAX_BOUNDARY_LEN;
                    if (m_pOutFile)
                    {
                        size_t iWritten = fwrite(m_strBuffer.data(), 1, iSafeWriteLen, m_pOutFile);
                        if (iWritten != iSafeWriteLen)
                        {
                            syslog(LOG_ERR, "Write error: expected %zu, wrote %zu", iSafeWriteLen, iWritten);
                            return FALSE;
                        }
                        m_iTotalWritten += iWritten;
                    }
                    m_strBuffer.erase(0, iSafeWriteLen);
                }
                // 保留长度 <= MAX_BOUNDARY_LEN 的数据暂不写入，等待更多数据
                break;
            }

            // 写入边界前的确定内容
            if (m_pOutFile && iWriteEnd > 0)
            {
                size_t iWritten = fwrite(m_strBuffer.data(), 1, iWriteEnd, m_pOutFile);
                if (iWritten != iWriteEnd)
                {
                    syslog(LOG_ERR, "Write error: expected %zu, wrote %zu", iWriteEnd, iWritten);
                    return FALSE;
                }
                m_iTotalWritten += iWritten;
            }
            m_strBuffer.erase(0, iWriteEnd);

            // 处理结束边界
            if (iEndPos == 0)
            {
                m_strBuffer.erase(0, m_strEndBoundary.size());
                m_bFinished = TRUE;
                if (m_pOutFile)
                {
                    fclose(m_pOutFile);
                    m_pOutFile = NULL;
                }
                break;
            }

            // 处理下一个 part 的开始边界
            if (iNextStartPos == 0)
            {
                if (m_pOutFile)
                {
                    fclose(m_pOutFile);
                    m_pOutFile = NULL;
                }
                // 删除实际匹配到的边界字符串（可能包含前置 CRLF）
                std::string strMatchedBoundary;
                if (m_strBuffer.find(m_strStartBoundary) == 0)
                    strMatchedBoundary = m_strStartBoundary;
                else
                    strMatchedBoundary = "\r\n" + m_strStartBoundary;
                m_strBuffer.erase(0, strMatchedBoundary.size());
                m_iState = 1;
                continue;
            }
        }
    }
    return TRUE;
}
    