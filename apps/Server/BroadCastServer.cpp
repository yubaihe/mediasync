#include "BroadCastServer.h"
#include "Util/JsonUtil.h"
CBroadCastServer::CBroadCastServer()
{
}

CBroadCastServer::~CBroadCastServer()
{
}
BOOL CBroadCastServer::Start(int iPort)
{
    return m_ConnectBroadCast.Start(iPort, OnConnectBroadCastMessage, this);
}
void CBroadCastServer::OnGetIdentifyIp(char* pszBuffer, struct sockaddr_in* pSocket)
{
	nlohmann::json jsonValue;
    BOOL bRet = Server::CJsonUtil::FromString(pszBuffer, jsonValue);
    if(FALSE == bRet)
    {
        return;
    }
    string strTag = jsonValue["tag"];
	if (0 == strcmp(strTag.c_str(), CASTTAG))
	{
		int iIdentifyLen = strlen(strTag.c_str());
		int iSendLen = sizeof(InnerBase) + iIdentifyLen;
		char* pszSendBuffer = (char*)malloc(iSendLen);
		memset(pszSendBuffer, 0, iSendLen);
		InnerBase* pInner = (InnerBase*)pszSendBuffer;
		pInner->iMsgType = CASTMSGTYPE_GETIDENTIFYIP_ACK;
		pInner->iMsgLen = iIdentifyLen;
		memcpy(pInner->szBuf, strTag.c_str(), iIdentifyLen);
		m_ConnectBroadCast.SendMessage(pszSendBuffer, iSendLen, pSocket);

		free(pszSendBuffer);
		pszSendBuffer = NULL;
	}
}

void CBroadCastServer::OnGetIdentifyIpAck(char* pszBuffer, struct sockaddr_in* pSocket)
{
    if (0 == strcmp(pszBuffer, CASTTAG))
    {
        printf("接收到地址: %s\n", inet_ntoa(pSocket->sin_addr));
    }
}
void CBroadCastServer::BroadCastNotifyOnLine(int iPort)
{
    m_ConnectBroadCast.BroadCastNotifyOnLine(iPort);
}
void CBroadCastServer::OnConnectBroadCastMessage(BaseMsg* pMsg, SOCKET iSocket, LPVOID param)
{
    CBroadCastServer* pBroadCastServer = (CBroadCastServer*)param;
    InnerBase* pBase = (InnerBase*)pMsg->pszBuf;
	switch (pBase->iMsgType)
	{
		case CASTMSGTYPE_GETIDENTIFYIP:
		{
			pBroadCastServer->OnGetIdentifyIp(pBase->szBuf, (struct sockaddr_in*)pMsg->pExtra);
			shutdown(iSocket, SD_BOTH);
            closesocket(iSocket);
            iSocket = 0;
			break;
		}
        case CASTMSGTYPE_GETIDENTIFYIP_ACK:
        {
            pBroadCastServer->OnGetIdentifyIpAck(pBase->szBuf, (struct sockaddr_in*)pMsg->pExtra);
            break;
        }
		default:
		{
			break;
		}
	}
}