#ifndef RELECH_QUEUE_H__
#define RELECH_QUEUE_H__
#include "stdafx.h"
struct BaseQueueItem
{
	void* pData;
	struct BaseQueueItem* pNext;
};

typedef struct RelechQueue
{
	int iItemLen;
	struct BaseQueueItem* pHead;
	struct BaseQueueItem* pTail;
}RelechQueue;

RelechQueue* RelechQueue_Init();
void RelechQueue_PushBack(RelechQueue* pQueue, void* pData);
void* RelechQueue_Front(RelechQueue* pQueue);
void RelechQueue_PopFront(RelechQueue* pQueue);
void RelechQueue_Clear(RelechQueue* pQueue);


#endif
