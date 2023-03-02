#ifndef RELECH_MAP_H_
#define RELECH_MAP_H_
#include "stdafx.h"
#define DEFAULT_MAPSIZE 200
typedef struct RelechMapElem
{
	struct RelechMapElem* pNext;
	struct RelechMapElem* pPrev;
	void* pData;
}RelechMapElem;
typedef struct RelechMapVector
{
	int iIndex;
	int iKeyLen;
	int iValueLen;
	void* pKey;
	void* pValue;
	struct RelechMapElem* pNext;
}RelechMapVector;

typedef struct RelechMap
{
	int iMapSize;
	int iItemLen;
	char** pMap;
}RelechMap;

RelechMap* RelechMap_Init(int iMapSize);
void RelechMap_RemoveAll(RelechMap* pMap);
void RelechMap_ClearMap(RelechMap** pMap);
RelechMapVector RelechMap_GetHead(RelechMap* pMap);
int RelechMap_GetNext(RelechMap* pMap, RelechMapVector* pVector);


int RelechMap_SetAt(RelechMap* pMap, char* pKey, void* pValue, int iValueLen);
void* RelechMap_LookUp(RelechMap* pMap, char* pKey);
int RelechMap_Earse(RelechMap* pMap, char* pKey);
//int
int RelechMap_SetAtInt(RelechMap* pMap, int iKey, void* pValue, int iValueLen);
void* RelechMap_LookUpInt(RelechMap* pMap, int iKey);
int RelechMap_EarseInt(RelechMap* pMap, int iKey);

#endif