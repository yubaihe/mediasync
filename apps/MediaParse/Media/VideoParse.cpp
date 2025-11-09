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
    BOOL bGetRotation = FALSE;
    int iRotation = 0;
    const AVCodec* pDecoder = NULL;
    int iVideoStreamIndex = av_find_best_stream(pContext, AVMEDIA_TYPE_VIDEO, -1, -1, &pDecoder, 0);
    if (iVideoStreamIndex < 0 || NULL == pDecoder)
    {
        return 0;
    }
    const char* pRotatekeys[] = {"rotate", "rotation", NULL};  // 兼容不同键名
    AVDictionaryEntry* pEntry = NULL;
    while ((pEntry = av_dict_get(pContext->streams[iVideoStreamIndex]->metadata, "", pEntry, AV_DICT_IGNORE_SUFFIX)))
    {
        std::string strKey(pEntry->key);
        std::transform(strKey.begin(), strKey.end(), strKey.begin(), ::tolower);
        for (int i = 0; pRotatekeys[i]; ++i)
        {
            if(0 == strKey.compare(pRotatekeys[i]))
            {
                iRotation = atoi(pEntry->value);
                bGetRotation = TRUE;
                break;
            }
        }
        if(TRUE == bGetRotation)
        {
            break;
        }
    }
    // if(FALSE == bGetRotation)
    // {
    //     // 初始化解码器
    //     AVCodecContext* pCodecContext = avcodec_alloc_context3(pDecoder);
    //     AVCodecParameters* pCodecParameters = pContext->streams[iVideoStreamIndex]->codecpar;
    //     avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    //     if (avcodec_open2(pCodecContext, pDecoder, nullptr) < 0)
    //     {
    //         printf("解码器初始化失败\n");
    //         avcodec_free_context(&pCodecContext);
    //         return 0;
    //     }
    //     AVPacket* pPacket = av_packet_alloc();
    //     AVFrame* pSrcFrame = av_frame_alloc();
    //     int iFrameIndex = 0;
    //     while (av_read_frame(pContext, pPacket) >= 0) 
    //     {
    //         iFrameIndex++;
    //         if(iFrameIndex > 5)
    //         {
    //             break;
    //         }
    //         if (pPacket->stream_index == iVideoStreamIndex && 
    //             avcodec_send_packet(pCodecContext, pPacket) == 0 &&
    //             avcodec_receive_frame(pCodecContext, pSrcFrame) == 0)
    //             {
    //                 AVFrameSideData* pFrameSideData = av_frame_get_side_data(pSrcFrame, AV_FRAME_DATA_DISPLAYMATRIX);  
    //                 if (NULL != pFrameSideData && pFrameSideData->size >= 9 * sizeof(int32_t)) 
    //                 {  
    //                     int32_t* pMatrix = (int32_t *)pFrameSideData->data;   
    //                     iRotation = av_display_rotation_get(pMatrix);
    //                     bGetRotation = TRUE;
    //                 }  
    //                 if(FALSE == bGetRotation)
    //                 {
    //                     size_t iSize;  
    //                     uint8_t* pSideData = av_packet_get_side_data(pPacket, AV_PKT_DATA_DISPLAYMATRIX, &iSize);  
    //                     if (NULL != pSideData && iSize >= 9 * sizeof(int32_t)) 
    //                     {  
    //                         int32_t* pMatrix = (int32_t*)pSideData;  
    //                         iRotation = av_display_rotation_get(pMatrix);
    //                         bGetRotation = TRUE;
    //                     }
    //                 }
    //             }
    //             av_packet_unref(pPacket);
    //             if(TRUE == bGetRotation)
    //             {
    //                 break;
    //             }
    //     }
    //     av_packet_free(&pPacket);
    //     av_frame_free(&pSrcFrame);
    //     avcodec_flush_buffers(pCodecContext);
    //     av_seek_frame(pContext, iVideoStreamIndex, -1, AVSEEK_FLAG_BACKWARD);
    //     av_seek_frame(pContext, -1, -1, AVSEEK_FLAG_BACKWARD);
    //     avcodec_free_context(&pCodecContext);
    // }
    return TRUE == bGetRotation ? abs(iRotation):0;
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