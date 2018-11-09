#ifndef SOCKET_H
#define SOCKET_H

#include <WinSock2.h>
#include <vector>
#include <string>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

enum TypeSocket { BlockingSocket, NonBlockingSocket };

class Socket_Base {
public:

	virtual ~Socket_Base();
	Socket_Base(const bool bTCP = true);
	Socket_Base(const Socket_Base&);
	Socket_Base& operator=(Socket_Base&);

	void Close();
	string showWSError();
	bool IsTCP();

	string ReceiveLine();
	vector<char> ReceiveBytes();

	void SendLine(string s, string strDestIP = "", int iDestPort = 0);
	void SendBytes(const char *pBuf, int size, string strDestIP = "", int iDestPort = 0);

protected:
	friend class SocketServer;
	friend class SocketSelect;

	Socket_Base(SOCKET s, const bool bTCP = true);

	SOCKET s_;
	bool bTCP;
	SOCKADDR UDPFrom_;
	int* refCounter_;

private:
	static void Start();
	static void End();
	static int  nofSockets_;
};

class SocketServer : public Socket_Base {
public:
	SocketServer(int port, const bool bTCP = true, int connections = 10, TypeSocket type = BlockingSocket);

	Socket_Base* Accept();
};

class SocketClient : public Socket_Base {
public:
	SocketClient(const std::string& host, int port);
};

class SocketSelect {
public:
	SocketSelect(Socket_Base const * const s1, Socket_Base const * const s2 = NULL, TypeSocket type = BlockingSocket);

	bool Readable(Socket_Base const * const s);

private:
	fd_set fds_;
};



#endif
