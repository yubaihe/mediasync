#ifndef VIDEOMETAPARSE_H__
#define VIDEOMETAPARSE_H__
#include "stdafx.h"

class CVideoMetaParse
{
public:
	CVideoMetaParse();
	BOOL ParseVideoMeta(string strFile);
	uint64_t GetCreateTime();
	vector<string> GetLocaiton();
	int GetDuration();
	int GetWidth();
	int GetHeight();
	void OUt();
	void OUtGps();
private:
	std::string m_strLong;
	string m_strLat;
	uint64_t m_iCreateTime;
	int m_iDuration;
	int m_iWidth;
	int m_iHeight;
	string m_strFile;
};

#endif