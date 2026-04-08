#include "stdafx.h"
pthread_t LinuxCreateThread(pthread_attr_t* lpThreadAttributes, LPTHREAD_START_ROUTINE lpStartAddress, void* lpParameter)
{
	pthread_t ntid;
	pthread_create(&ntid,NULL,lpStartAddress, lpParameter);
	return ntid;
}

