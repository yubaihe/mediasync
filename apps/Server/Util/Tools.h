#pragma once
#include "../stdafx.h"
namespace Server
{
    class CTools
    {
        public:
        static long long CurTimeMilSec();
        static int GetYear(long iTimeSec);
        static BOOL GetDiskInfo(const char* pszAddr, size_t* iTotal, size_t* iUsed);
        static void SecInfo(long iSec, uint16_t* piYear, uint8_t* piMonth, uint8_t* piDay);
        static BOOL CheckGps(string strGps, GPSTYPE* piGpsType, float* pdLong, float* pdLat);
        static time_t CurTimeSec();
        static string ReplaceString(string strOriginal, string strOld, string strNew);
        static int Rfind(const char* psz, char sz);
        static string GetMacAddr();
        static string GetMd5(const char* psz);
        static BOOL HasRootPermission();
        static BOOL UpdateTimeSec(long iSec);
        static string GetSambaVersion();
        static string Trim(const std::string& str);
        static int GetRandomNum(int iMin, int iMax);
        template <typename T>
        static string VecToString(const std::vector<T>& vec, const std::string& strDelimiter)
        {
            if (TRUE == vec.empty())
            {
                return "";
            }
            std::string strResult = std::to_string(vec[0]);
            for (size_t i = 1; i < vec.size(); ++i)
            {
                strResult += strDelimiter + std::to_string(vec[i]);
            }
            return strResult;
        }
    };
}