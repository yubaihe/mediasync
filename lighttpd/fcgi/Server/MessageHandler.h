#include "./stdafx.h"
class CMessageHandler
{
public:
    CMessageHandler();
    ~CMessageHandler();
    void OnMessage(const char* pMsg, char* pRet);
private:
};