#include "RelechList.h"
RelechList* RelechList_Init()
{
	int iLen = sizeof(RelechList);
	char* pBuffer = (char*)malloc(iLen);
	memset(pBuffer, 0, iLen);
	RelechList* pList = (RelechList*)pBuffer;
	pList->iItemLen = 0;
	pList->pListHead = NULL;
	pList->pListLast = NULL;
	return pList;
}

int RelechList_GetSize(RelechList* pList)
{
	return pList->iItemLen;
}

int RelechList_PushBack(RelechList* pList, void* pData)
{
	if (pData == NULL)
	{
		return -1;
	}
	
	char* pElem = (char *)malloc(sizeof(RelechListElem));
	memset(pElem, 0, sizeof(RelechListElem));

	RelechListElem* pListElem = (RelechListElem*)pElem;
	pListElem->pData = pData;
	pListElem->Next = NULL;
	if (NULL == pList->pListHead)
	{
		pListElem->prev = NULL;
		pList->pListHead = pListElem;
	}
	else
	{
		pListElem->prev = pList->pListLast;
		pList->pListLast->Next = pListElem;
	}
	pList->pListLast = pListElem;
	pList->iItemLen++;
	return 1;
}

void RelechList_RemoveElement(RelechList* pList, RelechListElem* pListElem)
{
	if(pListElem->prev != NULL && pListElem->Next != NULL)
	{
		//ɾ���м��
		pListElem->prev->Next = pListElem->Next;
		pListElem->Next->prev = pListElem->prev;

		if(NULL != pListElem->pData)
		{
			free((char*)pListElem->pData);
			pListElem->pData = NULL;
		}
		if(NULL != pListElem)
		{
			free((char*)pListElem);
			pListElem = NULL;
		}
		
		pList->iItemLen--;
	}
	else if (pListElem->prev == NULL && pListElem->Next == NULL)
	{
		//ɾ�����һ��
		if(NULL != pListElem->pData)
		{
			free((char*)pListElem->pData);
			pListElem->pData = NULL;
		}
		if(NULL != pListElem)
		{
			free((char*)pListElem);
			pListElem = NULL;
		}
		pList->iItemLen--;

		pList->pListHead = NULL;
		pList->pListLast = NULL;
	}
	else if (pListElem->prev == NULL)
	{
		//ɾ��ͷ
		pList->pListHead = pListElem->Next;
		pListElem->Next->prev = NULL;

		if(NULL != pListElem->pData)
		{
			free((char*)pListElem->pData);
			pListElem->pData = NULL;
		}
		if(NULL != pListElem)
		{
			free((char*)pListElem);
			pListElem = NULL;
		}
		pList->iItemLen--;
	}
	else //if(pListElem->Next == NULL)
	{
		//ɾ��β
		pList->pListLast = pListElem->prev;
		pListElem->prev->Next = NULL;

		if(NULL != pListElem->pData)
		{
			free((char*)pListElem->pData);
			pListElem->pData = NULL;
		}
		if(NULL != pListElem)
		{
			free((char*)pListElem);
			pListElem = NULL;
		}
		pList->iItemLen--;
	}
}

int RelechList_RemoveIndex(RelechList* pList, int iIndex)
{
	if (iIndex >= pList->iItemLen || iIndex < 0 || pList->iItemLen == 0)
	{
		return -1;
	}
	int iCurIndex = 0;
	RelechListElem* pListElem = pList->pListHead;
	
	while(iCurIndex < iIndex)
	{
		if(NULL == pListElem->Next)
		{
			return -1;
		}
		pListElem = pListElem->Next;
		iCurIndex++;
	}

	RelechList_RemoveElement(pList, pListElem);
	return 1;
}

void* RelechList_GetFromIndex(RelechList* pList, int iIndex)
{
	if (iIndex >= pList->iItemLen || iIndex < 0 || pList->iItemLen == 0)
	{
		return NULL;
	}

	int iCurIndex = 0;
	RelechListElem* pQueueElem = pList->pListHead;
	while(iCurIndex < iIndex)
	{
		if(NULL == pQueueElem->Next)
		{
			return NULL;
		}
		pQueueElem = pQueueElem->Next;
		iCurIndex++;
	}

	return pQueueElem->pData;
}


void RelechList_RemoveAll(RelechList* pList)
{
	int iListSize = pList->iItemLen;
	pList->iItemLen = 0;
	while(iListSize > 0)
	{
		RelechListElem* pTmpElem = pList->pListHead;
		RelechList_RemoveElement(pList, pTmpElem);
		iListSize--;
	}
	
	pList->pListHead = NULL;
}

void RelechList_Release(RelechList* pList)
{
	RelechList_RemoveAll(pList);
	pList->iItemLen = 0;
	pList->pListHead = NULL;
	pList->pListLast = NULL;
	free((char*)pList);
	pList = NULL;
}

RelechListElem* RelechList_First(RelechList* pList)
{
	return pList->pListHead;
}
RelechListElem* RelechList_Next(RelechListElem* pListElem)
{
	return pListElem->Next;
}


