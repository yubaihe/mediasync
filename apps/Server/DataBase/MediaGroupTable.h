#pragma once
#include "../stdafx.h"
struct MediaGroupItem
{
    int iID;
    string strName;
    string strCoverPic;
};

class CMediaGroupTable
{
public:
    CMediaGroupTable();
    ~CMediaGroupTable();
    static BOOL CreateTable();
    static string GetJsonAllGroups();
    static int AddGroup(string strGroupName);
    static int GroupIdFromName(string strGroupName);
    static BOOL SetCoverEmpty(string strCoverPic);
    static BOOL SetCover(int iID, string strCoverPic);
    static BOOL RemoveItemFromID(int iID);
    static BOOL RemoveItemFromName(string strName);
    static string GroupNameFromID(int iID);
    static string GetJsonAllGroupsFromGids(string strGids);
    static BOOL GroupItemUpdate(int iID, string strGroupName);
    static string GetCoverPic(int iID);
private:
    static list<MediaGroupItem> AssembleItems(list<map<string, string>> List);
};

