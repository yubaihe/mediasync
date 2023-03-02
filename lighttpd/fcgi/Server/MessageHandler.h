#ifndef MESSAGEHANDLE_H__
#define MESSAGEHANDLE_H__
#include "stdafx.h"
#include "RelechMap.h"
#define MAX_ACTION_LEN 50
typedef char* (*OnActionHandler)(json_object* pJsonRoot);

typedef struct 
{
    RelechMap* pHanlerMap;
}MessageHandler;

typedef struct 
{
    OnActionHandler actionHandler;
} ControllerItem;

// #define ADD_CONTROLLER(action, handler) \
// static ControllerItem  action##_controller_ = {\
// 	action,handler\
// };\
// __attribute((constructor(102))) void action##_init_() \
// {\
// 		MessageHandler_AddController(&action##_controller_);\
// }

MessageHandler* MessageHandler_Init();
void MessageHandler_UnInit(MessageHandler* pMessageHandler);
void OnMessage(RelechMap* pRelechMap, char* pMsg, char** pRet);
#endif