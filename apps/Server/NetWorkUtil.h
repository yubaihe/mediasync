#ifndef RELECH_NETWORKUTIL__
#define RELECH_NETWORKUTIL__
#include "stdafx.h"
#define MAX_SSID_LEN 33
#define MAX_BSSID_LEN 18
#define MAX_IPADDR_LEN 16
#define MAX_WIFIPASSWORDLEN 64
#define MAX_WIFICRYPTTYPELEN 50
typedef struct 
{
    char szSsid[MAX_SSID_LEN];
    char szBssid[MAX_BSSID_LEN];
    char szCrypt[MAX_WIFICRYPTTYPELEN];
    int iSignal;
} WifiItem;
typedef struct 
{
    char szSsid[MAX_SSID_LEN];
    char szBssid[MAX_BSSID_LEN];
    char szIpAddr[MAX_IPADDR_LEN];

    int iNetworkID;
    int iFlags;
} WifiStoreItem;
typedef struct 
{
    char szSsid[MAX_SSID_LEN];
    char szPasswd[MAX_WIFIPASSWORDLEN];
} ZHongJiItem;

typedef enum
{
    BRTYPE_NONE = 0,
    BRTYPE_ETH0,
    BRTYPE_WLAN1
}BRTYPE;
typedef enum
{
    WPASTATUS_INACTIVE = 0,
    WPASTATUS_SCANNING,
    WPASTATUS_ASSOCIATING,
    WPASTATUS_ASSOCIATED,
    WPASTATUS_COMPLETED,
    WPASTATUS_DISCONNECTED,
    WPASTATUS_4WAY_HANDSHAKE,
    WPASTATUS_INTERFACE_DISABLED
}WPASTATUS;
BOOL NetWorkUtil_WifiScan();
struct json_object* NetWorkUtil_GetWifiScanItems();
BOOL NetWorkUtil_GetWifiName(char* pszWifiName, char* pszWifiPwd);
BRTYPE NetWorkUtil_GetBrtype();
void NetWorkUtil_SetBrtype(BRTYPE BrType);
int NetWorkUtil_GetWlan1UdhcpcPid();
void NetWorkUtil_RestartWlan1Udhcpc();
int NetWorkUtil_GetWlan1Status(char* pszSsid, char* pszIpAddr, int* iNetWorkID);
struct json_object* NetWorkUtil_GetWlan1StoreItems();
void NetWorkUtil_SetHotPot(const char* pszSsid, const char* pszPasswd);
int NetWorkUtil_GetWifiItemFromSsid(const char* pszSsid);
int NetWorkUtil_AddWifiItem(const char* pszSsid, const char* pszPasswd, int iHidSsid);
int NetWorkUtil_ConnectWifiItem(int iNetWorkID, char* pszPasswd);
BOOL NetWorkUtil_RemoveStoreWifiItem(int iNetWorkID);
BOOL NetWorkUtil_DisableStoreWifiItem(int iNetWorkID);
void NetWorkUtil_Disconnect();
char* NetWorkUtil_EncodeUtf8(const char* pBuffer);
BOOL NetWorkUtil_HasUtfChar(const char* pBuffer);
char* NetWorkUtil_DecodeUtf8(char* pszBuffer);
char* NetWorkUtil_ToHexString(char* pszBuffer);

#endif