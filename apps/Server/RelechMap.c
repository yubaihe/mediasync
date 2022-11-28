#include "RelechMap.h"
RelechMap* RelechMap_Init(int iMapSize)
{
	char* pMapBuffer = (char*)malloc(sizeof(RelechMap));
	memset(pMapBuffer, 0, sizeof(RelechMap));
	RelechMap*pMap = (RelechMap*)pMapBuffer;
	if(iMapSize <= 0)
	{
		iMapSize = DEFAULT_MAPSIZE;
	}
	pMap->iMapSize = iMapSize;
	pMap->iItemLen = 0;
	int iPtrSize = sizeof(char*);
	if (NULL != pMap->pMap)
	{
		free(pMap->pMap);
		pMap->pMap = NULL;
	}
	pMap->pMap = (char**)malloc(iPtrSize * iMapSize);
	for (int i = 0; i < iMapSize; ++i)
	{
		pMap->pMap[i] = NULL;
	}
	return pMap;
}

void RelechMap_RemoveAll(RelechMap* pMap)
{
	int i = 0;
	for (; i < pMap->iMapSize; ++i)
	{
		RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[i];
		if (NULL == pElem)
		{
			continue;
		}
		do 
		{
			RelechMapElem* pNextElem = pElem->pNext;
			free(pElem->pData);
			pElem->pData = NULL;
			free(pElem);
			pElem = NULL;	
			pElem = pNextElem;
		}
		while(NULL != pElem);
		pMap->pMap[i] = NULL;
	}
	pMap->iItemLen = 0;
}

void RelechMap_ClearMap(RelechMap** pMap)
{
	RelechMap_RemoveAll(*pMap);
	free((*pMap)->pMap);
	(*pMap)->pMap = NULL;
	free(*pMap);
	*pMap = NULL;
}

unsigned int RelechMap_BkdrHash(RelechMap* pMap, char *str)
{
	unsigned int seed = 131;
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF)/pMap->iMapSize;
}

unsigned int RelechMap_BkdrHashInt(RelechMap* pMap, int iValue)
{
	unsigned int seed = 131;
	unsigned int hash = 0;
	hash = hash * seed + iValue;
	return (hash & 0x7FFFFFFF)/pMap->iMapSize;
}

int RelechMap_SetAt(RelechMap* pMap, char* pKey, void* pValue, int iValueLen)
{
	if(pMap->iMapSize <= 0)
	{
		RelechMap_Init(DEFAULT_MAPSIZE);
	}
	int iKeyLen = -1;
	if ((NULL == pKey)||((iKeyLen = strlen(pKey)) <= 0))
	{
		return 0;
	}
	int iPos = RelechMap_BkdrHash(pMap, (char*)pKey)%pMap->iMapSize;
	//printf("key:%s,index:%d\n", pKey, iPos);
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		////////////////////////////////////////////////////
		//    4      4       char*       char*   //
		//int=>keylen//int=>datalen//  key   //   data   //
		//////////////////////////////////////////////////
		int iTotalDataLen = iValueLen + iIntLen * 2 + iKeyLen;
		char* pData = (char*)malloc(iTotalDataLen);
		memset(pData, 0, iTotalDataLen);
		RelechMapElem* pElem = (RelechMapElem*)malloc(sizeof(RelechMapElem));
		pMap->pMap[iPos] = (char*)pElem;
		pElem->pPrev = NULL;
		pElem->pNext = NULL;
		pElem->pData = pData;
		memcpy(pData, &iKeyLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iValueLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, pKey, iKeyLen);
		pData += iKeyLen;
		memcpy(pData, pValue, iValueLen);
		pMap->iItemLen++;
	}
	else
	{
		RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
		RelechMapElem* pNextElem = pElem;
		int iFound = -1;
		do 
		{
			pElem = pNextElem;
			if (-1 == iFound)
			{
				int iElemKeyLen = 0;
				char* pData = (char*)pElem->pData;
				memcpy((char*)&iElemKeyLen, pData, iIntLen);
				pData += iIntLen * 2;
				char* pElemKey = (char*)malloc(iElemKeyLen); 
				memcpy(pElemKey, pData, iElemKeyLen);
				if ((iElemKeyLen == iKeyLen) &&(!memcmp(pElemKey, (char*)pKey, iKeyLen)))
				{
					iFound = 1;
					//�ҵ���,ֻ��һ��
					if ((NULL == pElem->pPrev)&&(NULL == pElem->pNext))
					{
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pMap->pMap[iPos] = NULL;
					}
					else if ((NULL == pElem->pPrev)&&(NULL != pElem->pNext))
					{
						pMap->pMap[iPos] = (char*)pElem->pNext;
						pElem->pNext->pPrev = NULL;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = (RelechMapElem*)pMap->pMap[iPos];
						pElem->pPrev = NULL;
					}
					else if ((NULL != pElem->pPrev)&&(NULL == pElem->pNext))
					{
						RelechMapElem* pTmpElem = pElem->pPrev;
						pElem->pPrev->pNext = NULL;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = pTmpElem;
					}
					else if ((NULL != pElem->pPrev)&&(NULL != pElem->pNext))
					{
						pElem->pPrev->pNext = pElem->pNext;
						pElem->pNext->pPrev = pElem->pPrev;
						RelechMapElem* pTmpElem = pElem->pNext;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = pTmpElem;
					}
				}
				free(pElemKey);
				pElemKey = NULL;
			}	
		} while ((NULL != pElem)&&(NULL != (pNextElem = pElem->pNext)));

		int iTotalDataLen = iValueLen + iIntLen * 2 + iKeyLen;
		char* pData = (char*)malloc(iTotalDataLen);
		memset(pData, 0, iTotalDataLen);
		RelechMapElem* pNewElem = (RelechMapElem*)malloc(sizeof(RelechMapElem));
		if (NULL == pElem)
		{
			pMap->pMap[iPos] = (char*)pNewElem;
			pNewElem->pPrev = NULL;
			pNewElem->pNext = NULL;
		}
		else
		{
			pNewElem->pPrev = pElem;
			pElem->pNext = pNewElem;
		}
		pNewElem->pNext = NULL;
		pNewElem->pData = pData;
		memcpy(pData, &iKeyLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iValueLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, pKey, iKeyLen);
		pData += iKeyLen;
		memcpy(pData, pValue, iValueLen);
		if(-1 == iFound)
		{
			pMap->iItemLen++;
		}
	}
	return 1;
}

void* RelechMap_LookUp(RelechMap* pMap, char* pKey)
{
	if(pMap->iMapSize <= 0)
	{
		pMap = RelechMap_Init(DEFAULT_MAPSIZE);
		return NULL;
	}
	int iKeyLen = -1;
	if ((NULL == pKey)||((iKeyLen = strlen(pKey)) <= 0))
	{
		return NULL;
	}
	
	int iPos = RelechMap_BkdrHash(pMap, (char*)pKey)%pMap->iMapSize;
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		return NULL;
	}
	RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
	do 
	{
			int iElemKeyLen = 0;
			char* pData = (char*)pElem->pData;
			memcpy((char*)&iElemKeyLen, pData, iIntLen);
			pData += iIntLen * 2;
			char* pElemKey = (char*)malloc(iElemKeyLen); 
			memcpy(pElemKey, pData, iElemKeyLen);
			if ((iKeyLen == iElemKeyLen)&&(!memcmp(pElemKey, (char*)pKey, iElemKeyLen)))
			{
				//printf("lookup:key:%s,index:%d\n", pKey, iPos);
				char* pRetData = pData + iKeyLen;
				free(pElemKey);
				pElemKey = NULL;
				return pRetData;
			}
			free(pElemKey);
			pElemKey = NULL;
	} while (NULL != (pElem = pElem->pNext));
	return NULL;
}

int RelechMap_Earse(RelechMap* pMap, char* pKey)
{
	if(pMap->iMapSize <= 0)
	{
		return -1;
	}
	int iKeyLen = -1;
	if ((NULL == pKey)||((iKeyLen = strlen(pKey)) <= 0))
	{
		return -1;
	}
	int iPos = RelechMap_BkdrHash(pMap, (char*)pKey)%pMap->iMapSize;
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		return -1;
	}
	else
	{
		RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
		RelechMapElem* pNextElem = pElem;
		do 
		{
			pElem = pNextElem;
			int iElemKeyLen = 0;
			char* pData = (char*)pElem->pData;
			memcpy((char*)&iElemKeyLen, pData, iIntLen);
			pData += iIntLen * 2;
			char* pElemKey = (char*)malloc(iElemKeyLen); 
			memcpy(pElemKey, pData, iElemKeyLen);
			if ((iElemKeyLen == iKeyLen) &&(!memcmp(pElemKey, (char*)pKey, iKeyLen)))
			{
				pMap->iItemLen--;
				//printf("erase:key:%s,index:%d\n", pKey, iPos);
				if ((NULL == pElem->pPrev)&&(NULL == pElem->pNext))
				{
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
					pMap->pMap[iPos] = NULL;
				}
				else if ((NULL == pElem->pPrev)&&(NULL != pElem->pNext))
				{
					pMap->pMap[iPos] = (char*)pElem->pNext;
					pElem->pNext->pPrev = NULL;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				else if ((NULL != pElem->pPrev)&&(NULL == pElem->pNext))
				{
					//RelechMapElem* pTmpElem = pElem->pPrev;
					pElem->pPrev->pNext = NULL;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				else if ((NULL != pElem->pPrev)&&(NULL != pElem->pNext))
				{
					pElem->pPrev->pNext = pElem->pNext;
					pElem->pNext->pPrev = pElem->pPrev;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				free(pElemKey);
				pElemKey = NULL;
				return 1;
			}	
			free(pElemKey);
			pElemKey = NULL;
		} while ((NULL != pElem)&&(NULL != (pNextElem = pElem->pNext)));
	}
		return -1;
}

RelechMapVector RelechMap_GetHead(RelechMap* pMap)
{
	RelechMapVector vc; 
	vc.pKey = NULL;
	vc.pValue = NULL;
	vc.iKeyLen = 0;
	vc.iValueLen = 0;
	vc.iIndex = 0;
	vc.pNext = NULL;
	if (pMap->pMap == NULL)
	{
		return vc;
	}
	int iIndex = 0;
	int iFound = -1;
	for (; iIndex < pMap->iMapSize; ++iIndex)
	{
		if (NULL != pMap->pMap[iIndex])
		{
			iFound = 0;
			break;
		}
	}
	if(iFound == -1)
	{
		return vc;
	}
	RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iIndex];
	if (pElem == NULL)
	{
		return vc;
	}
	char* pData = (char*)pElem->pData;
	int iIntLen = sizeof(int);
	memcpy((char*)&vc.iKeyLen, pData, iIntLen);
	pData += iIntLen;
	memcpy((char*)&vc.iValueLen, pData, iIntLen);
	vc.pKey = pData + iIntLen;
	vc.pValue = ((char*)vc.pKey) + vc.iKeyLen;
	vc.pNext = pElem->pNext;
	vc.iIndex = iIndex;
	return vc;
}

int RelechMap_GetNext(RelechMap* pMap, RelechMapVector* pVector)
{
	if (NULL == pVector->pNext)
	{
		while(1)
		{
			if (++pVector->iIndex >= pMap->iMapSize)
			{
				return -1;
			}

			if (NULL != pMap->pMap[pVector->iIndex])
			{
				RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[pVector->iIndex];
				char* pData = (char*)pElem->pData;
				int iIntLen = sizeof(int);
				memcpy((char*)&pVector->iKeyLen, pData, iIntLen);
				pData += iIntLen;
				memcpy((char*)&pVector->iValueLen, pData, iIntLen);
				pVector->pKey = pData+iIntLen;
				pVector->pValue = ((char*)pVector->pKey) + pVector->iKeyLen;
				pVector->pNext = pElem->pNext;
				return 1;
			}
		}
		
	}
	else
	{
		RelechMapElem* pElem = (RelechMapElem*)pVector->pNext;
		char* pData = (char*)pElem->pData;
		int iIntLen = sizeof(int);
		memcpy((char*)&pVector->iKeyLen, pData, iIntLen);
		pData += iIntLen;
		memcpy((char*)&pVector->iValueLen, pData, iIntLen);
		pVector->pKey = pData+iIntLen;
		pVector->pValue = ((char*)pVector->pKey) + pVector->iKeyLen;
		pVector->pNext = pElem->pNext;
		return 1;
	}
	return -1;
}
////////////////////////////////////////int//////////////////////////////////////////////
int RelechMap_SetAtInt(RelechMap* pMap, int iKey, void* pValue, int iValueLen)
{
	if(pMap->iMapSize <= 0)
	{
		RelechMap_Init(DEFAULT_MAPSIZE);
	}
	int iKeyLen = sizeof(int);
	int iPos = RelechMap_BkdrHashInt(pMap, iKey)%pMap->iMapSize;
	//printf("key:%d,index:%d\n", iKey, iPos);
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		////////////////////////////////////////////////////
		//    4�ֽ�      4�ֽ�        int*       char*   //
		//int=>keylen//int=>datalen//  key   //   data   //
		//////////////////////////////////////////////////
		int iTotalDataLen = iValueLen + iIntLen * 2 + iKeyLen;
		char* pData = (char*)malloc(iTotalDataLen);
		memset(pData, 0, iTotalDataLen);
		RelechMapElem* pElem = (RelechMapElem*)malloc(sizeof(RelechMapElem));
		pMap->pMap[iPos] = (char*)pElem;
		pElem->pPrev = NULL;
		pElem->pNext = NULL;
		pElem->pData = pData;
		memcpy(pData, &iKeyLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iValueLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iKey, iKeyLen);
		pData += iKeyLen;
		memcpy(pData, pValue, iValueLen);
		pMap->iItemLen++;
	}
	else
	{
		RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
		RelechMapElem* pNextElem = pElem;
		int iFound = -1;
		do 
		{
			pElem = pNextElem;
			if (-1 == iFound)
			{
				int iElemKeyLen = 0;
				char* pData = (char*)pElem->pData;
				memcpy((char*)&iElemKeyLen, pData, iIntLen);
				pData += iIntLen * 2;
				int iElemKey = 0; 
				memcpy(&iElemKey, pData, iElemKeyLen);
				if (iElemKey == iKey)
				{
					iFound = 1;
					//�ҵ���,ֻ��һ��
					if ((NULL == pElem->pPrev)&&(NULL == pElem->pNext))
					{
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pMap->pMap[iPos] = NULL;
					}
					else if ((NULL == pElem->pPrev)&&(NULL != pElem->pNext))
					{
						pMap->pMap[iPos] = (char*)pElem->pNext;
						pElem->pNext->pPrev = NULL;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = (RelechMapElem*)pMap->pMap[iPos];
						pElem->pPrev = NULL;
					}
					else if ((NULL != pElem->pPrev)&&(NULL == pElem->pNext))
					{
						RelechMapElem* pTmpElem = pElem->pPrev;
						pElem->pPrev->pNext = NULL;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = pTmpElem;
					}
					else if ((NULL != pElem->pPrev)&&(NULL != pElem->pNext))
					{
						pElem->pPrev->pNext = pElem->pNext;
						pElem->pNext->pPrev = pElem->pPrev;
						RelechMapElem* pTmpElem = pElem->pNext;
						free(pElem->pData);
						pElem->pData = NULL;
						free(pElem);
						pElem = NULL;
						pElem = pTmpElem;
					}
				}
				
			}	
		} while ((NULL != pElem)&&(NULL != (pNextElem = pElem->pNext)));

		int iTotalDataLen = iValueLen + iIntLen * 2 + iKeyLen;
		char* pData = (char*)malloc(iTotalDataLen);
		memset(pData, 0, iTotalDataLen);
		RelechMapElem* pNewElem = (RelechMapElem*)malloc(sizeof(RelechMapElem));
		if (NULL == pElem)
		{
			pMap->pMap[iPos] = (char*)pNewElem;
			pNewElem->pPrev = NULL;
			pNewElem->pNext = NULL;
		}
		else
		{
			pNewElem->pPrev = pElem;
			pElem->pNext = pNewElem;
		}
		pNewElem->pNext = NULL;
		pNewElem->pData = pData;
		memcpy(pData, &iKeyLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iValueLen, iIntLen);
		pData += iIntLen;
		memcpy(pData, &iKey, iKeyLen);
		pData += iKeyLen;
		memcpy(pData, pValue, iValueLen);
		if (-1 == iFound)
		{
			pMap->iItemLen++;
		}
	}
	return 1;
}

void* RelechMap_LookUpInt(RelechMap* pMap, int iKey)
{
	if(pMap->iMapSize <= 0)
	{
		pMap = RelechMap_Init(DEFAULT_MAPSIZE);
		return NULL;
	}
	
	int iPos = RelechMap_BkdrHashInt(pMap, iKey)%pMap->iMapSize;
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		return NULL;
	}
	RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
	do 
	{
			char* pData = (char*)pElem->pData;
			pData += iIntLen * 2;
			int iElemKey = 0; 
			memcpy(&iElemKey, pData, iIntLen);
			if (iKey == iElemKey)
			{
				//printf("lookup:key:%d,index:%d\n", iKey, iPos);
				char* pRetData = pData + iIntLen;
				
				return pRetData;
			}
			
	} while (NULL != (pElem = pElem->pNext));
	return NULL;
}

int RelechMap_EarseInt(RelechMap* pMap, int iKey)
{
	if(pMap->iMapSize <= 0)
	{
		return -1;
	}
	//int iKeyLen = -1;
	
	int iPos = RelechMap_BkdrHashInt(pMap, iKey)%pMap->iMapSize;
	int iIntLen = sizeof(int);
	if (NULL == pMap->pMap[iPos])
	{
		printf("RelechMap_EarseInt not find:%d\n", iPos);
		return -1;
	}
	else
	{
		RelechMapElem* pElem = (RelechMapElem*)pMap->pMap[iPos];
		RelechMapElem* pNextElem = pElem;
		do 
		{
			pElem = pNextElem;	
			char* pData = (char*)pElem->pData;
			pData += iIntLen * 2;
			int iElemKey = 0; 
			memcpy(&iElemKey, pData, iIntLen);
			if (iKey == iElemKey)
			{
				pMap->iItemLen--;
				//printf("erase:key:%d,index:%d\n", iKey, iPos);
				if ((NULL == pElem->pPrev)&&(NULL == pElem->pNext))
				{
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
					pMap->pMap[iPos] = NULL;
				}
				else if ((NULL == pElem->pPrev)&&(NULL != pElem->pNext))
				{
					pMap->pMap[iPos] = (char*)pElem->pNext;
					pElem->pNext->pPrev = NULL;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				else if ((NULL != pElem->pPrev)&&(NULL == pElem->pNext))
				{
					//RelechMapElem* pTmpElem = pElem->pPrev;
					pElem->pPrev->pNext = NULL;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				else if ((NULL != pElem->pPrev)&&(NULL != pElem->pNext))
				{
					pElem->pPrev->pNext = pElem->pNext;
					pElem->pNext->pPrev = pElem->pPrev;
					free(pElem->pData);
					pElem->pData = NULL;
					free(pElem);
					pElem = NULL;
				}
				
				return 1;
			}	
			
		} while ((NULL != pElem)&&(NULL != (pNextElem = pElem->pNext)));
	}
		return -1;
}