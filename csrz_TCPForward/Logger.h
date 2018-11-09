#pragma once
#include "util.h"
#include <cstdio>

using namespace std;

/**
 * 类名：Logger
 * 作用：提供写日志功能，支持多线程，支持可变形参数操作，支持写日志级别的设置
 * 接口：SetLogLevel：  设置写日志级别
 *       TraceKeyInfo： 忽略日志级别，写关键信息
 *       TraceError：   写错误信息
 *       TraceWarning： 写警告信息
 *       TraceInfo：    写一般信息
 *
 * 使用：
 * #include "Logger.h"
 * Logger log;
 * log.SetLogLevel(LogLevelAll);
 * log.TraceInfo("Ready Go");
 * log.TraceKeyInfo("KeyInfo");
 * log.TraceWarning("Warning");
 * log.TraceError("Error");
**/

//日志级别的提示信息
static const char * FATALPREFIX = " [Fatal]: ";
static const char * ERRORPREFIX = " [Error]: ";
static const char * WARNINGPREFIX = " [Warning]: ";
static const char * INFOPREFIX = " [Info]: ";

static const int MAX_STR_LEN = 1024;
//日志级别枚举
enum EnumLogLevel
{
	LogLevelAll = 0,    //所有信息都写日志
	LogLevelMid,        //写错误、警告信息
	LogLevelNormal,     //只写错误信息
	LogLevelStop        //不写日志
};

class Logger
{
public:
	//默认构造函数
	Logger();
	//构造函数
	Logger(const char * strLogPath, const int iMaxFileQty = 0, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal);
	//析构函数
	virtual ~Logger();
public:
	//写关键信息
	void TraceFatal(const char * strInfo, ...);
	//写错误信息
	void TraceError(const char* strInfo, ...);
	//写警告信息
	void TraceWarning(const char * strInfo, ...);
	//写一般信息
	void TraceInfo(const char * strInfo, ...);
	//设置写日志级别
	void SetLogLevel(EnumLogLevel nLevel);
	// 设置日志文件最大数量，0为不限制数量
	void SetMaxFileQty(const int iMaxFileQty);
	// 输出日志带文件名, 行号, 函数名
#define TRACE_FATAL(fmt,...) TraceFatal("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_ERR(fmt,...) TraceError("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_WARN(fmt,...) TraceWarning("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_INFO(fmt,...) TraceInfo("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
private:
	//写文件操作
	void Trace(const char * strInfo);
	//获取当前系统时间
	void GetDateTime(char *pTemp, int length);
	//创建日志文件名称
	void GenerateLogName();
	//创建日志路径
	void CreateLogPath();
	// 关闭文件流
	void CloseFile();
	// 控制文件数量
	void UpdateFileQty();
private:
	//写日志文件流
	FILE * m_pFileStream;
	//写日志级别
	EnumLogLevel m_nLogLevel;
	//日志的路径
	char m_strLogPath[MAX_STR_LEN];
	//日志的名称
	char m_strCurLogName[MAX_STR_LEN];
	//线程同步的临界区变量
	CRITICAL_SECTION m_cs;
	// 日志目录内的文件数量最大值
	int m_iMaxFileQty;
};