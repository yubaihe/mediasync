#include "VideoMetaParse.h"
#include "JsonUtil.h"
#include "CommonUtil.h"
// #include <shobjidl.h>   // SHGetPropertyStoreFromParsingName, etc
// #include <propkey.h>    // PKEY_Music_AlbumArtist
// #include <propvarutil.h>// InitPropVariantFromString, needs shlwapi.lib
/**
{
    "streams": [
        {
            "index": 0,
            "codec_name": "h264",
            "codec_long_name": "H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10",
            "profile": "Constrained Baseline",
            "codec_type": "video",
            "codec_time_base": "73577/4500000",
            "codec_tag_string": "avc1",
            "codec_tag": "0x31637661",
            "width": 1280,
            "height": 720,
            "coded_width": 1280,
            "coded_height": 720,
            "closed_captions": 0,
            "has_b_frames": 0,
            "sample_aspect_ratio": "1:1",
            "display_aspect_ratio": "16:9",
            "pix_fmt": "yuv420p",
            "level": 42,
            "chroma_location": "left",
            "refs": 1,
            "is_avc": "true",
            "nal_length_size": "4",
            "r_frame_rate": "15/1",
            "avg_frame_rate": "2250000/73577",
            "time_base": "1/90000",
            "start_pts": 0,
            "start_time": "0.000000",
            "duration_ts": 441344,
            "duration": "4.903822",
            "bit_rate": "8160217",
            "bits_per_raw_sample": "8",
            "nb_frames": "150",
            "disposition": {
                "default": 1,
                "dub": 0,
                "original": 0,
                "comment": 0,
                "lyrics": 0,
                "karaoke": 0,
                "forced": 0,
                "hearing_impaired": 0,
                "visual_impaired": 0,
                "clean_effects": 0,
                "attached_pic": 0,
                "timed_thumbnails": 0
            },
            "tags": {
                "rotate": "90",
                "creation_time": "2020-04-28T08:25:43.000000Z",
                "language": "eng",
                "handler_name": "VideoHandle"
            },
            "side_data_list": [
                {
                    "side_data_type": "Display Matrix",
                    "displaymatrix": "\n00000000:            0       65536           0\n00000001:       -65536           0           0\n00000002:            0           0  1073741824\n",
                    "rotation": -90
                }
            ]
        }
    ],
    "format": {
        "filename": "C:\\Users\\Administrator\\Desktop\\test\\video\\VID_20200428_162537_1.mp4",
        "nb_streams": 2,
        "nb_programs": 0,
        "format_name": "mov,mp4,m4a,3gp,3g2,mj2",
        "format_long_name": "QuickTime / MOV",
        "start_time": "0.000000",
        "duration": "4.971000",
        "size": "5065793",
        "bit_rate": "8152553",
        "probe_score": 100,
        "tags": {
            "major_brand": "isom",
            "minor_version": "0",
            "compatible_brands": "isom3gp4",
            "creation_time": "2020-04-28T08:25:43.000000Z",
            "location": "+32.0530+118.7806/",
            "location-eng": "+32.0530+118.7806/"
        }
    }
}
*/
CVideoMetaParse::CVideoMetaParse()
{
    m_strLong = "";
    m_strLat = "";
    m_iCreateTime = 0;
    m_iDuration = 0;
    m_iWidth = 0;
    m_iHeight = 0;
    m_strFile = "";
}

BOOL CVideoMetaParse::ParseVideoMeta(string strFile)
{
    m_strFile = strFile;
   
    /*IPropertyStore* store = NULL;
    SHGetPropertyStoreFromParsingName(L"C:\\Users\\Administrator\\Desktop\\Camera\\VID_20210102_172602.mp4",
        NULL, GPS_READWRITE, __uuidof(IPropertyStore), (void**)&store);
    PROPVARIANT variant;
    store->GetValue(PKEY_Video_FrameWidth, &variant);
    int iWidth = variant.iVal;
    store->GetValue(PKEY_Video_FrameHeight, &variant);
    int iHeight = variant.iVal;
    store->GetValue(PKEY_Video_Director, &variant);
    int iDirector = variant.intVal;
    store->GetValue(PKEY_Media_Duration, &variant);
    int iDuration = variant.intVal;
    store->Release();*/

	string strCmd = CCommonUtil::StringFormat("ffprobe -i \"%s\" -v quiet -print_format json -show_format -select_streams v -show_streams ", m_strFile.c_str());
    string strRet = CCommonUtil::ExecuteCmd(strCmd.c_str());
    CJsonUtil jsonRoot;
    BOOL bRet = jsonRoot.Parse(strRet.c_str());
    if (FALSE == bRet)
    {
        return FALSE;
    }
    string str = jsonRoot.GetObjectString("streams", 0);
    CJsonUtil jsonStream;
    jsonStream.Parse(str.c_str());

    m_iWidth = jsonStream.GetInt("width");
    m_iHeight = jsonStream.GetInt("height");
    CJsonUtil jsonTag;
    bRet = jsonTag.Parse(jsonStream.GetStrng("tags"));
    if (FALSE == bRet)
    {
        return FALSE;
    }
    int iRotate = jsonTag.GetInt("rotate");
    if (abs(iRotate) == 90)
    {
        int iTmp = m_iWidth;
        m_iWidth = m_iHeight;
        m_iHeight = iTmp;
    }

    m_iDuration = ceil(atof(jsonRoot.GetObjectString("format", "duration")));
    CJsonUtil jsonFormat;
    bRet = jsonFormat.Parse(jsonRoot.GetStrng("format"));
    if (FALSE == bRet)
    {
        return FALSE;
    }
    
    const char* pszCreateTime = jsonFormat.GetObjectString("tags", "creation_time");
    if (strlen(pszCreateTime) == 0)
    {
        m_iCreateTime = 0;
    }
    else
    {
        string strTime = CCommonUtil::StringSplit(pszCreateTime, ".")[0];
        strTime = CCommonUtil::ReplaceStr(strTime, "T", " ");
        m_iCreateTime = atol(CCommonUtil::TimeToSec(strTime).c_str());
    }

    const char* pszLocation = jsonFormat.GetObjectString("tags", "location");
    if (strlen(pszLocation) == 0)
    {
        m_strLat = "";
        m_strLong = "";
    }
    else
    {
        string strLocation(pszLocation);
        string strLocaton = CCommonUtil::ReplaceStr(strLocation, "/", "");
        strLocaton = CCommonUtil::ReplaceStr(strLocation, "+", " ");
        strLocaton = CCommonUtil::TrimStr(strLocation);
        vector<string> locationVec = CCommonUtil::StringSplit(strLocaton, " ");
        m_strLat = locationVec[0];
        m_strLong = locationVec[1];
    }
	return TRUE;
}

uint64_t CVideoMetaParse::GetCreateTime()
{
    if (m_iCreateTime == 0)
    {
        struct stat sb;
        stat(m_strFile.c_str(), &sb);
        time_t time = sb.st_atime > sb.st_mtime ? sb.st_mtime : sb.st_atime;
        time = time > sb.st_ctime ? sb.st_ctime : time;
        string strTime = CCommonUtil::SecToTime(time);
        m_iCreateTime = atol(CCommonUtil::TimeToSec(strTime).c_str());
    }
    return m_iCreateTime;
}

vector<string> CVideoMetaParse::GetLocaiton()
{
    vector<string> vector;
    if (m_strLat.size() == 0 || m_strLong.size() == 0)
    {
        vector.push_back("0");
        vector.push_back("0");
        return vector;
    }

    vector.push_back(m_strLong);
    vector.push_back(m_strLat);
    
    return vector;
}

int CVideoMetaParse::GetDuration()
{
    return m_iDuration;
}

int CVideoMetaParse::GetWidth()
{
    return m_iWidth;
}

int CVideoMetaParse::GetHeight()
{
    return m_iHeight;
}

void CVideoMetaParse::OUt()
{
    string strCtime = CCommonUtil::SecToTime(m_iCreateTime);
    char szJson[1024] = {0};
    if (m_strLat.size() == 0 || m_strLong.size() == 0)
    {
        sprintf(szJson, "{\"width\":%d,\"height\":%d,\"duration\":%d,\"lat\":\"0\",\"long\":\"0\",\"ctime\":\"%s\"}",
            m_iWidth, m_iHeight, m_iDuration, strCtime.c_str());
    }
    else
    {
        sprintf(szJson, "{\"width\":%d,\"height\":%d,\"duration\":%d,\"lat\":\"%s\",\"long\":\"%s\",\"ctime\":\"%s\"}",
        m_iWidth, m_iHeight, m_iDuration, m_strLat.c_str(), m_strLong.c_str(), strCtime.c_str());
    }
    printf("%s\n", szJson);

}

void CVideoMetaParse::OUtGps()
{
    if (m_strLat.size() == 0 || m_strLong.size() == 0)
    {
        printf("%s","");
    }
    else
    {
        printf("%s&%s", m_strLong.c_str(), m_strLat.c_str());
    }
}