#ifndef IMAGEPARSE_H__
#define IMAGEPARSE_H__
#include "stdafx.h"
class CImageParse
{
public:
    CImageParse();
    ~CImageParse();
    void Parse(const char* psz);
    float LocationItemToFloat(const char* pszLocationItem);
    void OUt();
    void OUtGps();
private:
    std::string m_strOrientation;
    std::string m_strWidth;
    std::string m_strHeight;
    std::string m_strLocationLatitude;
    std::string m_strLocationLongitude;
    std::string m_strCreatTime;
};



#endif