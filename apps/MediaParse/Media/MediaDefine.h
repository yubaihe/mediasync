#pragma once
#include "../stdafx.h"
#include <stdio.h>

#include "CommonUtil.h"
#define PIC_WIDTH 512
#define PIC_HEIGHT 384
typedef enum
{
    MEDIATYPE_NONE = 0,
    MEDIATYPE_IMAGE,
    MEDIATYPE_VIDEO
}MEDIATYPE;
class PhotoItem
{
public:
	int iWidth;
	int iHeight;
	string strName;
	long long iFileSize;
	int iMediaType; //image:1 video 2
	int iItemIndex;
    void Print()
    {
        printf("Index:%d Name:%s Size:%lld MediaType:%s Width:%d Height:%d\n",
            iItemIndex, strName.c_str(), iFileSize, iMediaType==MEDIATYPE_IMAGE?"IMAGE":"VIDEO",
            iWidth, iHeight);
    }
    void Clone(PhotoItem* pPhotoItem)
    {
        this->iWidth = pPhotoItem->iWidth;
        this->iHeight = pPhotoItem->iHeight;
        this->strName = pPhotoItem->strName;
        this->iFileSize = pPhotoItem->iFileSize;
        this->iMediaType = pPhotoItem->iMediaType;
        this->iItemIndex = pPhotoItem->iItemIndex;
    }
};
class PhotoExtra
{
public:
	string strFileName;
	vector<string> vecLocation;
	time_t iAddTime;
	int iDuration;
    void Print()
    {
        printf("Name:%s long:%s lat:%s AddTime:%ld Duration:%d\n",
            strFileName.c_str(), vecLocation[0].c_str(), vecLocation[1].c_str(), iAddTime, iDuration);
    }
    void Clone(PhotoExtra* pPhotoExtra)
    {
        strFileName = pPhotoExtra->strFileName;
        vecLocation = pPhotoExtra->vecLocation;
        iAddTime = pPhotoExtra->iAddTime;
        iDuration = pPhotoExtra->iDuration;
    }
};
