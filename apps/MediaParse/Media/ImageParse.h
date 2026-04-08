#pragma once
#include "stdafx.h"
/*
CImageParse imageParse;
imageParse.Parse("/home/relech/Share/1736471891851.jpg");
imageParse.GetThumbnail("/home/relech/Share/1736471891851.jpg", "/home/relech/Share/1736471891851_100X100.jpg", 100, 100);
imageParse.Out();
*/
class CImageParse
{
public:
    CImageParse();
    ~CImageParse();
    BOOL Parse(const char* psz, BOOL bLog = FALSE);
    float LocationItemToFloat(const char* pszLocationItem);
    void Out();
    void OutGps();
    BOOL GetThumbnail(const char* pszInput, const char* pszOut, int iThumbnailWidth, int iThumbnailHeight);
    int GetOrientation();
    int GetWidth();
    int GetHeight();
    float GetLatitude();
    float GetLongtitude();
    std::string GetCreateTime();
    vector<string> GetLocaiton();
private:
    int m_iOrientation;
    int m_iWidth;
    int m_iHeight;
    float m_dLatitude;
    float m_dLongitude;
    std::string m_strCreatTime;
};
