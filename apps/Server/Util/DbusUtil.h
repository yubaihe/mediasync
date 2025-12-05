#pragma once
#include "../stdafx.h"

#define DBUS_NETWORK "Network"
#define DBUS_MEDIAPARSE "MediaParse"
#define MAX_DBUSMESSAGE_LEN 65535
class CDbusUtil
{
private:
public:
    CDbusUtil();
    ~CDbusUtil();
    static BOOL ContainModule(string strModuleName);
    static string AllDisks();
    static string MediaParseGps(string strFile);
    static BOOL GenThumb(int iMediaType, string strFile, string strThumbFile);
    static BOOL GetNetworkConfig(string& strEth, string& strWlan, string& strHotpot);
    static BOOL BackupFoldSyncStart();
    static int BackupFoldSyncPrecent(BOOL& bError);
    static int BackupFoldCount(BOOL& bError);
    static BOOL BackupUpdateGps(int iItemID, string strGps, string strAddr);
    static string GetMime(string strFile);
    static string TranscodeStart(string strSrc, string strDest, int iItemID);
    static BOOL TranscodeStop(string strIdentify);
    static BOOL TranscodePrecent(string strIdentify, DWORD& iDurSec, DWORD& iCurSec, int& iPrecent);
    static BOOL TranscodeEnd(string strIdentify);
    static string GetBackupItem(int iItemID);
};


