#pragma once
#include "../stdafx.h"
struct CommentItem
{
    int iID;
    int iMediaID;
    string strModule;
    string strComment;
};
class CCommentTable
{
public:
    CCommentTable();
    ~CCommentTable();
    static BOOL CreateTable();
    static BOOL InsertItem(CommentItem item);
    static BOOL AddItem(int iMediaID, string strComment, string strModule = "");
    static BOOL UpdateItem(int iMediaID, string strComment, string strModule = "");
    static BOOL DeleteItem(int iMediaID, string strModule = "");
    static BOOL DeleteItems(string strMediaIDs, string strModule = "");
    static nlohmann::ordered_json GetItems(int iID, int iLimited, string strDevNames, string strQuery = "");
    static BOOL IsEmpty();
private:
};


