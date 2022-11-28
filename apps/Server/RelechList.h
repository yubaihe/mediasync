#ifndef RELECH_LIST_H_
#define RELECH_LIST_H_
#include "stdafx.h"
typedef struct RelechListElem
{
	struct RelechListElem* Next;
	struct RelechListElem* prev;
	void* pData;
}RelechListElem;


typedef struct RelechList
{
	int iItemLen;
	RelechListElem* pListHead;
	RelechListElem* pListLast;

}RelechList;

RelechList* RelechList_Init();
void RelechList_Release(RelechList* pList);
int RelechList_PushBack(RelechList* pList, void* pData);
void* RelechList_GetFromIndex(RelechList* pList, int iIndex);


int RelechList_RemoveIndex(RelechList* pList, int iIndex);
void RelechList_RemoveElement(RelechList* pList, RelechListElem* pListElem);
int RelechList_PushBack(RelechList* pList, void* pData);
int RelechList_GetSize(RelechList* pList);
RelechListElem* RelechList_First(RelechList* pList);
RelechListElem* RelechList_Next(RelechListElem* pListElem);









#endif
