#ifndef PACKAGE_H__
#define PACKAGE_H__
#include "stdafx.h"

#define  Hi  0
#define  Lo  1
#pragma pack(1)
struct BasePackage
{
    uint8_t szHead[3];
    uint8_t szRandNum[2];
    uint16_t iDataLen;
    uint16_t iCmdID;
    uint8_t szBuffer[1];
};
#pragma pack(0)
class CPackage
{
public:
    CPackage();
    ~CPackage();
    uint8_t* GetPackage(uint16_t iCommandID, uint8_t* pszData, uint16_t iDataLen, uint16_t* piDataLen);
    uint8_t* ParsePackage(uint8_t* pszSrcPackage, int iSrcPackageLen, int* piOutPackageLen, int* piCommandID);
private:
    long int GetRandomNum();
private:
    uint8_t* m_pszBuffer;
};



#endif