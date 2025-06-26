#pragma once
#include "stdafx.h"
#include <algorithm>
#include <string>
#include <cctype>
#include <stdlib.h>
extern "C" { 
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <stdexcept>
    #include <libavutil/display.h>
    #include <libavutil/pixfmt.h>  
    #include <libavutil/pixdesc.h> 
    #include <libavfilter/avfilter.h>
    #include <libavfilter/buffersrc.h>
    #include <libavfilter/buffersink.h>
}
/*
CVideoParse videoParse;
videoParse.Parse("/home/relech/Share/1736471896276.mp4");
videoParse.GetThumbnail("/home/relech/Share/1736471896276.mp4", "/home/relech/Share/1736471896276_100X100.jpg", 100, 100);
videoParse.Out();
*/
class CVideoParse
{
public:
    CVideoParse();
    ~CVideoParse();
    BOOL Parse(const char* psz);
    BOOL GetThumbnail(const char* pszInput, const char* pszOut, int iThumbnailWidth, int iThumbnailHeight);
    void Out();
    void OutGps();
    int GetWidth();
    int GetHeight();
    float GetLatitude();
    float GetLongtitude();
    std::string GetCreateTime();
    int GetDurationSec();
    std::vector<std::string> GetLocaiton();
private:
    int GetRotation(AVFormatContext* pContext);
private:
    int m_iWidth;
    int m_iHeight;
    float m_dLatitude;
    float m_dLongitude;
    uint32_t m_iDurationSec;
    std::string m_strCreatTime;
};


