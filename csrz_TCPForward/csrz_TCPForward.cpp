// csrz_TCPForward.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "Logger.h"
#include "Socket.h"
#include <iostream>

using namespace std;

class MyTCP {
public:
	Logger * lpLog = nullptr;
	SocketServer m_server;
	Socket_Base *lpClient = nullptr;
	SocketClient *m_lpClientShow = nullptr;
	vector<char> m_message;
	char m_result = '0';
	int m_localPort = 9876;
	int m_remotePort = 9876;
	string m_remoteIP = "127.0.0.1";

	MyTCP(Logger *lpLog, int localPort, string remoteIP, int remotePort, int connections = 10, TypeSocket type = BlockingSocket) :
		m_server(localPort, true, connections, type) {
		this->lpLog = lpLog;
		m_localPort = localPort;
		m_remotePort = remotePort;
		m_remoteIP = remoteIP;
		cout << "TCP server listening on port: " << localPort << endl;
		lpLog->TRACE_INFO("TCP server listening on port: %d", localPort);
	}

	~MyTCP() {
		// 断开客户端连接
		if (nullptr != lpClient) {
			delete lpClient;
			lpClient = nullptr;
		}
		if (nullptr != m_lpClientShow) {
			delete m_lpClientShow;
			m_lpClientShow = nullptr;
		}
	}

	void getData() {
		try {
			if (nullptr == lpClient) {
				// 接受客户端连接请求
				lpClient = m_server.Accept();
			}
			m_message = lpClient->ReceiveBytes();
		} catch (const string error) {
			cout << error << endl;
			lpLog->TRACE_ERR("%s", error.c_str());
		}
	}

	// 转发给内网的显示程序
	void sendData(const char *pBuf, int size) {
		try {
			m_lpClientShow = new SocketClient(m_remoteIP, m_remotePort);
			m_lpClientShow->SendBytes(pBuf, size);
			m_message.clear();
			while (m_message.size() == 0) {
				m_message = m_lpClientShow->ReceiveBytes();
			}
			m_result = m_message[0];
			// 将显示程序返回的结果转发给三菱远端
			lpClient->SendBytes(&m_result, 1);
			lpLog->TRACE_INFO("Response message: %c", m_result);
			m_lpClientShow->Close();
			delete m_lpClientShow;
			m_lpClientShow = nullptr;
		} catch (const string error) {
			cout << error << endl;
			lpLog->TRACE_ERR("%s", error.c_str());
		}
	}
};

int main()
{
	Logger log("./log", 100, LogLevelAll);
	string strCfgFile = "./config.ini";
	int localPort = 9876;
	string remoteIP = "127.0.0.1";
	int remotePort = 9876;
	char buf[BUFSIZ];

	localPort = GetPrivateProfileInt("TCP", "localPort", 9876, strCfgFile.c_str());
	remotePort = GetPrivateProfileInt("TCP", "remotePort", 9876, strCfgFile.c_str());
	GetPrivateProfileString("TCP", "remoteIP", "127.0.0.1", buf, BUFSIZ, strCfgFile.c_str());
	remoteIP = buf;

	MyTCP tcp(&log, localPort, remoteIP, remotePort);
	while (true) {
		// 接受三菱远端发来的消息
		tcp.getData();
		if (tcp.m_message.size() == 0) {
			continue;
		}
		char *pBuf = new char[tcp.m_message.size()];
		char *p = pBuf;
		for (auto &item : tcp.m_message) {
			cout << item;
			*p = item;
			p++;
		}
		cout << endl;
		// 转发到内网的显示程序
		tcp.sendData(pBuf, tcp.m_message.size());
		tcp.m_message.clear();
		delete[] pBuf;
		pBuf = nullptr;
	}
}
