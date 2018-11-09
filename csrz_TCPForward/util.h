#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>
#include <regex>
#include <io.h>
#include <direct.h>
#include <map>

using namespace std;

static string IntToString(int n)
{
	ostringstream stream;
	stream << n;
	return stream.str();
}

static string Normalize(string strSouce, size_t iMaxlen)
{
	string strResult;
	// 去除首尾空格
	strSouce.erase(0, strSouce.find_first_not_of(" "));
	strSouce.erase(strSouce.find_last_not_of(" ") + 1);
	// 将非法字符替换为_
	try
	{
		regex rgx("[/\\ _]+");
		string strFmt("_");
		strResult = regex_replace(strSouce, rgx, strFmt);
	}
	catch (regex_error &e)
	{
		cout << "regex error code: " << e.code() << endl;
	}
	// 截短到指定长度
	if (strResult.size() > iMaxlen)
	{
		strResult.resize(iMaxlen);
	}
	return strResult;
}

// 从左到右依次判断文件夹是否存在, 不存在就创建, 只适用于Windows平台
// example: D:/mkdir/1/2/3/4/
// 注意:最后一个如果是文件夹的话,需要加上 '\\' 或者 '/'
static int32_t mkMultiDir(const std::string &directoryPath)
{
	uint32_t dirPathLen = directoryPath.length();
	if (dirPathLen > MAX_PATH)
	{
		return -1;
	}
	char tmpDirPath[MAX_PATH] = { 0 };
	for (uint32_t i = 0; i < dirPathLen; ++i)
	{
		tmpDirPath[i] = directoryPath[i];
		if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
		{
			if (_access(tmpDirPath, 0) != 0)
			{
				int32_t ret = _mkdir(tmpDirPath);
				if (ret != 0)
				{
					return ret;
				}
			}
		}
	}
	return 0;
}

// 设置控制台文字前景和背景色, 返回原来的颜色属性
// 基本色：0x1-蓝，0x2-绿，0x4-红，0x8-加亮。
// 组合色：
// 0-黑		1-蓝		2-绿		3-青		4-红		5-紫		6-黄		7-白
// 8-灰		9-亮蓝	10-亮绿	11-亮青	12-亮红	13-亮紫	14-亮黄	15-亮白
static WORD setConsoleColor(int iForeColor, int iBackColor = 0) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		return 0;
	}
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	WORD wAttributes = info.wAttributes;
	bool bResult = SetConsoleTextAttribute(hConsole, iForeColor + iBackColor * 0x10);
	if (!bResult) {
		return 0;
	}
	return wAttributes;
}

// 设置控制台颜色属性, 返回原来的颜色属性
// 前景色组合：
// FOREGROUND_BLUE-0x0001	FOREGROUND_GREEN-0x0002		FOREGROUND_RED-0x0004	FOREGROUND_INTENSITY-0x0008
// 背景色组合（其实就是前景色*0x10）：
// BACKGROUND_BLUE-0x0010	BACKGROUND_GREEN-0x0020		BACKGROUND_RED-0x0040	BACKGROUND_INTENSITY-0x0080
static WORD setConsoleColor(WORD wAttributes) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hConsole == INVALID_HANDLE_VALUE) {
		return 0;
	}
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(hConsole, &info);
	WORD wOrigin = info.wAttributes;
	bool bResult = SetConsoleTextAttribute(hConsole, wAttributes);
	if (!bResult) {
		return 0;
	}
	return wOrigin;
}

// 删除指定目录内多余的文件，修改时间较旧的文件将会被删除
// strDir, 文件所在的目录
// iFileNum, 存放的文件数量
static void updateFileNum(string strDir, int iFileNum) {
	long handle;
	struct _finddata_t fileInfo;
	// multimap<修改时间，文件全路径>
	multimap<time_t, string> fileMap;
	string strFile = strDir + "/*";
	handle = _findfirst(strFile.c_str(), &fileInfo);
	if (handle != -1) {
		do {
			// 去除文件夹
			if (!(fileInfo.attrib & _A_SUBDIR)) {
				fileMap.insert(pair<time_t, string>(fileInfo.time_write, strDir + "/" + fileInfo.name));
			}
		} while (!_findnext(handle, &fileInfo));
		_findclose(handle);

#ifdef _DEBUG
		multimap<time_t, string>::iterator iter;
		multimap<time_t, string>::iterator end = fileMap.end();
		for (iter = fileMap.begin(); iter != end; ++iter) {
			cout << iter->first << " - " << iter->second << endl;
		}
		cout << "--------------------------------------" << endl;
#endif // _DEBUG

		if (iFileNum > 0) {
			int count = fileMap.size() - iFileNum;
			if (count > 0) {
				for (int i = 0; i < count; i++) {
					remove(fileMap.begin()->second.c_str());
					fileMap.erase(fileMap.begin());
				}
			}
		}
	} else {
		char err[BUFSIZ];
		_strerror_s(err, nullptr);
		WORD wOrigin = setConsoleColor(12, 14);
		cout << "_findfirst " << strDir << " error: " << err << endl;
		setConsoleColor(wOrigin);
	}
}
