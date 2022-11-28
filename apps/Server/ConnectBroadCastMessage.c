#include "ConnectBroadCastMessage.h"
#include "Tools.h"
//#include "Tools.h"

void ConnectBroadCast_OnGetIdentifyIp(struct ConnectBroadCast* pConnectBroadCast, char* pBuffer, struct sockaddr_in* pSocket)
{
	json_object* pJsonRoot = json_tokener_parse(pBuffer);
	const char* pszTag = json_object_get_string(json_object_object_get(pJsonRoot, "tag"));
	//const char* pszTimeTag = json_object_get_string(json_object_object_get(pJsonRoot, "time"));
	
	if (0 == strcmp(pszTag, CASTTAG))
	{
		int iIdentifyLen = strlen(pszTag);
		int iSendLen = sizeof(InnerBase) + iIdentifyLen;
		char* pSendBuffer = (char*)malloc(iSendLen);
		memset(pSendBuffer, 0, iSendLen);
		InnerBase* pInner = (InnerBase*)pSendBuffer;
		pInner->iMsgType = CASTMSGTYPE_GETIDENTIFYIP_ACK;
		pInner->iMsgLen = iIdentifyLen;
		memcpy(pInner->szBuf, pszTag, iIdentifyLen);
		ConnectBroadCast_SendMessage(pConnectBroadCast, pSendBuffer, iSendLen, pSocket);

		free(pSendBuffer);
		pSendBuffer = NULL;
	}
	json_object_put(pJsonRoot);
}

void ConnectBroadCast_OnGetIdentifyIpAck(struct ConnectBroadCast* pConnectBroadCast, char* pBuffer, struct sockaddr_in* pSocket)
{
    if (0 == strcmp(pBuffer, CASTTAG))
    {
        printf("接收到地址: %s\n", inet_ntoa(pSocket->sin_addr));
    }
}


void OnConnectBroadCastMessage(struct ConnectBroadCast* pConnectBroadCast, BaseMsg* pMsg, SOCKET iSocket)
{
	InnerBase* pBase = (InnerBase*)pMsg->pszBuf;
	switch (pBase->iMsgType)
	{
		case CASTMSGTYPE_GETIDENTIFYIP:
		{
			ConnectBroadCast_OnGetIdentifyIp(pConnectBroadCast, pBase->szBuf, (struct sockaddr_in*)pMsg->pExtra);
			ConnectBroadCast_Close(iSocket);
			break;
		}
        case CASTMSGTYPE_GETIDENTIFYIP_ACK:
        {
            ConnectBroadCast_OnGetIdentifyIpAck(pConnectBroadCast, pBase->szBuf, (struct sockaddr_in*)pMsg->pExtra);
            break;
        }
		default:
		{
			break;
		}
	}

	//free(pMsg->pExtra);
	//pMsg->pExtra = NULL;
}


