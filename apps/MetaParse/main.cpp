#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "ImageParse.h"
#include "CommonUtil.h"
#include "VideoMetaParse.h"


//MetaParse -c 检查环境
//MetaParse -t file 文件类型 image/video
//MetaParse -i file 文件详细信息 json格式
//MetaParse -g file GPS信息 long&lat
int main(int iArgc, char *pszArgsV[])
{
    if(iArgc == 2 && 0 == strcmp(pszArgsV[1], "-c"))
    {
        string strRet = CCommonUtil::ExecuteCmd("ffprobe -version");
        char sz[] = "ffprobe version";
        if (strRet.length() > strlen(sz) && 0 == memcmp(strRet.c_str(), sz, strlen(sz)))
        {
            //success next file
            string strRet = CCommonUtil::ExecuteCmd("file -v");
            if(0 == memcmp(strRet.c_str(), "file-", strlen("file-")))
            {
                int iPos = strRet.rfind(" ");
                string strMagicFile = strRet.substr(iPos + 1, strRet.length() - iPos - 1);
                strMagicFile = CCommonUtil::ReplaceStr(strMagicFile, "\n", "");
                BOOL bRet = CCommonUtil::CheckMagicFileExist(strMagicFile.c_str());
                if(FALSE == bRet)
                {
                    printf("magic error");
                    return 0;
                }
                printf("success");
                return 1;
            }
            else
            {
                printf("file error");
                return 0;
            }
        }
        else
        {
            printf("ffprobe error");
            return 0;
        }
        
    }
    if(iArgc == 3 && 0 == strcmp(pszArgsV[1], "-t"))
    {
        std::string strMime = CCommonUtil::GetMime(pszArgsV[2]);
        if(CCommonUtil::IsMimeTypeImage(strMime.c_str()))
        {
            printf("image");
            return 1;
        }
        if(CCommonUtil::IsMimeTypeVideo(strMime.c_str()))
        {
            printf("video");
            return 1;
        }
    }
    if(iArgc == 3 && 0 == strcmp(pszArgsV[1], "-i"))
    {
        std::string strMime = CCommonUtil::GetMime(pszArgsV[2]);
        if(CCommonUtil::IsMimeTypeImage(strMime.c_str()))
        {
            CImageParse imageParse;
            imageParse.Parse(pszArgsV[2]);
            imageParse.OUt();
            return 1;
        }
        if(CCommonUtil::IsMimeTypeVideo(strMime.c_str()))
        {
            CVideoMetaParse parse;
            parse.ParseVideoMeta(pszArgsV[2]);
            parse.OUt();
            return 1;
        }
    }

    if(iArgc == 3 && 0 == strcmp(pszArgsV[1], "-g"))
    {
        std::string strMime = CCommonUtil::GetMime(pszArgsV[2]);
        if(CCommonUtil::IsMimeTypeImage(strMime.c_str()))
        {
            CImageParse imageParse;
            imageParse.Parse(pszArgsV[2]);
            imageParse.OUtGps();
            return 1;
        }
        if(CCommonUtil::IsMimeTypeVideo(strMime.c_str()))
        {
            CVideoMetaParse parse;
            parse.ParseVideoMeta(pszArgsV[2]);
            parse.OUtGps();
            return 1;
        }
    }

    return 1;
}