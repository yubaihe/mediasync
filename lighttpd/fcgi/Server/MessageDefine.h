// #ifndef MESSAGEDEFINE_H_
// #define MESSAGEDEFINE_H_
// //#include "stdafx.h"
// #ifdef __x86_64__
// typedef uint64_t SOCKET;
// #else
// typedef uint32_t SOCKET;
// #endif

// typedef enum CASTMSGTYPE
// {
// 	CASTMSGTYPE_GETIDENTIFYIP = 0,
// 	CASTMSGTYPE_GETIDENTIFYIP_ACK
	
// }CASTMSGTYPE;

// typedef enum MESSAGETYPE
// {
//     MESSAGETYPE_CMD = 1000
// }MESSAGETYPE;


// #pragma pack (1)
// typedef struct InnerBase
// {
// 	uint16_t iMsgType;//消息类型
// 	uint32_t iMsgLen;//消息长度
// 	char szBuf[1];
// }InnerBase;
// typedef struct BaseMsg
// {
// 	uint8_t iVersion;
// 	uint32_t iDataLen;
// 	SOCKET iSockfd;
//     char* pExtra;
// 	char* pszBuf;
// 	struct BaseMessage* pBaseMessage;
// }BaseMsg;
// struct BaseMessage
// {
// 	uint8_t iVersion;
// 	uint32_t iDataLen;
// 	char szBuf[1];
// };
// #pragma pack ()
// #endif
