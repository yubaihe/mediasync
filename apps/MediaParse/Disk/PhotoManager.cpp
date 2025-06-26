#include "PhotoManager.h"
#include "CommonUtil.h"
#include "ImageParse.h"
#include "VideoParse.h"
#include "../Util/FileUtil.h"
#include <map>
#include "CpuDetect.h"
extern BOOL g_bExit;
CPhotoManager::CPhotoManager()
{
	InitializeCriticalSection(&m_PhotosVectorSection);
	InitializeCriticalSection(&m_TmpPhotoFilesSection);
}

CPhotoManager::~CPhotoManager()
{
	Empty();
	DeleteCriticalSection(&m_PhotosVectorSection);
	DeleteCriticalSection(&m_TmpPhotoFilesSection);
}

void CPhotoManager::Empty()
{
	m_TmpPhotoFilesVector.clear();
	m_iTmpPhotoFilesIndex = 0;
	m_strRoot = "";
	m_strTempRoot = "";
	EnterCriticalSection(&m_PhotosVectorSection);
	vector<PhotoItem*>::iterator itor = m_PhotosVector.begin();
	for (; itor != m_PhotosVector.end();)
	{
		PhotoItem* pItem = *itor;
		PhotoExtra* pExtra = GetExtraByItemIndexInner(pItem->iItemIndex);
		itor = m_PhotosVector.erase(itor);
		m_PhotoExtraMap.erase(pItem->iItemIndex);
		delete pItem;
		pItem = NULL;
		delete pExtra;
		pExtra = NULL;
	}
	itor = m_IgnorePhotosVector.begin();
	for (; itor != m_IgnorePhotosVector.end();)
	{
		PhotoItem* pItem = *itor;
		PhotoExtra* pExtra = GetExtraByItemIndexInner(pItem->iItemIndex);
		itor = m_IgnorePhotosVector.erase(itor);
		m_IgnorePhotoExtraMap.erase(pItem->iItemIndex);
		delete pItem;
		pItem = NULL;
		delete pExtra;
		pExtra = NULL;
	}
	LeaveCriticalSection(&m_PhotosVectorSection);
}
list<int> CPhotoManager::Init(vector<string> OrignalFileVec, string strOrignalRoot, string strThumbRoot, OnInitPhotoCallBack callBack, LPVOID* lpParameter, BOOL bSameNamePrefix /*= FALSE*/)
{
	m_bSameNamePrefix = bSameNamePrefix;
	printf("====>%s\n", strOrignalRoot.c_str());
	//源文件路径
	m_strRoot = strOrignalRoot;
	m_strRoot = CCommonUtil::ReplaceStr(m_strRoot, "\\", "/");
	m_lpParameter = lpParameter;
	//缩略图文件路径
	m_strTempRoot = strThumbRoot;
	m_strTempRoot = CCommonUtil::ReplaceStr(m_strTempRoot, "\\", "/");

	m_TmpPhotoFilesVector = OrignalFileVec;
	printf("Tmp Root dir:%s\n", m_strTempRoot.c_str());
	m_iTmpPhotoFilesIndex = 0;

	int iFilterThreadCount = CCpuDetect::GetInstance()->GetCpuCount()/2;
	if(iFilterThreadCount < 1)
	{
		iFilterThreadCount = 1;
	}
	pthread_t hFilterFile[iFilterThreadCount] = { 0 };
	for (int i = 0; i < iFilterThreadCount; ++i)
	{
		hFilterFile[i] = (pthread_t)LinuxCreateThread(NULL, (LPTHREAD_START_ROUTINE)FilterFileProc, this);
	}
	int iPrecent = 0;
	while (m_iTmpPhotoFilesIndex != (int)m_TmpPhotoFilesVector.size())
	{
		if(TRUE == g_bExit)
		{
			break;
		}
		int iNewPrecent = m_iTmpPhotoFilesIndex * 100 / m_TmpPhotoFilesVector.size();
		if(iNewPrecent != iPrecent)
		{
			callBack(iNewPrecent, m_lpParameter);
			iPrecent = iNewPrecent;
		}
		
		Sleep(500);
	}
	for(int i = 0; i < iFilterThreadCount; ++i)
	{
		pthread_join(hFilterFile[i], NULL);
		hFilterFile[i] = 0;
	}
	list<int> retList;
	EnterCriticalSection(&m_PhotosVectorSection);
	vector<PhotoItem*>::iterator itor = m_PhotosVector.begin();
	for (; itor != m_PhotosVector.end(); ++itor)
	{
		retList.push_back((*itor)->iItemIndex);
	}
	LeaveCriticalSection(&m_PhotosVectorSection);
	callBack(100, m_lpParameter);
	return retList;
}
void CPhotoManager::Init(string strOrignalRoot, string strThumbRoot, OnInitPhotoCallBack callBack, LPVOID* lpParameter, BOOL bSameNamePrefix /*= FALSE*/)
{
	m_bSameNamePrefix = bSameNamePrefix;
	printf("====>%s\n", strOrignalRoot.c_str());
	//源文件路径
	m_strRoot = strOrignalRoot;
	m_strRoot = CCommonUtil::ReplaceStr(m_strRoot, "\\", "/");
	m_lpParameter = lpParameter;
	//缩略图文件路径
	m_strTempRoot = strThumbRoot;
	m_strTempRoot = CCommonUtil::ReplaceStr(m_strTempRoot, "\\", "/");
	CCommonUtil::RemoveFold(m_strTempRoot.c_str());
	CCommonUtil::CreateFold(m_strTempRoot.c_str());

	GetAllFiles(m_strRoot.c_str());
	printf("Root dir:%s\n", m_strRoot.c_str());
	printf("Tmp Root dir:%s\n", m_strTempRoot.c_str());
	m_iTmpPhotoFilesIndex = 0;

	int iFilterThreadCount = CCpuDetect::GetInstance()->GetCpuCount()/2;
	if(iFilterThreadCount < 1)
	{
		iFilterThreadCount = 1;
	}
	pthread_t hFilterFile[iFilterThreadCount] = { 0 };
	for (int i = 0; i < iFilterThreadCount; ++i)
	{
		hFilterFile[i] = (pthread_t)LinuxCreateThread(NULL, (LPTHREAD_START_ROUTINE)FilterFileProc, this);
	}
	int iPrecent = 0;
	while (m_iTmpPhotoFilesIndex != (int)m_TmpPhotoFilesVector.size())
	{
		if(TRUE == g_bExit)
		{
			break;
		}
		int iNewPrecent = m_iTmpPhotoFilesIndex * 100 / m_TmpPhotoFilesVector.size();
		if(iNewPrecent != iPrecent)
		{
			callBack(iNewPrecent, m_lpParameter);
			iPrecent = iNewPrecent;
		}
		
		Sleep(500);
	}
	for(int i = 0; i < iFilterThreadCount; ++i)
	{
		pthread_join(hFilterFile[i], NULL);
		hFilterFile[i] = 0;
	}
	FilterIgnore();
	callBack(100, m_lpParameter);
}

void CPhotoManager::GetAllFiles(const char* pszRoot)
{
	DIR* pDir = opendir(pszRoot);
    if (pDir == NULL) 
    {
        return;
    }
    struct dirent* pEntry = NULL;
    while ((pEntry = readdir(pDir)))
    {
		
        // 忽略 "." 和 ".." 条目
        if (pEntry->d_name[0] == '.' && 
                (pEntry->d_name[1] == '\0' || (pEntry->d_name[1] == '.' && pEntry->d_name[2] == '\0'))
        ) 
        {
            continue;
        }
        // 构建完整路径
        char szFullPath[1024] {0};
		if(pszRoot[strlen(pszRoot) - 1] == '/')
		{
			snprintf(szFullPath, sizeof(szFullPath), "%s%s", pszRoot, pEntry->d_name);
		}
		else
		{
			snprintf(szFullPath, sizeof(szFullPath), "%s/%s", pszRoot, pEntry->d_name);
		}
        // m_FileList.push_back(szFullPath);
		AddFile(pszRoot, pEntry->d_name);

        struct stat pathStat;
        stat(szFullPath, &pathStat);
        // 如果是目录，则递归调用
        if (S_ISDIR(pathStat.st_mode)) 
        {
            GetAllFiles(szFullPath);
        }
    }
    closedir(pDir);
}

void CPhotoManager::AddFile(const char* pszPath, const char* pszFile)
{
	string str(pszPath);
	str.append("/");
	str.append(pszFile);
	string strNewFile = str.substr(m_strRoot.size(), str.size() - m_strRoot.size());
	m_TmpPhotoFilesVector.push_back(strNewFile);
}

string CPhotoManager::GetTempFile(int* pIndex)
{
	EnterCriticalSection(&m_TmpPhotoFilesSection);
	if (m_iTmpPhotoFilesIndex == (int)m_TmpPhotoFilesVector.size())
	{
		LeaveCriticalSection(&m_TmpPhotoFilesSection);
		return "";
	}
	*pIndex = m_iTmpPhotoFilesIndex;
	string strTempFile = m_TmpPhotoFilesVector[m_iTmpPhotoFilesIndex];
	m_iTmpPhotoFilesIndex++;
	LeaveCriticalSection(&m_TmpPhotoFilesSection);
	return strTempFile;
}

DWORD CPhotoManager::FilterFileProc(LPVOID* lpParameter)
{
	CPhotoManager* pPhotoManager = (CPhotoManager*)lpParameter;
	while (FALSE == g_bExit)
	{
		int iIndex = 0;
		string strTempFile = pPhotoManager->GetTempFile(&iIndex);
		if (strTempFile.length() == 0)
		{
			break;
		}
		string strFile(pPhotoManager->m_strRoot);
		strFile.append(strTempFile);
		printf("%s\n", pPhotoManager->m_strRoot.c_str());
		printf("%s\n", strTempFile.c_str());
		printf("%s\n", strFile.c_str());
		string strMimeType = CCommonUtil::GetMime(strFile.c_str());
		if (TRUE == CCommonUtil::IsMimeTypeImage(strMimeType.c_str()))
		{
			//string str = CCommonUtil::StringFormat("FileName:%s MimeType:%s  图片\r\n", strFile.c_str(), strMimeType.c_str());
			//printf("%s\n", str.c_str());
			if (pPhotoManager->GenPicThumb(iIndex, strFile))
			{
				pPhotoManager->GetImageItem(iIndex, strTempFile);
			}
		}
		else if (TRUE == CCommonUtil::IsMimeTypeVideo(strMimeType.c_str()))
		{
			//string str = CCommonUtil::StringFormat("FileName:%s MimeType:%s  视频\r\n", strFile.c_str(), strMimeType.c_str());
			//printf("%s\n", str.c_str());
			if (pPhotoManager->GenPicThumb(iIndex, strFile))
			{
				pPhotoManager->GetVideoItem(iIndex, strTempFile);
			}
		}
	}
	return 1;
}
BOOL CPhotoManager::GenPicThumb(int iIndex, string strFile)
{
	string strThumb = CCommonUtil::StringFormat("%sthumbtmp_%ld.jpg", m_strTempRoot.c_str(), iIndex);
	CImageParse imageParse;
    BOOL bSuccess = imageParse.GetThumbnail(strFile.c_str(), strThumb.c_str(), PIC_WIDTH, PIC_HEIGHT);
	return bSuccess;
}

BOOL CPhotoManager::GenVideoThumb(int iIndex, string strFile)
{
	string strThumb = CCommonUtil::StringFormat("%sthumbtmp_%ld.jpg", m_strTempRoot.c_str(), iIndex);
	CVideoParse videoParse;
    BOOL bSuccess = videoParse.GetThumbnail(strFile.c_str(), strThumb.c_str(), PIC_WIDTH, PIC_HEIGHT);
	return bSuccess;
}

void CPhotoManager::GetImageItem(int iThumbID, string strFile)
{
	PhotoItem* pItem = new PhotoItem();
	PhotoExtra* pExtra = new PhotoExtra();

	string strPathFile(m_strRoot);
	strPathFile.append(strFile);
	int iIndex = strFile.rfind("/") + 1;

	pItem->strName = strFile.substr(iIndex, strFile.length() - iIndex);
	pItem->iFileSize = CCommonUtil::GetFileSize(strPathFile.c_str());
	pItem->iMediaType = MEDIATYPE_IMAGE;
	printf("%s\n", strPathFile.c_str());
	CImageParse imageParse;
	imageParse.Parse(strPathFile.c_str());
	pExtra->strFileName = strFile;
	pExtra->iAddTime = CCommonUtil::TimeToSec(imageParse.GetCreateTime());
	pExtra->iDuration = 0;
	pExtra->vecLocation = imageParse.GetLocaiton();

	EnterCriticalSection(&m_PhotosVectorSection);
	size_t iItemID = m_PhotosVector.size();
	pItem->iWidth = imageParse.GetWidth();
	pItem->iHeight = imageParse.GetHeight();
	pItem->iItemIndex = iItemID;
	m_PhotosVector.push_back(pItem);
	m_PhotoExtraMap.insert(pair<int, PhotoExtra*>(pItem->iItemIndex, pExtra));
	LeaveCriticalSection(&m_PhotosVectorSection);
	string strThumb1 = CCommonUtil::StringFormat("%sthumbtmp_%ld.jpg", m_strTempRoot.c_str(), iThumbID);
	string strThumb2 = CCommonUtil::StringFormat("%sthumb_%ld.jpg", m_strTempRoot.c_str(), iItemID);
	if(TRUE == m_bSameNamePrefix)
	{
		strThumb2 = CCommonUtil::StringFormat("%s%s", m_strTempRoot.c_str(), GetThumbFileName(pItem).c_str());
	}
	CCommonUtil::MoveFile(strThumb1, strThumb2);
}

void CPhotoManager::GetVideoItem(int iThumbID, string strFile)
{
	PhotoItem* pItem = new PhotoItem();
	PhotoExtra* pExtra = new PhotoExtra();

	string strPathFile(m_strRoot);
	strPathFile.append(strFile);
	int iIndex = strFile.rfind("/") + 1;
	pItem->strName = strFile.substr(iIndex, strFile.length() - iIndex);
	pItem->iFileSize = CCommonUtil::GetFileSize(strPathFile.c_str());
	pItem->iMediaType = MEDIATYPE_VIDEO;

	CVideoParse videoParse;
	videoParse.Parse(strPathFile.c_str());
	pExtra->strFileName = strFile;
	pExtra->iAddTime = CCommonUtil::TimeToSec(videoParse.GetCreateTime());
	pExtra->iDuration = videoParse.GetDurationSec();
	pExtra->vecLocation = videoParse.GetLocaiton();

	pItem->iWidth = videoParse.GetWidth();
	pItem->iHeight = videoParse.GetHeight();

	EnterCriticalSection(&m_PhotosVectorSection);
	size_t iItemID = m_PhotosVector.size();
	pItem->iItemIndex = iItemID;
	m_PhotosVector.push_back(pItem);
	m_PhotoExtraMap.insert(pair<int, PhotoExtra*>(pItem->iItemIndex, pExtra));
	LeaveCriticalSection(&m_PhotosVectorSection);

	string strThumb1 = CCommonUtil::StringFormat("%sthumbtmp_%ld.jpg", m_strTempRoot.c_str(), iThumbID);
	string strThumb2 = CCommonUtil::StringFormat("%sthumb_%ld.jpg", m_strTempRoot.c_str(), iItemID);
	if(TRUE == m_bSameNamePrefix)
	{
		strThumb2 = CCommonUtil::StringFormat("%s%s", m_strTempRoot.c_str(), GetThumbFileName(pItem).c_str());
	}
	CCommonUtil::MoveFile(strThumb1, strThumb2);
}

list<string> CPhotoManager::GetIgnoreItems(int iStart, int iLimited)
{
	list<string> retList;
	for (int i = 0; i < iLimited; ++i)
	{
		if (iStart + i == (int)m_IgnorePhotosVector.size())
		{
			return retList;
		}
		int iIndex = iStart + i;
		PhotoItem* pItem = m_IgnorePhotosVector[iIndex];
		if(NULL == pItem)
		{
			continue;
		}
		map<int, PhotoExtra*>::iterator itor = m_IgnorePhotoExtraMap.find(pItem->iItemIndex);
		PhotoExtra* pExtra = itor->second;
		//全路径
		//string strThumbPic = CCommonUtil::StringFormat("%sthumb_%ld.jpg", m_strTempRoot.c_str(), pItem->iThumbID);
		//string strThumbPic = CCommonUtil::StringFormat("thumb_%ld.jpg", pItem->iItemIndex);
		string strThumbPic = GetThumbFileName(pItem);
		//文件名称
		string strItem = CCommonUtil::StringFormat("{\"id\":\"%ld\",\"w\":\"%ld\",\"h\":\"%ld\",\"dur\":\"%d\",\"mtype\":\"%ld\",\"msize\":\"%lld\",\"maddr\":\"%s\"}",
			pItem->iItemIndex, (int)pItem->iWidth, (int)pItem->iHeight, pExtra->iDuration, pItem->iMediaType, pItem->iFileSize, strThumbPic.c_str());
		retList.push_back(strItem);
	}

	return retList;
}
list<string> CPhotoManager::GetItems(int iStart, int iLimited)
{
	EnterCriticalSection(&m_PhotosVectorSection);
	list<string> retList;
	for (int i = 0; i < iLimited; ++i)
	{
		if (iStart + i == (int)(m_PhotosVector.size() + m_IgnorePhotosVector.size()))
		{
			LeaveCriticalSection(&m_PhotosVectorSection);
			return retList;
		}
		int iItemIndex = iStart + i;
		BOOL bIgnore = FALSE;
		map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(iItemIndex);
		if (itor == m_PhotoExtraMap.end())
		{
			itor = m_IgnorePhotoExtraMap.find(iItemIndex);
			bIgnore = TRUE;
		}
		PhotoItem* pItem = GetItemByItemIndexInner(iItemIndex);
		if(NULL == pItem)
		{
			continue;
		}
		PhotoExtra* pExtra = itor->second;
		//全路径
		//string strThumbPic = CCommonUtil::StringFormat("%sthumb_%ld.jpg", m_strTempRoot.c_str(), pItem->iThumbID);
		//文件名称
		//string strThumbPic = CCommonUtil::StringFormat("thumb_%ld.jpg", pItem->iItemIndex);
		string strThumbPic = GetThumbFileName(pItem);
		string strItem = CCommonUtil::StringFormat("{\"id\":\"%ld\",\"w\":\"%ld\",\"h\":\"%ld\",\"dur\":\"%d\",\"mtype\":\"%ld\",\"msize\":\"%lld\",\"maddr\":\"%s\",\"ignore\":%d}",
			pItem->iItemIndex, (int)pItem->iWidth, (int)pItem->iHeight, pExtra->iDuration, pItem->iMediaType, pItem->iFileSize, strThumbPic.c_str(), bIgnore);
		retList.push_back(strItem);
	}
	LeaveCriticalSection(&m_PhotosVectorSection);
	return retList;
}
string CPhotoManager::GetThumbFileName(PhotoItem* pItem)
{
	if(FALSE == m_bSameNamePrefix)
	{
		string strThumbPic = CCommonUtil::StringFormat("thumb_%ld.jpg", pItem->iItemIndex);
		return strThumbPic;
	}
	std::vector<std::string> vec = CCommonUtil::StringSplit(pItem->strName, "/");
	string strFileName = vec[vec.size() - 1];
	string strFilePrefx = "";
	string strPostfix = "";
	CCommonUtil::SpliteFile(strFileName, strFilePrefx, strPostfix);
	return CCommonUtil::StringFormat("%s_%s.jpg", strFilePrefx.c_str(), strPostfix.c_str());
}
list<string> CPhotoManager::GetItems2(int iStart, int iLimited)
{
	EnterCriticalSection(&m_PhotosVectorSection);
	list<string> retList;
	for (int i = 0; i < iLimited; ++i)
	{
		if (iStart + i >= (int)m_PhotosVector.size())
		{
			LeaveCriticalSection(&m_PhotosVectorSection);
			return retList;
		}
		int iItemIndex = iStart + i;
		PhotoItem* pItem = m_PhotosVector[iItemIndex];
		if(NULL == pItem)
		{
			continue;
		}
		map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(pItem->iItemIndex);
		if(itor == m_PhotoExtraMap.end())
		{
			continue;
		}
		PhotoExtra* pExtra = itor->second;
		//string strThumbPic = CCommonUtil::StringFormat("%s/thumb_%ld.jpg", m_strTempRoot.c_str(), pItem->iThumbID);
		string strThumbPic = CCommonUtil::StringFormat("thumb_%ld.jpg", pItem->iItemIndex);
		string strPaiTime = CCommonUtil::SecToTime(pExtra->iAddTime);
		string strFileSize = CCommonUtil::SizeToString(pItem->iFileSize);
		string strDur = CCommonUtil::SecondToTime(pExtra->iDuration);
		string strItem = CCommonUtil::StringFormat("{\"id\":\"%ld\",\"mtype\":\"%ld\",\"paitime\":\"%s\",\"filesize\":\"%s\",\"dur\":\"%s\",\"pic\":\"%s\"}",
			pItem->iItemIndex, (int)pItem->iMediaType, strPaiTime.c_str(), strFileSize.c_str(), strDur.c_str(), strThumbPic.c_str());

		retList.push_back(strItem);
	}
	LeaveCriticalSection(&m_PhotosVectorSection);
	return retList;
}

int CPhotoManager::GetItemCount()
{
	return m_PhotosVector.size();
}

PhotoItem* CPhotoManager::GetItemByItemIndexInner(int iItemIndex)
{
	for (size_t i = 0; i < m_PhotosVector.size(); ++i)
	{
		if (((PhotoItem*)m_PhotosVector[i])->iItemIndex == iItemIndex)
		{
			return (PhotoItem*)m_PhotosVector[i];
		}
	}
	for (size_t i = 0; i < m_IgnorePhotosVector.size(); ++i)
	{
		if (((PhotoItem*)m_IgnorePhotosVector[i])->iItemIndex == iItemIndex)
		{
			return (PhotoItem*)m_IgnorePhotosVector[i];
		}
	}
	return NULL;
}

PhotoItem* CPhotoManager::GetItemByPositionInner(int iPosition)
{
	return m_PhotosVector[iPosition];
}

vector<string> CPhotoManager::GetAddr(int iItemIndex)
{
	map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(iItemIndex);
	if (itor == m_PhotoExtraMap.end())
	{
		itor = m_IgnorePhotoExtraMap.find(iItemIndex);
		if(itor == m_IgnorePhotoExtraMap.end())
		{
			vector<string> vec;
			vec.push_back("0");
			vec.push_back("0");
			return vec;
		}
	}
	PhotoExtra* pExtra = itor->second;
	return pExtra->vecLocation;
}
string CPhotoManager::GetFile(int iItemIndex)
{
	PhotoExtra* pExtra = GetExtraByItemIndexInner(iItemIndex);
	if(NULL == pExtra)
	{
		return "";
	}
	string strFile(m_strRoot);
	strFile.append(pExtra->strFileName);
	return strFile;
}

string CPhotoManager::GetThumb(int iItemIndex)
{
	EnterCriticalSection(&m_PhotosVectorSection);
	PhotoItem* pPhotoItem = GetItemByItemIndexInner(iItemIndex);
	string strFileName = GetThumbFileName(pPhotoItem);
	LeaveCriticalSection(&m_PhotosVectorSection);
	string strThumbPic = CCommonUtil::StringFormat("%s%s", m_strTempRoot.c_str(), strFileName.c_str());
	return strThumbPic;
}

string CPhotoManager::GetCreateDate(int iItemIndex)//拍摄时间
{
	map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(iItemIndex);
	if (itor == m_PhotoExtraMap.end())
	{
		itor = m_IgnorePhotoExtraMap.find(iItemIndex);
	}
	PhotoExtra* pExtra = itor->second;
	return CCommonUtil::SecToTime(pExtra->iAddTime);
}

void CPhotoManager::RemoveItem(PhotoItem* pItem)
{
	map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(pItem->iItemIndex);
	PhotoExtra* pExtra = itor->second;
	m_PhotoExtraMap.erase(pItem->iItemIndex);
	delete pExtra;
	pExtra = NULL;

	vector<PhotoItem*>::iterator itor2 = m_PhotosVector.begin();
	for (; itor2 != m_PhotosVector.end(); ++itor2)
	{
		PhotoItem* pTmpItem = *itor2;
		if (pItem->iItemIndex == pTmpItem->iItemIndex)
		{
			m_PhotosVector.erase(itor2);
			delete pTmpItem;
			pTmpItem = NULL;
			break;
		}
	}
}
void CPhotoManager::RemoveItem(int iItemIndex)
{
	EnterCriticalSection(&m_PhotosVectorSection);
	map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(iItemIndex);
	if(itor == m_PhotoExtraMap.end())
	{
		LeaveCriticalSection(&m_PhotosVectorSection);
		return;
	}
	PhotoExtra* pExtra = itor->second;
	m_PhotoExtraMap.erase(iItemIndex);
	delete pExtra;
	pExtra = NULL;

	vector<PhotoItem*>::iterator itor2 = m_PhotosVector.begin();
	for (; itor2 != m_PhotosVector.end(); ++itor2)
	{
		PhotoItem* pTmpItem = *itor2;
		if (iItemIndex == pTmpItem->iItemIndex)
		{
			m_PhotosVector.erase(itor2);
			delete pTmpItem;
			pTmpItem = NULL;
			break;
		}
	}
	LeaveCriticalSection(&m_PhotosVectorSection);
}
PhotoExtra CPhotoManager::GetExtraByItemIndex(int iItemIndex)
{
    PhotoExtra* pPhotoExtra = GetExtraByItemIndexInner(iItemIndex);
	PhotoExtra retItem = {};
    if(NULL != pPhotoExtra)
    {
        retItem.Clone(pPhotoExtra);
    }

    return retItem;
}
PhotoExtra* CPhotoManager::GetExtraByItemIndexInner(int iItemIndex)
{
	map<int, PhotoExtra*>::iterator itor = m_PhotoExtraMap.find(iItemIndex);
	if (itor == m_PhotoExtraMap.end())
	{
		itor = m_IgnorePhotoExtraMap.find(iItemIndex);
		if (itor == m_IgnorePhotoExtraMap.end())
		{
			return NULL;
		}
	}
	PhotoExtra* pExtra = itor->second;
	return pExtra;
}
int CPhotoManager::GetDuration(int iItemIndex)
{
	return GetExtraByItemIndexInner(iItemIndex)->iDuration;
}
void CPhotoManager::AddIgnoreItem(int iItemIndex)
{
	vector<PhotoItem*>::iterator itor = m_PhotosVector.begin();
	for (; itor != m_PhotosVector.end(); ++itor)
	{
		PhotoItem* pPhotoItem = *itor;
		if (iItemIndex == pPhotoItem->iItemIndex)
		{
			AddIgnoreItem(pPhotoItem);
			break;
		}
	}
}
void CPhotoManager::AddIgnoreItem(PhotoItem* pItem)
{
	m_IgnorePhotosVector.push_back(pItem);
	m_IgnorePhotoExtraMap.insert(pair<int, PhotoExtra*>(pItem->iItemIndex, m_PhotoExtraMap.find(pItem->iItemIndex)->second));

	vector<PhotoItem*>::iterator itor = m_PhotosVector.begin();
	for (; itor != m_PhotosVector.end(); ++itor)
	{
		PhotoItem* pTmpItem = *itor;
		if (pItem->iItemIndex == pTmpItem->iItemIndex)
		{
			m_PhotosVector.erase(itor);
			break;
		}
	}
	m_PhotoExtraMap.erase(pItem->iItemIndex);

	SaveIgnoreItems();
}
void CPhotoManager::RemoveIgnoreItem(int iItemIndex)
{
	vector<PhotoItem*>::iterator itor = m_IgnorePhotosVector.begin();
	for (; itor != m_IgnorePhotosVector.end(); ++itor)
	{
		PhotoItem* pPhotoItem = *itor;
		if (iItemIndex == pPhotoItem->iItemIndex)
		{
			RemoveIgnoreItem(pPhotoItem);
			break;
		}
	}
}
void CPhotoManager::RemoveIgnoreItem(PhotoItem* pItem)
{
	BOOL bAdd = FALSE;
	for (size_t i = 0; i < m_PhotosVector.size(); ++i)
	{
		if (m_PhotosVector[i]->iItemIndex > pItem->iItemIndex)
		{
			m_PhotosVector.insert(m_PhotosVector.begin() + i, pItem);
			bAdd = TRUE;
			break;
		}
	}
	if (FALSE == bAdd)
	{
		m_PhotosVector.push_back(pItem);
	}

	m_PhotoExtraMap.insert(pair<int, PhotoExtra*>(pItem->iItemIndex, m_IgnorePhotoExtraMap.find(pItem->iItemIndex)->second));

	m_IgnorePhotoExtraMap.erase(pItem->iItemIndex);
	vector<PhotoItem*>::iterator itor = m_IgnorePhotosVector.begin();
	for (; itor != m_IgnorePhotosVector.end(); ++itor)
	{
		PhotoItem* pTmpItem = *itor;
		if (pItem->iItemIndex == pTmpItem->iItemIndex)
		{
			m_IgnorePhotosVector.erase(itor);
			break;
		}
	}
	SaveIgnoreItems();
}
void  CPhotoManager::SaveIgnoreItems()
{
	string strIgnoreFile(m_strRoot);
	CFileUtil fileUtil;
	strIgnoreFile.append("/");
	strIgnoreFile.append("ignore.txt");
	string strIgnoreIds = "";
	std::map<int, PhotoExtra*>::iterator itor;
	for (itor = m_IgnorePhotoExtraMap.begin(); itor != m_IgnorePhotoExtraMap.end(); ++itor)
	{
		PhotoExtra* pPhotoExtra = itor->second;
		strIgnoreIds.append(pPhotoExtra->strFileName);
		strIgnoreIds.append("\n");
	}

	if (strIgnoreIds.length() > 0)
	{
		strIgnoreIds = strIgnoreIds.substr(0, strIgnoreIds.length() - 1);
	}
	if (strIgnoreIds.length() == 0)
	{
		CCommonUtil::RemoveFile(strIgnoreFile.c_str());
	}
	else
	{
		fileUtil.SetFileContent(strIgnoreFile.c_str(), strIgnoreIds.c_str());
	}
}
void CPhotoManager::FilterIgnore()
{
	string strIgnoreFile(m_strRoot);
	strIgnoreFile.append("/");
	strIgnoreFile.append("ignore.txt");
	if (FALSE == CCommonUtil::CheckFileExist(strIgnoreFile.c_str()))
	{
		return;
	}
	CFileUtil fileUtil;
	string strFileContent = fileUtil.GetFileContent(strIgnoreFile.c_str());

	std::vector<std::string> fileItemVec = CCommonUtil::StringSplit(strFileContent, "\n");
	std::vector<string> IgnoreFileList;
	for (size_t i = 0; i < fileItemVec.size(); ++i)
	{
		string strFile(m_strRoot);
		strFile.append(fileItemVec[i]);
		if (TRUE == CCommonUtil::CheckFileExist(strFile.c_str()))
		{
			IgnoreFileList.push_back(fileItemVec[i]);
		}
	}
	std::vector<string> fileNameVec;
	std::vector<int> itemIndexVec;
	for (size_t i = 0; i < m_PhotosVector.size(); ++i)
	{
		PhotoItem* pPhotoItem = m_PhotosVector[i];
		PhotoExtra* pPhotoExtra = m_PhotoExtraMap.find(pPhotoItem->iItemIndex)->second;
		fileNameVec.push_back(pPhotoExtra->strFileName);
		itemIndexVec.push_back(pPhotoItem->iItemIndex);
	}
	string strOutContent = "";
	for (size_t i = 0; i < fileNameVec.size(); ++i)
	{
		string strFileName = fileNameVec[i];
		int iItemIndex = itemIndexVec[i];
		for (size_t j = 0; j < IgnoreFileList.size(); ++j)
		{
			if (0 == IgnoreFileList[j].compare(strFileName))
			{
				AddIgnoreItem(GetItemByItemIndexInner(iItemIndex));
				strOutContent += strFileName;
				strOutContent += "\n";
			}
		}
	}
	if (strOutContent.length() > 0)
	{
		strOutContent = strOutContent.substr(0, strOutContent.length() - 1);
	}
	if (strOutContent.length() == 0)
	{
		CCommonUtil::RemoveFile(strIgnoreFile.c_str());
	}
	else
	{
		fileUtil.SetFileContent(strIgnoreFile.c_str(), strOutContent.c_str());
	}
}
BOOL CPhotoManager::IsIgnore(int iItemIndex)
{
	map<int, PhotoExtra*>::iterator itor = m_IgnorePhotoExtraMap.find(iItemIndex);
	if (itor == m_IgnorePhotoExtraMap.end())
	{
		return FALSE;
	}
	return TRUE;
}

int CPhotoManager::GetNextVisiableIndex(int iCurItemIndex, BOOL bIgnoreGroup)
{
	int iIndex = -1;

	if (FALSE == bIgnoreGroup)
	{
		for (size_t i = 0; i < m_PhotosVector.size(); ++i)
		{
			int iItemIndex = m_PhotosVector[i]->iItemIndex;
			if (m_PhotosVector[i]->iItemIndex > iCurItemIndex)
			{
				iIndex = iItemIndex;
				break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < m_IgnorePhotosVector.size(); ++i)
		{
			int iItemIndex = m_IgnorePhotosVector[i]->iItemIndex;
			if (m_IgnorePhotosVector[i]->iItemIndex > iCurItemIndex)
			{
				iIndex = iItemIndex;
				break;
			}
		}
		
	}
	
	return iIndex;
}
int CPhotoManager::GetPrevVisiableIndex(int iCurIndex, BOOL bIgnoreGroup)
{
	int iIndex = -1;
	if (FALSE == bIgnoreGroup)
	{
		for (int i = m_PhotosVector.size() - 1; i >= 0; --i)
		{
			int iItemIndex = m_PhotosVector[i]->iItemIndex;
			if (m_PhotosVector[i]->iItemIndex < iCurIndex)
			{
				iIndex = iItemIndex;
				break;
			}
		}
	}
	else
	{
		for (int i = m_IgnorePhotosVector.size() - 1; i >= 0; --i)
		{
			int iItemIndex = m_IgnorePhotosVector[i]->iItemIndex;
			if (m_IgnorePhotosVector[i]->iItemIndex < iCurIndex)
			{
				iIndex = iItemIndex;
				break;
			}
		}
	}
	return iIndex;
}
void CPhotoManager::Print()
{
	// EnterCriticalSection(&m_PhotosVectorSection);
	// for(size_t i = 0; i < m_PhotosVector.size(); ++i)
	// {
	// 	PhotoItem* pPhotoItem = m_PhotosVector[i];
	// 	pPhotoItem->Print();
	// 	PhotoExtra* pPhotoExtra = m_PhotoExtraMap.find(pPhotoItem->iItemIndex)->second;
	// 	pPhotoExtra->Print();
	// }
	// LeaveCriticalSection(&m_PhotosVectorSection);
}
PhotoItem CPhotoManager::GetItemByItemIndex(int iItemIndex)
{
    PhotoItem* pPhotoItem = GetItemByItemIndexInner(iItemIndex);
    PhotoItem retItem = {};
    if(NULL != pPhotoItem)
    {
        retItem.Clone(pPhotoItem);
    }

    return retItem;
}
BOOL CPhotoManager::GenThumbPic(string strOrginalFile, string strThumbFile)
{
	//源文件在不在
	if (FALSE == CCommonUtil::CheckFileExist(strOrginalFile.c_str()))
	{
		return FALSE;
	}
	string strDestPath = CCommonUtil::GetPath(strThumbFile.c_str());
	if(FALSE == CCommonUtil::CheckFoldExist(strDestPath.c_str()))
	{
		CCommonUtil::CreateFold(strDestPath.c_str());
	}
	string strMimeType = CCommonUtil::GetMime(strOrginalFile.c_str());
	printf("MimeType: %s\n", strMimeType.c_str());
	if (TRUE == CCommonUtil::IsMimeTypeImage(strMimeType.c_str()))
	{
		CImageParse imageParse;
		BOOL bSuccess = imageParse.GetThumbnail(strOrginalFile.c_str(), strThumbFile.c_str(), PIC_WIDTH, PIC_HEIGHT);
		return bSuccess;
	}
	else if (TRUE == CCommonUtil::IsMimeTypeVideo(strMimeType.c_str()))
	{
		CVideoParse videoParse;
		BOOL bSuccess = videoParse.GetThumbnail(strOrginalFile.c_str(), strThumbFile.c_str(), PIC_WIDTH, PIC_HEIGHT);
		return bSuccess;
	}
	return FALSE;
}