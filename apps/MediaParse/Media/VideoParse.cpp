#include "VideoParse.h"
#include "CommonUtil.h"
#include "CpuDetect.h"
#include "../Util/CommonUtil.h"
extern BOOL g_bExit;
CVideoParse::CVideoParse()
{
    m_iWidth = 0;
    m_iHeight = 0;
    m_dLatitude = 0.0f;
    m_dLongitude = 0.0f;
    m_strCreatTime = "";
}

CVideoParse::~CVideoParse()
{
}
int CVideoParse::GetRotation(AVFormatContext* pContext)
{
    int iVideoStreamIndex = av_find_best_stream(pContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (iVideoStreamIndex < 0)
    {
        return 0;
    }

    AVStream* pAVStream = pContext->streams[iVideoStreamIndex];
    int iRotation = 0;
    const int32_t* pDisplaymatrix = NULL;
    // 1. 优先尝试新版 API：从 codecpar 的侧数据数组中获取显示矩阵
    // 新版 API 使用 av_packet_side_data_get 从 coded_side_data 数组查找
    const AVPacketSideData* pAVPacketSideData = av_packet_side_data_get(
        pAVStream->codecpar->coded_side_data,
        pAVStream->codecpar->nb_coded_side_data,
        AV_PKT_DATA_DISPLAYMATRIX
    );
    if (NULL != pAVPacketSideData)
    {
        // 确保数据大小足够包含 9 个 int32_t 的矩阵
        if (pAVPacketSideData->size >= 9 * sizeof(int32_t))
        {
            pDisplaymatrix = (const int32_t*)pAVPacketSideData->data;
        }
    }

    // 3. 如果找到了显示矩阵数据，从中解析出旋转角度
    if (NULL != pDisplaymatrix)
    {
        double theta = av_display_rotation_get(pDisplaymatrix);
        // 将浮点数角度四舍五入取整，并规范化到 [0, 360) 范围
        iRotation = (int)llrint(theta);
        iRotation = (iRotation % 360 + 360) % 360;
    }

    // 4. 如果侧数据中没有旋转信息，回退到检查元数据（如 "rotate" 标签）
    if (iRotation == 0)
    {
        AVDictionaryEntry* pAVDictionaryEntry = av_dict_get(pAVStream->metadata, "rotate", NULL, AV_DICT_MATCH_CASE);
        if (NULL == pAVDictionaryEntry)
        {
            // 有些文件可能使用 "rotation" 键
            pAVDictionaryEntry = av_dict_get(pAVStream->metadata, "rotation", NULL, AV_DICT_MATCH_CASE);
        }
        if (NULL != pAVDictionaryEntry)
        {
            iRotation = atoi(pAVDictionaryEntry->value) % 360;
        }
    }

    return iRotation;
}
BOOL CVideoParse::Parse(const char* psz)
{
    struct stat pathStat;
    if (lstat(psz, &pathStat) != -1)
    {
        m_strCreatTime = CCommonUtil::SecToTime(pathStat.st_ctime);
    }
    
    //av_log_set_level(AV_LOG_INFO);
    //avformat_network_init();
    AVFormatContext* pContext = NULL;
    int iRet = avformat_open_input(&pContext, psz, NULL, NULL);
    if (iRet < 0) 
    {  
        char szError[AV_ERROR_MAX_STRING_SIZE] = {0};  
        av_strerror(iRet, szError, sizeof(szError));  
        printf("打开文件失败: %s\n", szError); 
        return FALSE;  
    }
    if (avformat_find_stream_info(pContext, NULL) < 0) 
    {  
        printf("获取流信息失败\n");  
        avformat_close_input(&pContext);  
        return FALSE;  
    }
    //pContext->duration; // 单位：微秒
    m_iDurationSec = (uint32_t)(pContext->duration / AV_TIME_BASE);
    printf("during:%d\n", m_iDurationSec);  
    AVDictionaryEntry* pEntry = NULL;  
    while ((pEntry = av_dict_get(pContext->metadata, "", pEntry, AV_DICT_IGNORE_SUFFIX)))
    {
        printf("元数据: key=%s, value=%s\n", pEntry->key, pEntry->value);  
        if (strcmp(pEntry->key, "location") == 0 || strstr(pEntry->key, "gps") != NULL) 
        {  
            char szLat[20] = {0};
            char szLong[20] = {0};
            sscanf(pEntry->value, "+%[0-9.]%*[/+]%[0-9.]", szLat, szLong);
            m_dLatitude = atof(szLat);
            m_dLongitude = atof(szLong);
            //printf("GPS 信息: lat=%f, long=%f\n", m_dLatitude, m_dLongitude);
        }
        if (strcmp(pEntry->key, "creation_time") == 0)
        {
            m_strCreatTime = CCommonUtil::TransTime(pEntry->value);
            //printf("CreateTime 信息: %s\n", m_strCreatTime.c_str()); 
        }
        else 
        {  
            printf("元数据: key=%s, value=%s\n", pEntry->key, pEntry->value);  
        }  
    }
    int iStreamIndex = av_find_best_stream(pContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if(iStreamIndex < 0)
    {
        avformat_close_input(&pContext);
        return FALSE;
    }
    AVStream* pStream = pContext->streams[iStreamIndex];
    int iRotation = GetRotation(pContext);
    printf("Rotation:%d\n", iRotation);
    if (iRotation == 90 || iRotation == 270)
    {
        m_iWidth = pStream->codecpar->height;
        m_iHeight = pStream->codecpar->width;
    }
    else
    {
        m_iWidth = pStream->codecpar->width;
        m_iHeight = pStream->codecpar->height;
    }
    avformat_close_input(&pContext);
    return TRUE;
}
BOOL CVideoParse::GetThumbnail(const char* pszInput, const char* pszOut, int iThumbnailWidth, int iThumbnailHeight)
{
    //ffmpeg -ss 00:00:01 -i /home/relech/Share/1736471896276.mp4 -vf "scale=100:100,setsar=1:1" -frames:v 1 /home/relech/Share/1736471896276_100X100.jpg
    //-ss 00:00:01 第一秒的帧
    //-vf "scale=100:100,setsar=1:1" 过滤器长宽100*100 setsar=1:1：设置像素纵横比为 1:1，确保每个像素是方形的。这在一些显示设备上有助于防止图像失真。
    //-frames:v 1 指定输出1帧
    //-y 不要给Overwrite的提示
    //-loglevel quiet 不要有日志输出
   
    CCommonUtil::ExecuteCmdWithOutReplay("ffmpeg -y -loglevel quiet -ss 00:00:01 -i \"%s\" -vf \"scale=%d:%d,setsar=1:1\" -frames:v 1 \"%s\"", pszInput, iThumbnailWidth, iThumbnailHeight, pszOut);
    system("sync");
    return CCommonUtil::CheckFileExist(pszOut);
}
void CVideoParse::Out()
{
    char szJson[1024] = {0};
    sprintf(szJson, "{\"width\":%d,\"height\":%d,\"lat\":\"%.6f\",\"long\":\"%.6f\",\"ctime\":\"%s\",\"duration\":%d}",
        m_iWidth, m_iHeight, m_dLatitude, m_dLongitude, m_strCreatTime.c_str(), m_iDurationSec);
    printf("%s\n", szJson);
}

void CVideoParse::OutGps()
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
int CVideoParse::GetWidth()
{
    return m_iWidth;
}
int CVideoParse::GetHeight()
{
    return m_iHeight;
}
float CVideoParse::GetLatitude()
{
    return m_dLatitude;
}
float CVideoParse::GetLongtitude()
{
    return m_dLongitude;
}
std::string CVideoParse::GetCreateTime()
{
    return m_strCreatTime;
}
int CVideoParse::GetDurationSec()
{
    return m_iDurationSec;
}
std::vector<std::string> CVideoParse::GetLocaiton()
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