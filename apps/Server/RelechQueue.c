#include "RelechQueue.h"
RelechQueue* RelechQueue_Init()
{
	char* pBuffer = (char*)malloc(sizeof(RelechQueue));
	memset(pBuffer, 0, sizeof(RelechQueue));
	RelechQueue* pQueue = (RelechQueue*)pBuffer;
	return pQueue;
}

void RelechQueue_PushBack(RelechQueue* pQueue, void* pData)
{
	if(NULL == pQueue)
	{
		return;
	}

	struct BaseQueueItem* pItem = (struct BaseQueueItem*)malloc(sizeof(struct BaseQueueItem));
	pItem->pData = pData;
	pItem->pNext = NULL;
	if(pQueue->iItemLen == 0)
	{
		pQueue->pHead = pItem;
		pQueue->pTail = pItem;
	}
	else
	{
		pQueue->pTail->pNext = pItem;
		pQueue->pTail = pItem;
	}
	pQueue->iItemLen++;
	
	
}

void* RelechQueue_Front(RelechQueue* pQueue)
{
	if(NULL == pQueue)
	{
		return NULL;
	}
	if(pQueue->iItemLen == 0)
	{
		return NULL;
	}
	return pQueue->pHead->pData;
}

void RelechQueue_PopFront(RelechQueue* pQueue)
{
	if(NULL == pQueue)
	{
		return ;
	}
	if(pQueue->iItemLen == 0)
	{
		return ;
	}
	pQueue->iItemLen--;
	struct BaseQueueItem* pItem = pQueue->pHead;
	pQueue->pHead = pQueue->pHead->pNext;
	free((char*)pItem);
	pItem = NULL;
}


void RelechQueue_Clear(RelechQueue* pQueue)
{
	if(NULL == pQueue)
	{
		return ;
	}
	while(pQueue->iItemLen > 0)
	{
		RelechQueue_PopFront(pQueue);
	}

	free((char*)pQueue);
	pQueue = NULL;
}


