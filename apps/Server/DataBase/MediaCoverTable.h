#pragma once
#include "../stdafx.h"
#define DATABASECOVER_HOME 0
struct MediaCoverItem
{
    int iCoverType;
    string strMediaAddr;
};

class CMediaCoverTable
{
private:
public:
    CMediaCoverTable();
    ~CMediaCoverTable();
    static BOOL CreateTable();
    static BOOL SetHomeCover(string strMediaAddr);
    static BOOL RemoveHomeCover();
    static string GetHomeCover();
private:
    static list<MediaCoverItem> AssembleItems(list<map<string, string>> List);
};

