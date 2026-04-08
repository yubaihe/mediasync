#ifndef MULTIPARTPARSER_H_
#define MULTIPARTPARSER_H_
#include "stdafx.h"
#include <string>
class CMultipartParser
{
public:
    CMultipartParser(const std::string& strBoundary, const std::string& strSaveDir);
    ~CMultipartParser();
    BOOL FeedData(const char* pszData, size_t iDataLen);
    BOOL IsFinished() const;
private:
    std::string ExtractFileName(const std::string& strHeaders);
    std::string SanitizeFileName(const std::string& strName);
    BOOL ProcessBuffer();
private:
    std::string m_strBuffer;
    std::string m_strStartBoundary;
    std::string m_strEndBoundary;
    std::string m_strSaveDir;
    FILE* m_pOutFile;
    size_t m_iTotalWritten;
    int m_iState;
    BOOL m_bFinished;
};




#endif