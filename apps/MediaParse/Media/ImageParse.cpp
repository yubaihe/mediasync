#include <vector>
#include <math.h>
#include <memory.h>
#include "ImageParse.h"
#include "exiv2/image.hpp"
#include "../Util/CommonUtil.h"
#include "VideoParse.h"
#include "CpuDetect.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
extern BOOL g_bExit;
CImageParse::CImageParse()
{
    m_iOrientation = -100;
    m_iWidth = 0;
    m_iHeight = 0;
    m_dLatitude = 0.0f;
    m_dLongitude = 0.0f;
    m_strCreatTime = "";
}

CImageParse::~CImageParse()
{

}
//在程序的最后需要加上一句 Exiv2::XmpParser::terminate();
BOOL CImageParse::Parse(const char* psz, BOOL bLog/* = FALSE*/)
{
    try
    {
        struct stat filestat;
        if (stat(psz, &filestat) == 0) 
        {
            m_strCreatTime = CCommonUtil::SecToTime(filestat.st_ctime);
        }
        else
        {
            return FALSE;
        }
        BOOL bSuccess = stbi_info(psz, &m_iWidth, &m_iHeight, NULL);
        if(FALSE == bSuccess)
        {
            return FALSE;
        }
        std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(psz);
        //读取照片的exif信息
        image->readMetadata();
        Exiv2::ExifData ed = image->exifData();//得到exif信息
        if(TRUE == ed.empty())
        {
            image.reset();
            return TRUE;
        }
        m_iOrientation = -100;
        std::string strLatitude = "";
        std::string strLongitude = "";
        for (Exiv2::ExifData::const_iterator i = ed.begin(); i != ed.end(); ++i)
        {
            if(TRUE == bLog)
            {
                printf("%s==>%s\n", i->key().c_str(), i->value().toString().c_str());
            }
            if(m_iOrientation >= 0 && 
                m_iWidth > 0 && 
                m_iHeight > 0 &&
                strLatitude.length() > 0 && 
                strLongitude.length() > 0 && 
                m_strCreatTime.length() > 0
                )
            {
                break;
            }
            if( 0 == i->key().compare("Exif.Image.DateTime"))
            {
                //Exif.Image.DateTime==>2021:01:08 23:45:11
                m_strCreatTime = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.Image.Orientation"))
            {
                //Exif.Image.Orientation==>6
                std::string strValue = i->value().toString();
                m_iOrientation = atoi(strValue.c_str());
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.PixelXDimension"))
            {
                //Exif.Photo.PixelXDimension==>4000
                std::string strValue = i->value().toString();
                m_iWidth = atoi(strValue.c_str());
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.PixelYDimension"))
            {
                //Exif.Photo.PixelYDimension==>3000
                std::string strValue = i->value().toString();
                m_iHeight = atoi(strValue.c_str());
                continue;
            }
            if( 0 == i->key().compare("Exif.GPSInfo.GPSLatitude"))
            {
                //Exif.GPSInfo.GPSLatitude==>0/0 0/0 0/0
                strLatitude = i->value().toString();
                m_dLatitude = LocationItemToFloat(strLatitude.c_str());
                continue;
            }
            if( 0 == i->key().compare("Exif.GPSInfo.GPSLongitude"))
            {
                //Exif.GPSInfo.GPSLongitude==>0/0 0/0 0/0
                strLongitude = i->value().toString();
                m_dLongitude = LocationItemToFloat(strLongitude.c_str());
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.DateTimeOriginal"))
            {
                //Exif.Photo.DateTimeOriginal==>2021:01:08 23:45:11
                m_strCreatTime = i->value().toString();
                continue;
            }
        }
        if(m_strCreatTime.size() == 0)
        {
            struct stat filestat;
            if (stat(psz, &filestat) == 0) 
            {
                m_strCreatTime = CCommonUtil::SecToTime(filestat.st_ctime);
            }
        }
        else
        {
            std::string strTime = m_strCreatTime;
            std::vector<std::string> vecTime = CCommonUtil::StringSplit(strTime, " ");
            strTime = CCommonUtil::ReplaceStr(vecTime[0], ":", "-");
            strTime.append(" ");
            strTime.append(vecTime[1]);
            //m_iCreateTime = atol(CCommonUtil::TimeToSec(strTime).c_str());
            m_strCreatTime = strTime;
        }
        if(m_iOrientation == 5 ||   //旋转90度顺时针，然后水平翻转
            m_iOrientation == 6 ||  //旋转90度逆时针
            m_iOrientation == 7 ||  //旋转90度顺时针，然后垂直翻转
            m_iOrientation == 8   //旋转270度逆时针（或旋转90度顺时针两次）
            )
        {
            int iTemp = m_iWidth;
            m_iWidth = m_iHeight;
            m_iHeight = iTemp;
        }
        image.reset();
        if(m_iWidth == 0 || m_iHeight == 0)
        {
            string strRet = CCommonUtil::ExecuteCmd("ffprobe -loglevel quiet -select_streams v:0 -show_entries stream=width,height -of csv=p=0 \"%s\"", psz);
            if(TRUE == bLog)
            {
                printf("strRet:%s\n", strRet.c_str());
            }
            strRet = CCommonUtil::ReplaceStr(strRet, "\n", "");
            std::vector<std::string> vec = CCommonUtil::StringSplit(strRet, ",");
            if(vec.size() == 2)
            {
                m_iWidth = atoi(vec[0].c_str());
                m_iHeight = atoi(vec[1].c_str());
            }
        }
        return TRUE;
    }
    catch(...)
    {
        return FALSE;
    }
}

float CImageParse::LocationItemToFloat(const char* pszLocationItem)
{
    if(strlen(pszLocationItem) == 0)
    {
        return 0;
    }
    
    std::vector<std::string> itemVec = CCommonUtil::StringSplit(pszLocationItem, " ");
    if(itemVec.size() != 3)
    {
        return 0;
    }
    
    std::vector<std::string> DuVec = CCommonUtil::StringSplit(itemVec[0], "/");
    std::vector<std::string> FenVec = CCommonUtil::StringSplit(itemVec[1], "/");
    std::vector<std::string> MiaoVec = CCommonUtil::StringSplit(itemVec[2], "/");

    
    if(DuVec.size() != 2 || atoi(DuVec[1].c_str()) == 0)
    {
        return 0;
    }
    if(FenVec.size() != 2 || atoi(FenVec[1].c_str()) == 0)
    {
        return 0;
    }
    if(MiaoVec.size() != 2 || atoi(MiaoVec[1].c_str()) == 0)
    {
        return 0;
    }
    float fDu = atof(DuVec[0].c_str()) / atof(DuVec[1].c_str());
    float fFen = atof(FenVec[0].c_str()) / atof(FenVec[1].c_str());
    float fMiao = atof(MiaoVec[0].c_str()) / atof(MiaoVec[1].c_str());
    //pow(60, 0);
    return (float)( fDu /pow(60, 0) + fFen/pow(60, 1) + fMiao/pow(60, 2));
}

void CImageParse::Out()
{
    char szJson[1024] = {0};
    sprintf(szJson, "{\"width\":%d,\"height\":%d,\"orientation\":%d,\"lat\":\"%.6f\",\"long\":\"%.6f\",\"ctime\":\"%s\"}",
        m_iWidth, m_iHeight, m_iOrientation, m_dLatitude, m_dLongitude, m_strCreatTime.c_str());
    printf("%s\n", szJson);
}

void CImageParse::OutGps()
{
 //   char szJson[1024] = {0};
    if(m_dLatitude == 0 || m_dLongitude == 0)
    {
        printf("0.000000&0.000000\n");
    }
    else
    {
        printf("%.6f&%.6f\n", m_dLongitude, m_dLatitude);
    }
}
BOOL CImageParse::GetThumbnail(const char* pszInput, const char* pszOut, int iThumbnailWidth, int iThumbnailHeight)
{
    //ffmpeg -i "/home/relech/Share/1736471891851.jpg" -vf "scale=100:100,setsar=1:1" /home/relech/Share/1736471891851_100X100.jpg
    //-vf "scale=100:100,setsar=1:1" 过滤器长宽100*100 setsar=1:1：设置像素纵横比为 1:1，确保每个像素是方形的。这在一些显示设备上有助于防止图像失真。
    //-y 不要给Overwrite的提示
    //-loglevel quiet 不要有日志输出
    CCommonUtil::ExecuteCmdWithOutReplay("ffmpeg -y -loglevel quiet -i \"%s\" -vf \"scale=%d:%d,setsar=1:1\" -update 1 -frames:v 1 \"%s\" ", pszInput, iThumbnailWidth, iThumbnailHeight, pszOut);
    system("sync");
    BOOL bSuccess = CCommonUtil::CheckFileExist(pszOut);
    if(FALSE == bSuccess)
    {
        printf("CheckFileExist false:%s\n", pszOut);
    }
    return bSuccess;
}
int CImageParse::GetOrientation()
{
    return m_iOrientation;
}
int CImageParse::GetWidth()
{
    return m_iWidth;
}
int CImageParse::GetHeight()
{
    return m_iHeight;
}
float CImageParse::GetLatitude()
{
    return m_dLatitude;
}
float CImageParse::GetLongtitude()
{
    return m_dLongitude;
}
std::string CImageParse::GetCreateTime()
{
    return m_strCreatTime;
}
std::vector<std::string> CImageParse::GetLocaiton()
{
    char szTmp[100] = {0};
    sprintf(szTmp, "%f", m_dLongitude);
    std::vector<std::string> vec;
    vec.push_back(szTmp);
    memset(szTmp, 0, sizeof(szTmp));
    sprintf(szTmp, "%f", m_dLatitude);
    vec.push_back(szTmp);
    return vec;
}