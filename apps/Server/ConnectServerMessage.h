#ifndef _RELECH_CONNECTSERVERMESSAGE_H_
#define _RELECH_CONNECTSERVERMESSAGE_H_
#include "stdafx.h"
#include "ConnectServer.h"
#include "DataBaseDriver.h"
#include "DataBaseMedia.h"
#include "DataBaseDevice.h"
#include "DataBaseUserInfo.h"
void OnComputerServerMessage(struct ConnectServer* pServer, BaseMsg* pMsg, SOCKET iSocket);
void ComputerServerContectedConn(struct ConnectServer* pServer, SOCKET iSocket);
void ComputerServerUnContectedConn(struct ConnectServer* pServer, SOCKET iSocket);



#endif
