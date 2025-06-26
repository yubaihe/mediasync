#include "BackupOrganize.h"
#include "BackupManager.h"
#include "../Util/CommonUtil.h"
#include "../Util/FileUtil.h"
CBackupOrganize* CBackupOrganize::m_pInstance = NULL;
CBackupOrganize::CBackupOrganize()
{
    m_eOrganizeState = ORGANIZESTATE_NONE;
    m_hOrganize = NULL;
    m_bExit = TRUE;
}

CBackupOrganize::~CBackupOrganize()
{
    m_bExit = TRUE;
    if(NULL != m_hOrganize)
    {
        WaitForSingleObject(m_hOrganize, INFINITE);
        CloseHandle(m_hOrganize);
        m_hOrganize = NULL;
    }
}
CBackupOrganize* CBackupOrganize::GetInstance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CBackupOrganize();
    }
    return m_pInstance;
}
void CBackupOrganize::Release()
{
    if(NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}
BOOL CBackupOrganize::Start()
{
    if(FALSE == CBackupManager::GetInstance()->IsSupportBackup())
    {
        return TRUE;
    }
    if(ORGANIZESTATE_RUNNING == m_eOrganizeState)
    {
        return TRUE;
    }
    if(NULL != m_hOrganize)
    {
        WaitForSingleObject(m_hOrganize, INFINITE);
        CloseHandle(m_hOrganize);
        m_hOrganize = NULL;
    }
    m_bExit = FALSE;
    m_iPrecent = 0;
    m_hOrganize = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)BackupOrganizeProc, this, 0, NULL);
    return TRUE;
}
void CBackupOrganize::RemoveFile(string strFile, string strReason)
{
    printf("======Notification Remove File=======>%s\n", strReason.c_str());
    CFileUtil::RemoveFile(strFile.c_str());
}
DWORD CBackupOrganize::BackupOrganizeProc(void* lpParameter)
{
    CBackupOrganize* pBackupOrganize = (CBackupOrganize*)lpParameter;
    std::vector<string> foldVec = CBackupManager::GetInstance()->BackupFoldList();
    std::map<string, time_t> thumbFoldMap = CBackupManager::GetInstance()->BackupThumbFoldMap();
    //将不存在的表删除掉
    CBackupTable table;
    table.RemoveFoldNotIn(foldVec);

    pBackupOrganize->m_iBackupFoldCount = foldVec.size();
    printf("Fold count:%d\n", pBackupOrganize->m_iBackupFoldCount);
    for(size_t i = 0; i < foldVec.size(); ++i)
    {
        if(TRUE == pBackupOrganize->m_bExit)
        {
            break;
        }
        std::map<string, time_t>::iterator itor = thumbFoldMap.find(foldVec[i]);
        if(itor != thumbFoldMap.end())
        {
            thumbFoldMap.erase(itor);
        }
        pBackupOrganize->m_iBackupFoldCurrent = i;
        pBackupOrganize->DealOneFold(foldVec[i]);
    }
    std::map<string, time_t>::iterator itor = thumbFoldMap.begin();
    for(; itor != thumbFoldMap.end(); ++itor)
    {
        string strFoldThumbRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(itor->first);
        CCommonUtil::RemoveFold(strFoldThumbRoot.c_str());
    }
    printf("Organize finish\n");
    pBackupOrganize->m_iPrecent = 100;
    return 1;
}
void CBackupOrganize::OnInitPhotoCallBack(int iPrecent, LPVOID* lpParameter)
{
    CBackupOrganize* pBackupOrganize = (CBackupOrganize*)lpParameter;
    pBackupOrganize->SetProcess((int)(20+((iPrecent*60.0f)/100)));
}
int CBackupOrganize::GetProcess()
{
    if(FALSE == CBackupManager::GetInstance()->IsSupportBackup())
    {
        return 100;
    }
    return m_iPrecent;
}
void CBackupOrganize::SetProcess(int iPrecent)
{
    int iBasePrecent = m_iBackupFoldCurrent/(m_iBackupFoldCount*1.0f)*100;
    int iCurrentPrecent = iBasePrecent + 100/(m_iBackupFoldCount*1.0f)*(iPrecent/100.0f);
    printf("m_iBackupFoldCurrent:%d\tm_iBackupFoldCount:%d\tiBasePrecent:%d\tiPrecent:%d\n", m_iBackupFoldCurrent, m_iBackupFoldCount, iBasePrecent, iPrecent);
    printf("Short process:%d\n", iPrecent);
    printf("big process:%d\n", iCurrentPrecent);
    m_iPrecent = iCurrentPrecent;
}
void CBackupOrganize::DealOneFold(string strFoldName)
{
    string strFoldRoot = CBackupManager::GetInstance()->GetBackupRoot(strFoldName);
    string strFoldThumbRoot = CBackupManager::GetInstance()->GetBackupThumbRoot(strFoldName);
    CCommonUtil::CreateFold(strFoldRoot.c_str());
    CCommonUtil::CreateFold(strFoldThumbRoot.c_str());
    map<int, BackupItem> tableMap = BackupFileMap(strFoldName);//key:id
    CBackupFlod backupfold(strFoldName);
    std::vector<FileEntry> originalFileVec = backupfold.AllFilesVec(strFoldRoot);
    std::map<string, time_t> thumbFileMap = backupfold.AllFilesMap(strFoldThumbRoot);
    printf("tableMap:%ld\n", tableMap.size());
    printf("originalFileVec:%ld\n", originalFileVec.size());
    printf("thumbFileMap:%ld\n", thumbFileMap.size());
    vector<string> needSyncVec;
    for(size_t i = 0; i < originalFileVec.size(); ++i)
    {
        if(TRUE == m_bExit)
        {
            break;
        }
        SetProcess((int)((i*10.0f)/originalFileVec.size()));
        FileEntry entry = originalFileVec[i];
        string strFile = CCommonUtil::StringFormat("%s%s", strFoldRoot.c_str(), entry.strName.c_str());
        string strFileMd5 = CFileUtil::GetFileMd51(strFile.c_str(), 2);
        CBackupTable table;
        BackupItemFull tableItem = table.GetItemFromMd5(strFoldName, strFileMd5);
        printf("===========%s=========\n", strFile.c_str());
        if(tableItem.iID == -1)
        {
            //数据库里面没有这个文件的记录 应该是新加进来的 需要同步
            printf("need sync => Can not find file from db table %s\n", strFile.c_str());
            needSyncVec.push_back(entry.strName);
        }
        else
        {
            //有记录 判断记录对应的缩略图在不在？
            printf("find record from db table id:%d\n", tableItem.iID);
            string strOriginalFile = CCommonUtil::StringFormat("%s%s", strFoldRoot.c_str(), tableItem.strFile.c_str());
            string strThumbFileName = backupfold.GetThumbFile(tableItem.eMediaType, tableItem.strFile);
            string strThumbFile = CCommonUtil::StringFormat("%s%s", strFoldThumbRoot.c_str(), strThumbFileName.c_str());
            std::map<string, time_t>::iterator itorThumb = thumbFileMap.find(strThumbFileName);
            if(thumbFileMap.end() == itorThumb)
            {
                //缩略图不在 把数据库记录删除掉 让他重新生成
                printf("need sync => thumb not equal:%s  %s\n", entry.strName.c_str(), strThumbFileName.c_str());
                needSyncVec.push_back(entry.strName);
                table.RemoveItem(tableItem.iID);
            }
            else
            {
                //缩略图在 源文件呢？
                if(FALSE == CFileUtil::CheckFileExist(strOriginalFile.c_str()))
                {
                    //源文件不存在 可能是源文件改名了 把数据库和缩略图全部删除掉就作为新的记录吧
                    printf("need sync => Original file not exist %s\n", strOriginalFile.c_str());
                    needSyncVec.push_back(entry.strName);
                    printf("Remove table record %d\n", tableItem.iID);
                    table.RemoveItem(tableItem.iID);
                    printf("Remove thumb file %s\n", strThumbFile.c_str());
                    string strReason = CCommonUtil::StringFormat("Original file not exist %s remove thumbfile:%s\n", strOriginalFile.c_str(), strThumbFile.c_str());
                    RemoveFile(strThumbFile, strReason);
                }
                else
                {
                    string strTbFileMd5 = CFileUtil::GetFileMd51(strOriginalFile.c_str(), 3);
                    if(0 != strTbFileMd5.compare(strFileMd5))
                    {
                        printf("need sync => record md5 not equal %s  => %s\n", strOriginalFile.c_str(), strFile.c_str());
                        needSyncVec.push_back(entry.strName);
                        printf("Remove table record %d\n", tableItem.iID);
                        table.RemoveItem(tableItem.iID);
                        printf("Remove thumb file %s\n", strThumbFile.c_str());
                        string strReason = CCommonUtil::StringFormat("record md5 not equal %s remove thumbfile:%s\n", strOriginalFile.c_str(), strThumbFile.c_str());
                        RemoveFile(strThumbFile, strReason);
                    }
                    else
                    {
                        //数据库也找到了相同MD5的记录 数据库对应的源文件也存在 缩略图也在 那么判断这个文件名是不是复制品
                        //如果是复制品需要删除掉
                        if(0 != tableItem.strFile.compare(entry.strName))
                        {
                            //这个是一个复制品 删除掉
                            string strReason = CCommonUtil::StringFormat("a copy from %s remove file:%s\n", strOriginalFile.c_str(), strFile.c_str());
                            RemoveFile(strFile, strReason);
                        }
                        else
                        {
                            //就不需要同步了
                            //先删除缩略图列表的记录
                            thumbFileMap.erase(itorThumb);
                            //再删除数据库列表的记录
                            map<int, BackupItem>::iterator itor = tableMap.find(tableItem.iID);
                            if(itor != tableMap.end())
                            {
                                tableMap.erase(itor);
                            }
                        }
                    }
                }
            }
        }
    }
    //把数据库多余的删除掉
    map<int, BackupItem>::iterator itorTable = tableMap.begin();
    list<int> idList;
    for(; itorTable != tableMap.end(); ++itorTable)
    {
        idList.push_back(itorTable->first);
    }
    CBackupTable table;
    table.RemoveFromIDList(idList);
    //多余的缩略图文件也删除掉
    int iIndex = 0;
    std::map<string, time_t>::iterator itorThumb = thumbFileMap.begin();
    for(; itorThumb != thumbFileMap.end(); ++itorThumb)
    {
        iIndex++;
        SetProcess((int)(10+((iIndex*10.0f)/thumbFileMap.size())));
        string strFile = CCommonUtil::StringFormat("%s%s", strFoldThumbRoot.c_str(), itorThumb->first.c_str());
        string strReason = CCommonUtil::StringFormat("no used thumb file:%s\n", strFile.c_str());
        RemoveFile(strFile, strReason);
    }
    printf("needSyncVec:%ld\n", needSyncVec.size());
    if(needSyncVec.size() == 0)
    {
        SetProcess(100);
        return;
    }
    printf("============needSyncVec start=====================\n");
    for(size_t i = 0; i < needSyncVec.size(); ++i)
    {
        printf("%s\n", needSyncVec[i].c_str());
    }
    printf("============needSyncVec end=====================\n");
    CPhotoManager photoManager;
    list<int> retList = photoManager.Init(needSyncVec, strFoldRoot, strFoldThumbRoot, OnInitPhotoCallBack,  (LPVOID*)this, TRUE);
    printf("over\n");
    list<int>::iterator itor = retList.begin();
    iIndex = 0;
    for(; itor != retList.end(); ++itor)
    {
        if(TRUE == m_bExit)
        {
            break;
        }
        iIndex++;
        SetProcess((int)(80+((iIndex*19.0f)/retList.size())));
        
        PhotoItem item = photoManager.GetItemByItemIndex(*itor);
        PhotoExtra extra = photoManager.GetExtraByItemIndex(*itor);
        string strFilePrefx("");
        string strPostfix("");
        BOOL bRet = CCommonUtil::SpliteFile(item.strName, strFilePrefx, strPostfix);
        if(FALSE == bRet)
        {
            if(item.iMediaType == MEDIATYPE_IMAGE)
            {
                strPostfix = "jpg";
            }
            else
            {
                strPostfix = "mp4";
            }
            string strOriginalFileName = CCommonUtil::StringFormat("%s%s.%s", strFoldRoot.c_str(), item.strName.c_str());
            string strDestFileName = CCommonUtil::StringFormat("%s%s.%s", strFoldRoot.c_str(), item.strName.c_str(), strPostfix.c_str());
            printf("11==>%s=====>%s\n", strOriginalFileName.c_str(), strDestFileName.c_str());
            CCommonUtil::MoveFile(strOriginalFileName, strDestFileName);
            item.strName = CCommonUtil::StringFormat("%s.%s", item.strName.c_str(), strPostfix.c_str());
        }

        string strThumbFrom = CCommonUtil::StringFormat("%s%s.jpg", strFoldThumbRoot.c_str(), strFilePrefx.c_str());
        string strThumbFileName = backupfold.GetThumbFile((MEDIATYPE)item.iMediaType, item.strName);
        string strThumbTo = CCommonUtil::StringFormat("%s%s", strFoldThumbRoot.c_str(), strThumbFileName.c_str());
        printf("%s=====>%s\n", strThumbFrom.c_str(), strThumbTo.c_str());
        CCommonUtil::MoveFile(strThumbFrom, strThumbTo);

        string strFileName = CCommonUtil::StringFormat("%s%s", strFoldRoot.c_str(), item.strName.c_str());
        time_t iCreateTimeSec = 0;
        struct stat filestat;
        if (stat(strFileName.c_str(), &filestat) == 0) 
        {
            iCreateTimeSec = filestat.st_ctime;
        }
        else
        {
            iCreateTimeSec = CCommonUtil::CurTimeSec();
        }
        string strMd5 = CFileUtil::GetFileMd51(strFileName.c_str(), 10);
        BackupItemFull record = {};
        record.strFile = item.strName;
        record.strMd5 = strMd5;
        record.iCreateTimeSec = iCreateTimeSec;
        record.eMediaType = (MEDIATYPE)item.iMediaType;
        record.strAddr = CCommonUtil::StringFormat("%s&%s", extra.vecLocation[0].c_str(), extra.vecLocation[1].c_str());
        record.strLocation = "";
        record.iMeiTiSize = item.iFileSize;
        record.strFoldName = strFoldName;
        record.iWidth = item.iWidth;
        record.iHeight = item.iHeight;
        record.iDuration = extra.iDuration;
        record.iHasExtra = 0;
        CBackupTable table;
        table.AddItem(record);
    }
    SetProcess(100);
}
map<int, BackupItem> CBackupOrganize::BackupFileMap(string strFoldName)
{
    CBackupTable table;
    vector<BackupItem> vec = table.BackupFileListShort(strFoldName, NULL, NULL);
    map<int, BackupItem> retMap;
    for(size_t i = 0; i < vec.size(); ++i)
    {
        retMap.insert(pair<int, BackupItem>(vec[i].iID, vec[i]));
    }
    return retMap;
}