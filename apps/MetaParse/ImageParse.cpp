 #include <vector>
 #include <math.h>
 #include <memory.h>
 #include "ImageParse.h"
 #include "exiv2/image.hpp"
 #include "CommonUtil.h"
 #include <json/json.h>

CImageParse::CImageParse()
{
    m_strOrientation = "";
    m_strWidth = "";
    m_strHeight = "";
    m_strLocationLatitude = "";
    m_strLocationLongitude = "";
    m_strCreatTime = "";
}

CImageParse::~CImageParse()
{

}

void CImageParse::Parse(const char* psz)
{
    std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(psz);
     //读取照片的exif信息
    image->readMetadata();
    Exiv2::ExifData ed = image->exifData();//得到exif信息
    
    if(!ed.empty())
    {
        for (Exiv2::ExifData::const_iterator i = ed.begin(); i != ed.end(); ++i) {
            //printf("%s==>%s\n", i->key().c_str(), i->value().toString().c_str());
            if(m_strOrientation.length() > 0 && 
                m_strWidth.length() > 0 && 
                m_strHeight.length() > 0 &&
                m_strLocationLongitude.length() > 0 && 
                m_strLocationLatitude.length() > 0 && 
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
                m_strOrientation = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.PixelXDimension"))
            {
                //Exif.Photo.PixelXDimension==>4000
                m_strWidth = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.PixelYDimension"))
            {
                //Exif.Photo.PixelYDimension==>3000
                m_strHeight = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.GPSInfo.GPSLatitude"))
            {
                //Exif.GPSInfo.GPSLatitude==>0/0 0/0 0/0
                m_strLocationLatitude = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.GPSInfo.GPSLongitude"))
            {
                //Exif.GPSInfo.GPSLongitude==>0/0 0/0 0/0
                m_strLocationLongitude = i->value().toString();
                continue;
            }
            if( 0 == i->key().compare("Exif.Photo.DateTimeOriginal"))
            {
                //Exif.Photo.DateTimeOriginal==>2021:01:08 23:45:11
                m_strCreatTime = i->value().toString();
                continue;
            }
        }
    }
    if(m_strCreatTime.size() == 0)
    {
        m_strCreatTime = CCommonUtil::SecToTime(CCommonUtil::CurTimeSec());
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
    //printf("time:%s\n", m_strCreatTime.c_str());
    //printf("strLocationLatitude:%s\n", m_strLocationLatitude.c_str());
    //printf("strLocationLongitude:%s\n", m_strLocationLongitude.c_str());
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


void CImageParse::OUt()
{
    float dLat = LocationItemToFloat(m_strLocationLatitude.c_str());
    float dLong = LocationItemToFloat(m_strLocationLongitude.c_str());
    char szJson[1024] = {0};
    if(dLat == 0 || dLong == 0)
    {
        sprintf(szJson, "{\"width\":%s,\"height\":%s,\"orientation\":%s,\"lat\":\"0\",\"long\":\"0\",\"ctime\":\"%s\"}",
        m_strWidth.c_str(), m_strHeight.c_str(), m_strOrientation.c_str(), m_strCreatTime.c_str());
    }
    else
    {
        sprintf(szJson, "{\"width\":%s,\"height\":%s,\"orientation\":%s,\"lat\":\"%.6f\",\"long\":\"%.6f\",\"ctime\":\"%s\"}",
        m_strWidth.c_str(), m_strHeight.c_str(), m_strOrientation.c_str(), dLat, dLong, m_strCreatTime.c_str());
    }
    printf("%s", szJson);
}

void CImageParse::OUtGps()
{
    float dLat = LocationItemToFloat(m_strLocationLatitude.c_str());
    float dLong = LocationItemToFloat(m_strLocationLongitude.c_str());
 //   char szJson[1024] = {0};
    if(dLat == 0 || dLong == 0)
    {
        printf("0&0");
    }
    else
    {
        printf("%.6f&%.6f", dLong, dLat);
    }
}