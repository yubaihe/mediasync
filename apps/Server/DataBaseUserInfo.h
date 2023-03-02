#ifndef RELECH_DATABASEUSERINFO__
#define RELECH_DATABASEUSERINFO__
#include "stdafx.h"
#include "DataBaseDriver.h"
typedef struct 
{
    //uint32_t最大值4,294,967,295
    uint32_t iId; 
    char* pszUserPwd;
    char* pszUserPwdTip;
}UserItem;
BOOL DataBaseUserInfo_GetRecordCount(int* iCount);
BOOL DataBaseUserInfo_DeleteRecord();
BOOL DataBaseUserInfo_AddRecord(UserItem* pItem);
BOOL DataBaseUserInfo_CheckPwd(const char* pszPwd);
BOOL DataBaseUserInfo_GetPwdTip(char** pszTip);
#endif