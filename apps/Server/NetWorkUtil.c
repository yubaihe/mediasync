#include "NetWorkUtil.h"
#include "Tools.h"
BOOL NetWorkUtil_WifiScan()
{
    char* pszRet = Tools_ExecuteCmd("wpa_cli -i wlan1 scan");
    BOOL bRet = strcmp(pszRet, "OK\n") == 0 ? TRUE:FALSE;
    free(pszRet);
    pszRet = NULL;
    return bRet;
}

struct json_object* NetWorkUtil_GetItem(char* pszWifiItem)
{
    if(NULL == pszWifiItem)
    {
        return NULL;
    }
    char* pSrcBufferTmp = pszWifiItem;
	char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    int iIndex = 0;
    WifiItem item;
    memset(item.szSsid, 0, MAX_SSID_LEN);
    memset(item.szBssid, 0, MAX_BSSID_LEN);
    item.iSignal = 0;
    while((pTmp = strtok_r(pTmp, "\t", &pOut)) != NULL)
    {
        
        switch (iIndex)
        {
            case 0:
            {
                if(strlen(pTmp) > MAX_SSID_LEN)
                {
                    return NULL;
                }
                //BSSID
                //printf("BSSID=>%s", pTmp);
                strcpy(item.szBssid, pTmp);
                break;
            }
            case 2:
            {
                //SIGNAL
                //printf(" SIGNAL=>%s", pTmp);
                item.iSignal = atoi(pTmp);
                break;
            }
            case 3:
            {
                //SSID
                //printf(" SSID=>%s", pTmp);
                strcpy(item.szCrypt, pTmp);
                break;
            }
            case 4:
            {
                //SSID
                
                char* pszSSid = NetWorkUtil_DecodeUtf8(pTmp);
                strcpy(item.szSsid, pszSSid);
                //printf(" %s=>%s\n", pTmp, item.szSsid);
                free(pszSSid);
                pszSSid = NULL;
                break;
            }
            
        }
        iIndex++;
		pTmp = NULL;
    }



    //char* pRet = (char*)malloc(200);
    //memset(pRet, 0, 200);
    //sprintf(pRet, "{\"bssid\":\"%s\",\"ssid\":\"%s\",\"signal\":%d,\"crypt\":\"%s\"}", item.szBssid, item.szSsid, item.iSignal, item.szCrypt);
    struct json_object* pJsonItem = json_object_new_object();
    json_object_object_add(pJsonItem, "bssid", json_object_new_string(item.szBssid)); 
    json_object_object_add(pJsonItem, "ssid", json_object_new_string(item.szSsid)); 
    json_object_object_add(pJsonItem, "signal", json_object_new_int(item.iSignal)); 
    json_object_object_add(pJsonItem, "crypt", json_object_new_string(item.szCrypt)); 
    return pJsonItem;
}

struct json_object* NetWorkUtil_ParseWifiItems(char* pszWifiItems)
{
    char* pSrcBufferTmp = pszWifiItems;
	char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    struct json_object* pJsonItems = json_object_new_array();
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        struct json_object* pJsonItem = NetWorkUtil_GetItem(pTmp);
        if(NULL != pJsonItem)
        {
            json_object_array_add(pJsonItems, pJsonItem); 
        }
		pTmp = NULL;
    }
   
    return pJsonItems;
}

struct json_object* NetWorkUtil_GetWifiScanItems()
{
    char* pszRet = Tools_ExecuteCmd("wpa_cli -i wlan1 scan_result");
    struct json_object* pItems = NetWorkUtil_ParseWifiItems(pszRet);
    free(pszRet);
    pszRet = NULL;
    return pItems;
}

BOOL NetWorkUtil_GetWifiName(char* pszWifiName, char* pszWifiPwd)
{
    char* pSrcBufferTmp = Tools_ExecuteCmd("cat /etc/hostapd.conf");
    if(NULL == pSrcBufferTmp)
    {
        return FALSE;
    }
    char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        if(0 == memcmp(pTmp, "ssid=", strlen("ssid=")))
        {
            strcpy(pszWifiName, pTmp + strlen("ssid="));
        }
        if(0 == memcmp(pTmp, "wpa_passphrase=", strlen("wpa_passphrase=")))
        {
            strcpy(pszWifiPwd, pTmp + strlen("wpa_passphrase="));
        }
		pTmp = NULL;
    }
    free(pSrcBufferTmp);
    pSrcBufferTmp = NULL;
    return TRUE;
}
int NetWorkUtil_GetWlan1UdhcpcPid()
{
    char* pszBuffer = Tools_ExecuteCmd("ps -ef | grep \"udhcpc -i wlan1\" | grep -v grep | awk '{print $1}'");
    if(NULL == pszBuffer)
    {
        return 0;
    }
    if(strlen(pszBuffer) == 0)
    {
        free(pszBuffer);
        pszBuffer = NULL;
        return 0;
    }
    int iRet = atoi(pszBuffer);
    free(pszBuffer);
    pszBuffer = NULL;
    return iRet;
}
void NetWorkUtil_RestartWlan1Udhcpc()
{
    int iUdhcpcID = NetWorkUtil_GetWlan1UdhcpcPid();
    if(iUdhcpcID > 0)
    {
        char szCmd[100] = {0};
        sprintf(szCmd, "kill -9 %d", iUdhcpcID);
        Tools_ExecuteCmd2(szCmd);
    }
    Tools_ExecuteCmd2("udhcpc -i wlan1 -n -b");
}
void NetWorkUtil_RestartHostApd()
{
    Tools_ExecuteCmd2("killall hostapd;hostapd -d /etc/hostapd.conf -B;");
    int iIndex = 0;
    do
    {
        if(iIndex++ == 3)
        {
            break;
        }
        usleep(500*iIndex);
        char* pszRet = Tools_ExecuteCmd("ps -ef | grep \"hostapd\" | grep -v grep | awk '{print $1}'");
        if(0 == strlen(pszRet))
        {
            free(pszRet);
            pszRet = NULL;
            usleep(1000);
            Tools_ExecuteCmd2("hostapd -d /etc/hostapd.conf -B;");
        }
        else
        {
            free(pszRet);
            pszRet = NULL;
            break;
        }
    } while (TRUE);
}

void NetWorkUtil_SetHotPot(const char* pszSsid, const char* pszPasswd)
{
    char szText[300] = {0};
    sprintf(szText, "ctrl_interface=/var/run/hostapd\n" \
                    "driver=rtl871xdrv\n" \
                    "hw_mode=g\n" \
                    "channel=6\n" \
                    "interface=wlan0\n" \
                    "wpa=3\n" \
                    "wpa_pairwise=CCMP TKIP\n" \
                    "rsn_pairwise=CCMP\n" \
                    "wpa_key_mgmt=WPA-PSK\n" \
                    "ssid=%s\n" \
                    "wpa_passphrase=%s\n", pszSsid, pszPasswd);
    FILE* pFile = fopen("/etc/hostapd.conf", "w");
    fwrite(szText, sizeof(char), strlen(szText), pFile);
    fflush(pFile);
    fclose(pFile);
    NetWorkUtil_RestartHostApd();
    // char szCmd[100] = {0};
    // sprintf(szCmd, "hostapd_cli -i wlan0 set ssid %s", pszSsid);
    // char* pszRet = Tools_ExecuteCmd(szCmd);
    // if(0 != strcmp(pszRet, "OK\n"))
    // {
    //     free(pszRet);
    //     pszRet = NULL;  
    //     return FALSE;
    // }
    // free(pszRet);
    // pszRet = NULL;  
    // memset(szCmd, 0, 100);
    // sprintf(szCmd, "hostapd_cli -i wlan0 set wpa_passphrase %s", pszPasswd);
    // pszRet = Tools_ExecuteCmd(szCmd);
    // if(0 != strcmp(pszRet, "OK\n"))
    // {
    //     free(pszRet);
    //     pszRet = NULL;  
    //     return FALSE;
    // }
    // free(pszRet);
    // pszRet = NULL;  

    // memset(szCmd, 0, 100);
    // pszRet = Tools_ExecuteCmd("hostapd_cli -i wlan0 reload");
    // if(0 != strcmp(pszRet, "OK\n"))
    // {
    //     free(pszRet);
    //     pszRet = NULL;  
    //     return FALSE;
    // }
    // free(pszRet);
    // pszRet = NULL;  

    
    
    
}

BRTYPE NetWorkUtil_GetBrtype()
{
    char* pszBrInfos = Tools_ExecuteCmd("brctl show");
    char* pSrcBufferTmp = pszBrInfos;
	char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        int iPosition = Tools_Rfind(pTmp, '\t');
        if(0 == strcmp(pTmp + iPosition + 1, "wlan1"))
        {
            free(pszBrInfos);
            pszBrInfos = NULL;
            return BRTYPE_WLAN1;
        }
        else if(0 == strcmp(pTmp + iPosition + 1, "eth0"))
        {
            free(pszBrInfos);
            pszBrInfos = NULL;
            return BRTYPE_ETH0;
        }
		pTmp = NULL;
    }
    free(pszBrInfos);
    pszBrInfos = NULL;
    return BRTYPE_NONE;
}

void NetWorkUtil_SetBrtype(BRTYPE BrType)
{
    BRTYPE oldBrType = NetWorkUtil_GetBrtype();
    if(oldBrType == BrType)
    {
        return ;
    }

    char* pRet = NULL;
    switch (BrType)
    {
        case BRTYPE_ETH0:
        {
            if(oldBrType == BRTYPE_NONE)
            {
                pRet = Tools_ExecuteCmd("brctl addif br0 wlan0;brctl addif br0 eth0;");
            }
            else if(oldBrType == BRTYPE_WLAN1)
            {
                pRet = Tools_ExecuteCmd("brctl delif br0 wlan1;brctl addif br0 eth0;");
            }
            break;
        }
        case BRTYPE_WLAN1:
        {
            if(oldBrType == BRTYPE_NONE)
            {
                pRet = Tools_ExecuteCmd("brctl addif br0 wlan0;brctl addif br0 wlan1;");
            }
            else if(oldBrType == BRTYPE_ETH0)
            {
                pRet = Tools_ExecuteCmd("brctl delif br0 eth0;brctl addif br0 wlan1;");
            }
            break;
        }
        default:
        {
            if(oldBrType == BRTYPE_ETH0)
            {
                pRet = Tools_ExecuteCmd("brctl delif br0 eth0;brctl delif br0 wlan0;");
            }
            else if(oldBrType == BRTYPE_WLAN1)
            {
                pRet = Tools_ExecuteCmd("brctl delif br0 wlan1;brctl delif br0 wlan0;");
            }
            
            break;
        }
    }

    free(pRet);
    pRet = NULL;
    NetWorkUtil_RestartHostApd();
}

int NetWorkUtil_GetWlan1Status(char* pszSsid, char* pszIpAddr, int* iNetWorkID)
{
    char* pSrcBufferTmp = Tools_ExecuteCmd("wpa_cli -i wlan1 status");
    if(NULL == pSrcBufferTmp)
    {
        return WPASTATUS_INACTIVE;
    }
    int iRetStatus = WPASTATUS_INACTIVE;
    char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    *iNetWorkID = -1;
    memset(pszIpAddr, 0, MAX_IPADDR_LEN);
    memset(pszSsid, 0, MAX_SSID_LEN);
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        if(0 == memcmp(pTmp, "ssid=", strlen("ssid=")))
        {
            char* pszSSid = NetWorkUtil_DecodeUtf8(pTmp + strlen("ssid="));
            strcpy(pszSsid, pszSSid);
            free(pszSSid);
            pszSSid = NULL;
            //strcpy(pszSsid, pTmp + strlen("ssid="));
        }
        else if(0 == memcmp(pTmp, "id=", strlen("id=")))
        {
            *iNetWorkID = atoi(pTmp + strlen("id="));
        }
        else if(0 == memcmp(pTmp, "wpa_state=", strlen("wpa_state=")))
        {
            int iLen = strlen("wpa_state=");
            if(0 == strcmp(pTmp + iLen, "INACTIVE"))
            {
                iRetStatus = WPASTATUS_INACTIVE;
            }
            else if(0 == strcmp(pTmp + iLen, "SCANNING"))
            {
                iRetStatus = WPASTATUS_SCANNING;
            }
            else if(0 == strcmp(pTmp + iLen, "ASSOCIATING"))
            {
                iRetStatus = WPASTATUS_ASSOCIATING;
            }
            else if(0 == strcmp(pTmp + iLen, "ASSOCIATED"))
            {
                iRetStatus = WPASTATUS_ASSOCIATED;
            }
            else if(0 == strcmp(pTmp + iLen, "COMPLETED"))
            {
                iRetStatus = WPASTATUS_COMPLETED;
            }
            else if(0 == strcmp(pTmp + iLen, "DISCONNECTED"))
            {
                iRetStatus = WPASTATUS_DISCONNECTED;
            }
            else if(0 == strcmp(pTmp + iLen, "4WAY_HANDSHAKE"))
            {
                iRetStatus = WPASTATUS_4WAY_HANDSHAKE;
            }
            else if(0 == strcmp(pTmp + iLen, "INTERFACE_DISABLED"))
            {
                //wifi口断开了
                iRetStatus = WPASTATUS_INACTIVE;
            }
            else 
            {
                printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~unknow wifi status:%s\n", pTmp + iLen);
            }
        }
        else if(0 == memcmp(pTmp, "ip_address=", strlen("ip_address=")))
        {
           strcpy(pszIpAddr, pTmp + strlen("ip_address="));
        }
		pTmp = NULL;
    }
    free(pSrcBufferTmp);
    pSrcBufferTmp = NULL;
    return iRetStatus;
}

struct json_object* NetWorkUtil_GetWlan1StoreItem(char* pszWifiStoreItem, char* pszSsid, char* pszIpAddr, int iNetworkID, WPASTATUS eStatus)
{
    if(NULL == pszWifiStoreItem)
    {
        return NULL;
    }
    char* pSrcBufferTmp = pszWifiStoreItem;
	char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    int iIndex = 0;
    WifiStoreItem item;
    memset(item.szSsid, 0, MAX_SSID_LEN);
    memset(item.szBssid, 0, MAX_BSSID_LEN);
    memset(item.szIpAddr, 0, MAX_IPADDR_LEN);
    item.iFlags = 0;
    item.iNetworkID = 0;
    while((pTmp = strtok_r(pTmp, "\t", &pOut)) != NULL)
    {
        switch (iIndex)
        {
            case 0:
            {
                if(memcmp(pTmp, "network id", strlen("network id")) == 0)
                {
                    return NULL;
                }
                //Network ID
                item.iNetworkID = atoi(pTmp);
                if(iNetworkID == item.iNetworkID)
                {
                    strcpy(item.szIpAddr, pszIpAddr);
                    item.iFlags = eStatus;
                }
                break;
            }
            case 1:
            {
                //ssid
                char* pszSSid = NetWorkUtil_DecodeUtf8(pTmp);
                strcpy(item.szSsid, pszSSid);
                free(pszSSid);
                pszSSid = NULL;
                //strcpy(item.szSsid, pTmp);
                break;
            }
            case 2:
            {
                //bssid
                strcpy(item.szBssid, pTmp);
                break;
            }
            case 3:
            {
                //[DISABLED] [CURRENT]
                // if(memcmp(pTmp, "[CURRENT]", strlen("[CURRENT]")) == 0)
                // {
                //     item.iFlags = 1;
                // }
                break;
            }
            
        }
        iIndex++;
		pTmp = NULL;
    }
    // char* pRet = (char*)malloc(100);
    // memset(pRet, 0, 100);
    // sprintf(pRet, "{\"bssid\":\"%s\",\"ssid\":\"%s\",\"id\":\"%d\",\"flag\":%d,\"ipaddr\":\"%s\"}", item.szBssid, item.szSsid, item.iNetworkID, item.iFlags, item.szIpAddr);
    //printf("%s\n", item.szSsid);
    struct json_object* pJsonItem = json_object_new_object();
    json_object_object_add(pJsonItem, "bssid", json_object_new_string(item.szBssid)); 
    json_object_object_add(pJsonItem, "ssid", json_object_new_string(item.szSsid)); 
    json_object_object_add(pJsonItem, "id", json_object_new_int(item.iNetworkID)); 
    json_object_object_add(pJsonItem, "flag", json_object_new_int(item.iFlags)); 
    json_object_object_add(pJsonItem, "ipaddr", json_object_new_string(item.szIpAddr));
    return pJsonItem;
}

struct json_object* NetWorkUtil_GetWlan1StoreItems()
{
    char szSsid[MAX_SSID_LEN] = {0};
    char szIpAddr[MAX_IPADDR_LEN] = {0};
    int iNetworkID;
    WPASTATUS eStatus = NetWorkUtil_GetWlan1Status(szSsid, szIpAddr, &iNetworkID);

    char* pSrcBufferTmp = Tools_ExecuteCmd("wpa_cli -i wlan1 list_network");
    //printf("%s\n", pSrcBufferTmp);
    if(NULL == pSrcBufferTmp)
    {
        return NULL;
    }

    char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    struct json_object* pAllStoreItems = json_object_new_array();
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
       struct json_object* pJsonItem = NetWorkUtil_GetWlan1StoreItem(pTmp, szSsid, szIpAddr, iNetworkID, eStatus);
        if(NULL != pJsonItem)
        {
            json_object_array_add(pAllStoreItems, pJsonItem); 
        }
		pTmp = NULL;
    }
    free(pSrcBufferTmp);
    pSrcBufferTmp = NULL;
    return pAllStoreItems;
}

char* NetWorkUtil_GetWifiItemFromNetWorkId(int iNetWorkId)
{
    char szCmd[100] = {0};
    sprintf(szCmd, "wpa_cli get_network %d ssid -i wlan1", iNetWorkId);
    char* pszRet = Tools_ExecuteCmd(szCmd);
    if(0 == strcmp(pszRet, "FAIL\n"))
    {
        free(pszRet);
        pszRet = NULL;
        return NULL;
    }
    if(pszRet[0] == '"')
    {
        //字符串
        int iLen = strlen(pszRet);
        char* pszSsid = malloc(iLen - 1);
        memset(pszSsid, 0, iLen - 1);
        memcpy(pszSsid, pszRet + 1, iLen - 2);
        free(pszRet);
        pszRet = NULL;
        return pszSsid;
    }
    else
    {
        //带汉字
        char* pHexString = NetWorkUtil_ToHexString(pszRet);
        free(pszRet);
        pszRet = NULL;
        char* pszSsid = NetWorkUtil_DecodeUtf8(pHexString);
        free(pHexString);
        pHexString = NULL;
        return pszSsid;
    }
    
}

int NetWorkUtil_GetWifiItemFromSsid(const char* pszSsid)
{
    char* pSrcBufferTmp = Tools_ExecuteCmd("wpa_cli -i wlan1 list_network |awk '{print $1}'");
    if(NULL == pSrcBufferTmp)
    {
        return 0;
    }

    char* pTmp = pSrcBufferTmp;
	char* pOut = NULL;
    int iNetWorkID = -1;
    while((pTmp = strtok_r(pTmp, "\n", &pOut)) != NULL)
    {
        if(0 == strcmp(pTmp, "network"))
        {
            pTmp = NULL;
            continue;
        }
       char* pItem = NetWorkUtil_GetWifiItemFromNetWorkId(atoi(pTmp));
        if(NULL != pItem)
        {
            if(0 == strcmp(pItem, pszSsid))
            {
                iNetWorkID = atoi(pTmp);
            }
            else
            {
                //printf("not equal :%s~%s\n", pItem, pszSsid);
            }
            
            free(pItem);
            pItem = NULL;
            if(iNetWorkID >= 0)
            {
                break;
            }
        }
		pTmp = NULL;
    }
    free(pSrcBufferTmp);
    pSrcBufferTmp = NULL;
    
    return iNetWorkID;
}

BOOL NetWorkUtil_SetWlan1Password(int iNetWorkID, const char* pszPasswd)
{
     long long iIdentify = Tools_CurTimeMilSec();
     char szCmd[200] = {0};
     sprintf(szCmd, "wpa_cli -i wlan1 set_network %d psk '\"%lld\"'", iNetWorkID, iIdentify);
     char* pszRet = Tools_ExecuteCmd(szCmd);
     if(0 != strcmp(pszRet, "OK\n"))
     {
        free(pszRet);
        pszRet = NULL;
        return FALSE;
     }
     free(pszRet);
     pszRet = NULL;

     memset(szCmd, 0, 200);
     strcpy(szCmd, "wpa_cli -i wlan1 save_config");
     pszRet = Tools_ExecuteCmd(szCmd);
     if(0 != strcmp(pszRet, "OK\n"))
     {
        free(pszRet);
        pszRet = NULL;
        return FALSE;
     }
     free(pszRet);
     pszRet = NULL;

     char* pszPwd1 = Tools_ReplaceString(pszPasswd, "\"", "\\\"");
     char* pszPwd2 = Tools_ReplaceString(pszPwd1, "\'", "\\\'");
     free(pszPwd1);
     pszPwd1 = NULL;
    
     memset(szCmd, 0, 200);
     sprintf(szCmd, "sed -i \"s|%lld|%s|\" /etc/wpa_supplicant.conf", iIdentify, pszPwd2);
     free(pszPwd2);
     pszPwd2 = NULL;
     pszRet = Tools_ExecuteCmd(szCmd);
     //sed无返回值 不需要判断
     free(pszRet);
     pszRet = NULL;
     memset(szCmd, 0, 200);
     strcpy(szCmd, "wpa_cli -i wlan1 reconfigure");
     pszRet = Tools_ExecuteCmd(szCmd);
     if(0 != strcmp(pszRet, "OK\n"))
     {
        free(pszRet);
        pszRet = NULL;
        return FALSE;
     }
     free(pszRet);
     pszRet = NULL;
     return TRUE;
}

int NetWorkUtil_AddWifiItem(const char* pszSsid, const char* pszPasswd, int iHidSsid)
{
    int iNetWorkID = NetWorkUtil_GetWifiItemFromSsid(pszSsid);
    if(iNetWorkID < 0)
    {
        char* pszNetWorkID = Tools_ExecuteCmd("wpa_cli -i wlan1 add_network");
        iNetWorkID = atoi(pszNetWorkID);
        free(pszNetWorkID);
        pszNetWorkID = NULL;
    }

    char szCmd[300] = {0};
    // if(NetWorkUtil_HasUtfChar(pszSsid))
    // {
    //     char* pszEncodeSsid = NetWorkUtil_EncodeUtf8(pszSsid);
    //     sprintf(szCmd, "wpa_cli -i wlan1 set_network %d ssid %s", iNetWorkID, pszEncodeSsid);
    //     printf("%s\n", szCmd);
    //     free(pszEncodeSsid);
    //     pszEncodeSsid = NULL;
    // }
    // else
    // {
    //     sprintf(szCmd, "wpa_cli -i wlan1 set_network %d ssid '\"%s\"'", iNetWorkID, pszSsid);
    //     printf("%s\n", szCmd);
    // }
    char* pszEncodeSsid = NetWorkUtil_EncodeUtf8(pszSsid);
    sprintf(szCmd, "wpa_cli -i wlan1 set_network %d ssid %s", iNetWorkID, pszEncodeSsid);
    free(pszEncodeSsid);
    pszEncodeSsid = NULL;
    
    char* pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
        free(pszRet);
        pszRet = NULL;
        return -1;
    }
    free(pszRet);
    pszRet = NULL;
    memset(szCmd, 0, 100);
    if(strlen(pszPasswd) == 0)
    {
        sprintf(szCmd, "wpa_cli -i wlan1 set_network %d key_mgmt NONE", iNetWorkID);
    }
    else
    {
        sprintf(szCmd, "wpa_cli -i wlan1 set_network %d psk '\"%s\"'", iNetWorkID, pszPasswd);
    }
    pszRet = Tools_ExecuteCmd(szCmd);
        
    if(0 != strcmp(pszRet, "OK\n"))
    {
        if(strlen(pszPasswd) > 0)
        {
            BOOL bRet = NetWorkUtil_SetWlan1Password(iNetWorkID, pszPasswd);
            if(FALSE == bRet)
            {
                NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
                free(pszRet);
                pszRet = NULL;
                return -1;
            }
        }
        else
        {
            NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
            free(pszRet);
            pszRet = NULL;
            return -1;
        }
    }
    free(pszRet);
    pszRet = NULL;  
    if(iHidSsid == 1)
    {
        memset(szCmd, 0, 100);
        sprintf(szCmd, "wpa_cli -i wlan1 set_network %d scan_ssid 1", iNetWorkID);
        pszRet = Tools_ExecuteCmd(szCmd);
        if(0 != strcmp(pszRet, "OK\n"))
        {
            printf("%s\n", "failed 2");
            NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
            free(pszRet);
            pszRet = NULL;
            return -1;
        }
        free(pszRet);
        pszRet = NULL; 
    } 
    // memset(szCmd, 0, 100);
    // sprintf(szCmd, "wpa_cli -i wlan1 enable_network %d", iNetWorkID);
    // pszRet = Tools_ExecuteCmd(szCmd);
    // if(0 != strcmp(pszRet, "OK\n"))
    // {
    //     NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
    //     free(pszRet);
    //     pszRet = NULL;
    //     return -1;
    // }
    // free(pszRet);
    // pszRet = NULL;  
    memset(szCmd, 0, 100);
    strcpy(szCmd, "wpa_cli -i wlan1 save_config");
    pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        printf("%s\n", "failed 3");
        NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
        free(pszRet);
        pszRet = NULL;
        return -1;
    }

    free(pszRet);
    pszRet = NULL;  
    // int iUdhcpcID = NetWorkUtil_GetWlan1UdhcpcPid();
    // if(iUdhcpcID > 0)
    // {
    //     memset(szCmd, 0, 100);
    //     sprintf(szCmd, "kill -9 %d", iUdhcpcID);
    //     Tools_ExecuteCmd2(szCmd);
    // }
    // Tools_ExecuteCmd2("ifconfig wlan1 0.0.0.0");
    
    // memset(szCmd, 0, 100);
    // sprintf(szCmd, "wpa_cli -i wlan1 select_network %d", iNetWorkID);
    // pszRet = Tools_ExecuteCmd(szCmd);
    // if(0 != strcmp(pszRet, "OK\n"))
    // {
    //     NetWorkUtil_RemoveStoreWifiItem(iNetWorkID);
    //     free(pszRet);
    //     pszRet = NULL;
    //     return -1;
    // }
    // free(pszRet);
    // pszRet = NULL;  
    return iNetWorkID;
}

int NetWorkUtil_ConnectWifiItem(int iNetWorkID, char* pszPasswd)
{
    char szCmd[100] = {0};
    memset(szCmd, 0, 100);
    char* pszRet = NULL;
    if(strlen(pszPasswd) > 0)
    {
        //pszPasswd 表示密码没有改变 不需要在配置了
        sprintf(szCmd, "wpa_cli -i wlan1 set_network %d psk '\"%s\"'", iNetWorkID, pszPasswd);
        pszRet = Tools_ExecuteCmd(szCmd);
        if(0 != strcmp(pszRet, "OK\n"))
        {
            BOOL bRet = NetWorkUtil_SetWlan1Password(iNetWorkID, pszPasswd);
            if(FALSE == bRet)
            {
                free(pszRet);
                pszRet = NULL;
                return -1;
            }
        }
        free(pszRet);
        pszRet = NULL;   
    }
    int iUdhcpcID = NetWorkUtil_GetWlan1UdhcpcPid();
    if(iUdhcpcID > 0)
    {
        memset(szCmd, 0, 100);
        sprintf(szCmd, "kill -9 %d", iUdhcpcID);
        Tools_ExecuteCmd2(szCmd);
    }
    Tools_ExecuteCmd2("ifconfig wlan1 0.0.0.0");

    memset(szCmd, 0, 100);
    sprintf(szCmd, "wpa_cli -i wlan1 enable_network %d", iNetWorkID);
    pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;
        return -1;
    }
    free(pszRet);
    pszRet = NULL;  
    memset(szCmd, 0, 100);
    strcpy(szCmd, "wpa_cli -i wlan1 save_config");
    pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;
        return -1;
    }

    free(pszRet);
    pszRet = NULL;  
    //disconnect状态会连接不上 这里要选择一次
    memset(szCmd, 0, 100);
    sprintf(szCmd, "wpa_cli -i wlan1 select_network %d", iNetWorkID);
    pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;
        return -1;
    }
    free(pszRet);
    pszRet = NULL;  
    return iNetWorkID;
}

BOOL NetWorkUtil_RemoveStoreWifiItem(int iNetWorkID)
{
    char szCmd[100] = {0};
    sprintf(szCmd, "wpa_cli -i wlan1 remove_network %d", iNetWorkID);
    char* pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;  
        return FALSE;
    }
    free(pszRet);
    pszRet = NULL;  
    memset(szCmd, 0, 100);
    strcpy(szCmd, "wpa_cli -i wlan1 save_config");
    pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;  
        return FALSE;
    }
    free(pszRet);
    pszRet = NULL;  
    return TRUE;
}

BOOL NetWorkUtil_DisableStoreWifiItem(int iNetWorkID)
{
    char szCmd[100] = {0};
    sprintf(szCmd, "wpa_cli -i wlan1 disable_network %d", iNetWorkID);
    char* pszRet = Tools_ExecuteCmd(szCmd);
    if(0 != strcmp(pszRet, "OK\n"))
    {
        free(pszRet);
        pszRet = NULL;  
        return FALSE;
    }
    free(pszRet);
    pszRet = NULL; 
    int iUdhcpcID = NetWorkUtil_GetWlan1UdhcpcPid();
    if(iUdhcpcID > 0)
    {
        memset(szCmd, 0, 100);
        sprintf(szCmd, "kill -9 %d", iUdhcpcID);
        Tools_ExecuteCmd2(szCmd);
    }
    Tools_ExecuteCmd2("ifconfig wlan1 0.0.0.0");
    return TRUE; 
}

void NetWorkUtil_Disconnect()
{
    Tools_ExecuteCmd2("wpa_cli disconnect");
    int iUdhcpcID = NetWorkUtil_GetWlan1UdhcpcPid();
    if(iUdhcpcID > 0)
    {
        char szCmd[100] = {0};
        sprintf(szCmd, "kill -9 %d", iUdhcpcID);
        Tools_ExecuteCmd2(szCmd);
    }
    Tools_ExecuteCmd2("ifconfig wlan1 0.0.0.0");
}

BOOL NetWorkUtil_HasUtfChar(const char* pBuffer)
{
    for(int i = 0; i < strlen(pBuffer); ++i)
    {
        char szt = pBuffer[i];
        if (szt&0xE0 && pBuffer[i]&0X80 && pBuffer[i+1]&0x80 && pBuffer[i+2]&0x80)
        {
            return TRUE;
        }
    }
    return FALSE;
}

char* NetWorkUtil_EncodeUtf8(const char* pBuffer)
{
    int iEncodeBufferLen = strlen(pBuffer) *3 + 1;
    char*pEncodeBuffer = (char*)malloc(iEncodeBufferLen);
    memset(pEncodeBuffer, 0, iEncodeBufferLen);
    char* pszRet = pEncodeBuffer;
    for(int i = 0; i < strlen(pBuffer); ++i)
    {
        char szt = pBuffer[i];
        if (szt&0xE0 && pBuffer[i]&0X80 && pBuffer[i+1]&0x80 && pBuffer[i+2]&0x80&& pBuffer[i+3]&0x80)
        {
            char szBuffer[10] = {0};
            sprintf(szBuffer, "%X%X%X%X", (unsigned char)pBuffer[i], (unsigned char)pBuffer[i+1], (unsigned char)pBuffer[i+2], (unsigned char)pBuffer[i+3]);
            strcat(pEncodeBuffer, szBuffer);
            i++;
            i++;
            i++;
            pszRet += 8;
        }
        else if (szt&0xE0 && pBuffer[i]&0X80 && pBuffer[i+1]&0x80 && pBuffer[i+2]&0x80)
        {
            char szBuffer[10] = {0};
            sprintf(szBuffer, "%X%X%X", (unsigned char)pBuffer[i], (unsigned char)pBuffer[i+1], (unsigned char)pBuffer[i+2]);
            strcat(pEncodeBuffer, szBuffer);
            i++;
            i++;
            pszRet += 6;
        }
        // else if(pBuffer[i] == '"'||pBuffer[i] == '\\'||pBuffer[i] == '\'')
        // {
        //     pszRet[0] = '\\';
        //     pszRet[1] = pBuffer[i];
        //     pszRet++;
        //     pszRet++;
        // }
        else
        {
            char szBuffer[10] = {0};
            sprintf(szBuffer, "%X", pBuffer[i]);
            strcat(pszRet, szBuffer);
            pszRet += strlen(szBuffer);
        }
    }
    return pEncodeBuffer;
}

char* NetWorkUtil_DecodeUtf8(char* pszBuffer)
{
    int iDecodeLen = strlen(pszBuffer) + 1;
    char* pDecodeBuffer = (char*)malloc(iDecodeLen);
    memset(pDecodeBuffer, 0, iDecodeLen);
    int iBufferIndex = 0;
    for (int i = 0; i < strlen(pszBuffer); ++i)
    {
        char* pTmp = pszBuffer + i;
        if(0 == memcmp(pTmp, "\\x", 2) && 0 == memcmp((pTmp + 4), "\\x", 2) && 0 == memcmp((pTmp + 8) , "\\x", 2) && 0 == memcmp((pTmp + 12) , "\\x", 2))
        {
            char szTmpBuffer[5] = {0};
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 2)[0], (pTmp + 3)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            memset(szTmpBuffer, 0, 5);
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 6)[0], (pTmp + 7)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            memset(szTmpBuffer, 0, 5);
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 10)[0], (pTmp + 11)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            memset(szTmpBuffer, 0, 5);
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 14)[0], (pTmp + 15)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            i+=15;
        }
        else if(0 == memcmp(pTmp, "\\x", 2) && 0 == memcmp((pTmp + 4), "\\x", 2) && 0 == memcmp((pTmp + 8) , "\\x", 2))
        {
            char szTmpBuffer[5] = {0};
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 2)[0], (pTmp + 3)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            memset(szTmpBuffer, 0, 5);
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 6)[0], (pTmp + 7)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            memset(szTmpBuffer, 0, 5);
            sprintf(szTmpBuffer, "0x%c%c", (pTmp + 10)[0], (pTmp + 11)[0]);
            (pDecodeBuffer + iBufferIndex++)[0] = strtol(szTmpBuffer, NULL, 16);
            i+=11;
        }
        else if(pszBuffer[i] == '\\')
        {
            (pDecodeBuffer + iBufferIndex++)[0] = pTmp[1];
            i += 1;
        }
        else
        {
            (pDecodeBuffer + iBufferIndex++)[0] = pTmp[0];
        }
    }
    return pDecodeBuffer;
}

char* NetWorkUtil_ToHexString(char* pszBuffer)
{
    int iFromLen = strlen(pszBuffer);
    int iLen = iFromLen * 4 + 1;
    char* pRetBuffer = (char*)malloc(iLen);
    memset(pRetBuffer, 0, iLen);
    int iIndex = 0;
    for(int i = 0; i < iFromLen; i += 2)
    {
        char szBuffer[5] = {0};
        sprintf(szBuffer, "\\x%c%c", (unsigned char)pszBuffer[i], (unsigned char)pszBuffer[i + 1]);
        memcpy(pRetBuffer + iIndex, szBuffer, 4);
        iIndex += 4;
    }
    return pRetBuffer;
}