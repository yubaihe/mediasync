#pragma once
#include <signal.h>
#include <map>
#include <list>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <regex>
#include <algorithm>
#include <map>
#include <cassert>
#include <atomic>
#include <stdexcept>
#include <cstring>
#include <limits>
#include "Util/CriticalSectionManager.h"
#include "Util/CommonSdkUtil.h"
#include "Dbus/libdbus.h"
#include "Database/DbDriver.h"
#ifndef BOOL
#define BOOL int
#define TRUE 1
#define FALSE 0

#endif
#ifdef __x86_64__
typedef uint64_t SOCKET;
#else
typedef uint32_t SOCKET;
#endif
#define LOGD(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_DEBUG, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);
#define LOGI(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_INFO, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);
#define LOGE(pszFormat, ...)  CCommonSdkUtil::Log(LOGLEVEL_ERROR, __func__, __FILE__, __LINE__, pszFormat, ##__VA_ARGS__);