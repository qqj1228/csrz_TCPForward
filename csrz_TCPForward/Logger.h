#pragma once
#include "util.h"
#include <cstdio>

using namespace std;

/**
 * ������Logger
 * ���ã��ṩд��־���ܣ�֧�ֶ��̣߳�֧�ֿɱ��β���������֧��д��־���������
 * �ӿڣ�SetLogLevel��  ����д��־����
 *       TraceKeyInfo�� ������־����д�ؼ���Ϣ
 *       TraceError��   д������Ϣ
 *       TraceWarning�� д������Ϣ
 *       TraceInfo��    дһ����Ϣ
 *
 * ʹ�ã�
 * #include "Logger.h"
 * Logger log;
 * log.SetLogLevel(LogLevelAll);
 * log.TraceInfo("Ready Go");
 * log.TraceKeyInfo("KeyInfo");
 * log.TraceWarning("Warning");
 * log.TraceError("Error");
**/

//��־�������ʾ��Ϣ
static const char * FATALPREFIX = " [Fatal]: ";
static const char * ERRORPREFIX = " [Error]: ";
static const char * WARNINGPREFIX = " [Warning]: ";
static const char * INFOPREFIX = " [Info]: ";

static const int MAX_STR_LEN = 1024;
//��־����ö��
enum EnumLogLevel
{
	LogLevelAll = 0,    //������Ϣ��д��־
	LogLevelMid,        //д���󡢾�����Ϣ
	LogLevelNormal,     //ֻд������Ϣ
	LogLevelStop        //��д��־
};

class Logger
{
public:
	//Ĭ�Ϲ��캯��
	Logger();
	//���캯��
	Logger(const char * strLogPath, const int iMaxFileQty = 0, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal);
	//��������
	virtual ~Logger();
public:
	//д�ؼ���Ϣ
	void TraceFatal(const char * strInfo, ...);
	//д������Ϣ
	void TraceError(const char* strInfo, ...);
	//д������Ϣ
	void TraceWarning(const char * strInfo, ...);
	//дһ����Ϣ
	void TraceInfo(const char * strInfo, ...);
	//����д��־����
	void SetLogLevel(EnumLogLevel nLevel);
	// ������־�ļ����������0Ϊ����������
	void SetMaxFileQty(const int iMaxFileQty);
	// �����־���ļ���, �к�, ������
#define TRACE_FATAL(fmt,...) TraceFatal("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_ERR(fmt,...) TraceError("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_WARN(fmt,...) TraceWarning("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TRACE_INFO(fmt,...) TraceInfo("%s(%d)<%s> "##fmt, strrchr(__FILE__,'\\')+1, __LINE__, __FUNCTION__, ##__VA_ARGS__)
private:
	//д�ļ�����
	void Trace(const char * strInfo);
	//��ȡ��ǰϵͳʱ��
	void GetDateTime(char *pTemp, int length);
	//������־�ļ�����
	void GenerateLogName();
	//������־·��
	void CreateLogPath();
	// �ر��ļ���
	void CloseFile();
	// �����ļ�����
	void UpdateFileQty();
private:
	//д��־�ļ���
	FILE * m_pFileStream;
	//д��־����
	EnumLogLevel m_nLogLevel;
	//��־��·��
	char m_strLogPath[MAX_STR_LEN];
	//��־������
	char m_strCurLogName[MAX_STR_LEN];
	//�߳�ͬ�����ٽ�������
	CRITICAL_SECTION m_cs;
	// ��־Ŀ¼�ڵ��ļ��������ֵ
	int m_iMaxFileQty;
};