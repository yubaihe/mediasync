#pragma once
#include "../stdafx.h"
struct MediaGpsItem
{
    string strGps;
    string strGps2;
    string strLocation;
};
class CMediaGpsTable
{
public:
    CMediaGpsTable();
    ~CMediaGpsTable();
    BOOL CreateTable();
    BOOL QueryItem(string strGps, MediaGpsItem& item);
    BOOL GetRecordFromGps(string strGps, MediaGpsItem& item);
    BOOL GetRecordFromGps2(string strGps2, MediaGpsItem& item);
    BOOL SetRecord(string strGps, string strGps2, string strLocation);
    BOOL RemoveAll();
private:
    list<MediaGpsItem> AssembleItems(list<map<string, string>> List);
};

