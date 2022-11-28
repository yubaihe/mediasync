#ifndef RELECH_CLEARCACHE_H__
#define RELECH_CLEARCACHE_H__
#include "stdafx.h"
#include "DataBaseMediaJiShu.h"

typedef enum
{
    CLEARCACHE_FAILED = -1,
    CLEARCACHE_PROCESS,
    CLEARCACHE_SUCCESS
    
}CLEARCACHESTATUS;
struct ClearCache
{
    CLEARCACHESTATUS eStatus;
    int iStepCount;
    int iCurStepIndex;
    int iPrecent;
    HANDLE hClearHandle;
    ResetJiShuCallBack JiShuCallBack;
};


BOOL ClearCache_Start(struct ClearCache** pClearCache);
void ClearCache_Release(struct ClearCache* pClearCache);
#endif