#pragma once
#include "../stdafx.h"
#include "MediaDefine.h"
using namespace std;
typedef	void(*OnInitPhotoCallBack)(int iPrecent, LPVOID* lpParameter);



class CPhotoManager
{
public:
	CPhotoManager();
	~CPhotoManager();
	void Empty();
	void Init(string strOrignalRoot, string strThumbRoot, OnInitPhotoCallBack callBack, LPVOID* lpParameter, BOOL bSameNamePrefix = FALSE);
	list<int> Init(vector<string> OrignalFileVec, string strOrignalRoot, string strThumbRoot, OnInitPhotoCallBack callBack, LPVOID* lpParameter, BOOL bSameNamePrefix = FALSE);
	list<string> GetItems(int iStart, int iLimited);
	list<string> GetItems2(int iStart, int iLimited);
	int GetItemCount();
	vector<string> GetAddr(int iItemIndex);
	string GetFile(int iItemIndex);
	string GetThumb(int iItemIndex);
	string GetCreateDate(int iItemIndex);
	void RemoveItem(PhotoItem* pItem);
	void RemoveItem(int iItemIndex);
	int GetDuration(int iItemIndex);
	void AddIgnoreItem(int iItemIndex);
	void RemoveIgnoreItem(int iItemIndex);
	list<string> GetIgnoreItems(int iStart, int iLimited);
	BOOL IsIgnore(int iItemIndex);
	int GetNextVisiableIndex(int iCurItemIndex, BOOL bIgnoreGroup);
	int GetPrevVisiableIndex(int iCurItemIndex, BOOL bIgnoreGroup);
	void Print();
	void GetAllFiles(const char* pszRoot);
	PhotoExtra GetExtraByItemIndex(int iItemIndex);
	PhotoItem GetItemByItemIndex(int iItemIndex);
	static BOOL GenThumbPic(string strOrginalFile, string strThumbFile);
	string GetThumbFileName(PhotoItem* pItem);
private:
	PhotoExtra* GetExtraByItemIndexInner(int iItemIndex);
	PhotoItem* GetItemByItemIndexInner(int iItemIndex);
	PhotoItem* GetItemByPositionInner(int iPosition);
	void FilterIgnore();
	void AddIgnoreItem(PhotoItem* pItem);
	void RemoveIgnoreItem(PhotoItem* pItem);
	void SaveIgnoreItems();
	void AddFile(const char* pszPath, const char* pszFile);
	static DWORD FilterFileProc(LPVOID* lpParameter);
	BOOL GenPicThumb(int iIndex, string strFile);
	BOOL GenVideoThumb(int iIndex, string strFile);
	void GetImageItem(int iThumbID, string strFile);
	void GetVideoItem(int iThumbID, string strFile);
	string GetTempFile(int* pIndex);
private:
	CRITICAL_SECTION m_PhotosVectorSection;
	vector<PhotoItem*> m_PhotosVector;
	map<int, PhotoExtra*> m_PhotoExtraMap;
	vector<PhotoItem*> m_IgnorePhotosVector;
	map<int, PhotoExtra*> m_IgnorePhotoExtraMap;
	string m_strRoot;
	string m_strTempRoot;
	BOOL m_bSameNamePrefix;
	CRITICAL_SECTION m_TmpPhotoFilesSection;
	vector<string> m_TmpPhotoFilesVector;
	int m_iTmpPhotoFilesIndex;
	LPVOID* m_lpParameter;
};

