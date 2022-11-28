#ifndef RELECH_DATABASECOVER_H__
#define RELECH_DATABASECOVER_H__
#include "stdafx.h"
#define DATABASECOVER_HOME 0
BOOL DataBaseCover_SetHomeCover(const char* pszMediaAddr);
char* DataBaseCover_GetHomeCover();
BOOL DataBaseCover_RemoveHomeCover();
#endif