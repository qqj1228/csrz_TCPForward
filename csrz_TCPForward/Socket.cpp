#include "pch.h"
#include "Socket.h"
#include <iostream>

int Socket_Base::nofSockets_ = 0;

void Socket_Base::Start() {
	if (!nofSockets_) {
		WSADATA info;
		if (WSAStartup(MAKEWORD(2, 2), &info)) {
			throw string("Could not start WSA");
		}
	}
	++nofSockets_;
}

void Socket_Base::End() {
	WSACleanup();
}

Socket_Base::Socket_Base(const bool bTCP) : s_(0), bTCP(bTCP) {
	Start();
	if (bTCP) {
		// Use TCP
		s_ = socket(AF_INET, SOCK_STREAM, 0);
	} else {
		// Use UDP
		s_ = socket(AF_INET, SOCK_DGRAM, 0);
	}

	if (s_ == INVALID_SOCKET) {
		throw string("INVALID_SOCKET");
	}

	refCounter_ = new int(1);
}

Socket_Base::Socket_Base(SOCKET s, const bool bTCP) : s_(s), bTCP(bTCP) {
	Start();
	refCounter_ = new int(1);
};

Socket_Base::~Socket_Base() {
	if (!--(*refCounter_)) {
		Close();
		delete refCounter_;
	}

	--nofSockets_;
	if (!nofSockets_) End();
}

Socket_Base::Socket_Base(const Socket_Base& o) {
	refCounter_ = o.refCounter_;
	(*refCounter_)++;
	s_ = o.s_;
	bTCP = o.bTCP;
	nofSockets_++;
}

Socket_Base& Socket_Base::operator=(Socket_Base& o) {
	(*o.refCounter_)++;

	refCounter_ = o.refCounter_;
	s_ = o.s_;
	bTCP = o.bTCP;
	nofSockets_++;

	return *this;
}

void Socket_Base::Close() {
	closesocket(s_);
}

string Socket_Base::showWSError() {
	string err;
	switch (WSAGetLastError()) {
	case WSANOTINITIALISED:
		err = "A successful WSAStartup call must occur before using this function. ";
		break;
	case WSAENETDOWN:
		err = "The network subsystem has failed. ";
		break;
	case WSAEACCES:
		err = "The requested address is a broadcast address, but the appropriate flag was not set. Call setsockopt with the SO_BROADCAST parameter to allow the use of the broadcast address. ";
		break;
	case WSAEINVAL:
		err = "An unknown flag was specified, or MSG_OOB was specified for a socket with SO_OOBINLINE enabled. ";
		break;
	case WSAEINTR:
		err = "A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall. ";
		break;
	case WSAEINPROGRESS:
		err = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function. ";
		break;
	case WSAEFAULT:
		err = "The buf or to parameters are not part of the user address space, or the tolen parameter is too small. ";
		break;
	case WSAENETRESET:
		err = "The connection has been broken due to keep-alive activity detecting a failure while the operation was in progress. ";
		break;
	case WSAENOBUFS:
		err = "No buffer space is available. ";
		break;
	case WSAENOTCONN:
		err = "The socket is not connected (connection-oriented sockets only). ";
		break;
	case WSAENOTSOCK:
		err = "The descriptor is not a socket. ";
		break;
	case WSAEOPNOTSUPP:
		err = "MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only receive operations. ";
		break;
	case WSAESHUTDOWN:
		err = "The socket has been shut down; it is not possible to sendto on a socket after shutdown has been invoked with how set to SD_SEND or SD_BOTH. ";
		break;
	case WSAEWOULDBLOCK:
		err = "The socket is marked as nonblocking and the requested operation would block. ";
		break;
	case WSAEMSGSIZE:
		err = "The socket is message oriented, and the message is larger than the maximum supported by the underlying transport. ";
		break;
	case WSAEHOSTUNREACH:
		err = "The remote host cannot be reached from this host at this time. ";
		break;
	case WSAECONNABORTED:
		err = "The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable. ";
		break;
	case WSAECONNRESET:
		err = "The virtual circuit was reset by the remote side executing a hard or abortive close. For UPD sockets, the remote host was unable to deliver a previously sent UDP datagram and responded with a \"Port Unreachable\" ICMP packet. The application should close the socket as it is no longer usable. ";
		break;
	case WSAEADDRNOTAVAIL:
		err = "The remote address is not a valid address, for example, ADDR_ANY. ";
		break;
	case WSAEAFNOSUPPORT:
		err = "Addresses in the specified family cannot be used with this socket. ";
		break;
	case WSAEDESTADDRREQ:
		err = "A destination address is required. ";
		break;
	case WSAENETUNREACH:
		err = "The network cannot be reached from this host at this time. ";
		break;
	case WSAETIMEDOUT:
		err = "The connection has been dropped, because of a network failure or because the system on the other end went down without notice. ";
		break;
	default:
		err = "Unknown socket error. ";
		break;
	}
	return err;
}

bool Socket_Base::IsTCP() {
	return bTCP;
}

// 接收二进制数据，没有结束标志，接收缓冲区中无数据时返回结果
vector<char> Socket_Base::ReceiveBytes() {
	vector<char> result;
	if (bTCP) {
		// TCP协议下，数据包是无序的，可以分步读取缓冲区中的数据，直至全部读取完
		char buf[1024];
		while (1) {
			u_long arg = 0;
			if (ioctlsocket(s_, FIONREAD, &arg) != 0)
				break;

			if (arg == 0)
				break;

			if (arg > 1024) arg = 1024;

			int rv = recv(s_, buf, arg, 0);
			if (rv <= 0) break;

			for (u_long i = 0; i < arg; i++) {
				result.push_back(buf[i]);
			}
		}
	} else {
		// UDP协议下，每次接收缓冲区中一帧完整数据，不能分步接收
		SOCKADDR from;
		int len = sizeof(from);
		u_long arg = 0;
		if (ioctlsocket(s_, FIONREAD, &arg) != 0)
			return result;
		if (arg == 0)
			return result;
		char *pBuf = new char[arg];
		int rv = recvfrom(s_, pBuf, arg, 0, &from, &len);
		if (rv <= 0) {
			delete[] pBuf;
			pBuf = nullptr;
			return result;
		}
		for (u_long i = 0; i < arg; i++) {
			result.push_back(pBuf[i]);
		}
		delete[] pBuf;
		pBuf = nullptr;
	}
	return result;
}

// 接收以\r\n或者\n结尾的字符串, 返回的字符串含有一个\r\n或者\n
string Socket_Base::ReceiveLine() {
	string ret = "";
	if (bTCP) {
		// TCP协议下，数据包是无序的，可以分步读取缓冲区中的数据，直至"换行"符为止
		while (1) {
			char r;
			switch (recv(s_, &r, 1, 0)) {
			case 0: // not connected anymore;
					// ... but last line sent
					// might not end in \n,
					// so return ret anyway.
				break;
			case -1:
				ret = "";
				//      if (errno == EAGAIN) {
				//        return ret;
				//      } else {
				//      // not connected anymore
				//      return "";
				//      }
				break;
			}
			ret += r;
			if (r == '\n') break;
		}
	} else {
		// UDP协议下，每次接收缓冲区中一帧完整数据，直到收到含有"换行"符的数据
		int len = sizeof(UDPFrom_);
		while (true) {
			u_long arg = 0;
			if (ioctlsocket(s_, FIONREAD, &arg) != 0) {
				throw showWSError();
				break;
			}
			if (arg == 0)
				continue;
			char *pBuf = new char[arg + 1];
			int rv = recvfrom(s_, pBuf, arg, 0, &UDPFrom_, &len);
			switch (rv) {
			case 0:
				break;
			case -1:
				//ret = "";
				throw showWSError();
				break;
			}
			pBuf[rv] = '\x00';
			ret += pBuf;
			size_t pos = -1;
			size_t offset = 0;
			pos = ret.find("\x0A", offset);
			offset = ret.length();
			delete[] pBuf;
			pBuf = nullptr;
			if (pos != string::npos) {
				ret.resize(pos + 1);
				break;
			}
		}
	}
	return ret;
}

// strDestIP, iDestPort用于UDP
void Socket_Base::SendLine(string s, string strDestIP, int iDestPort) {
	s += "\r\n";
	if (bTCP) {
		send(s_, s.c_str(), s.length(), 0);
	} else {
		if (strDestIP == "") {
			sendto(s_, s.c_str(), s.length(), 0, &UDPFrom_, sizeof(UDPFrom_));
		} else {
			sockaddr_in destAddr;
			memset(&destAddr, 0, sizeof(destAddr));
			destAddr.sin_family = PF_INET;
			destAddr.sin_addr.s_addr = inet_addr(strDestIP.c_str());
			destAddr.sin_port = htons(iDestPort);
			sendto(s_, s.c_str(), s.length(), 0, (sockaddr*)&destAddr, sizeof(destAddr));
		}
	}
}

// strDestIP, iDestPort用于UDP
void Socket_Base::SendBytes(const char *pBuf, int size, string strDestIP, int iDestPort) {
	if (bTCP) {
		send(s_, pBuf, size, 0);
	} else {
		if (strDestIP == "") {
			sendto(s_, pBuf, size, 0, &UDPFrom_, sizeof(UDPFrom_));
		} else {
			sockaddr_in destAddr;
			memset(&destAddr, 0, sizeof(destAddr));
			destAddr.sin_family = PF_INET;
			destAddr.sin_addr.s_addr = inet_addr(strDestIP.c_str());
			destAddr.sin_port = htons(iDestPort);
			sendto(s_, pBuf, size, 0, (sockaddr*)&destAddr, sizeof(destAddr));
		}
	}
}

SocketServer::SocketServer(int port, const bool bTCP, int connections, TypeSocket type) {
	this->bTCP = bTCP;

	sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = PF_INET;
	sa.sin_port = htons(port);
	if (bTCP) {
		s_ = socket(AF_INET, SOCK_STREAM, 0);
	} else {
		s_ = socket(AF_INET, SOCK_DGRAM, 0);
	}
	if (s_ == INVALID_SOCKET) {
		throw string("INVALID_SOCKET");
	}

	if (type == NonBlockingSocket) {
		u_long arg = 1;
		ioctlsocket(s_, FIONBIO, &arg);
	}

	/* bind the socket to the internet address */
	if (bind(s_, (sockaddr *)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR) {
		closesocket(s_);
		throw string("INVALID_SOCKET");
	}

	if (bTCP) {
		listen(s_, connections);
	}
}

Socket_Base* SocketServer::Accept() {
	Socket_Base* r;

	if (bTCP) {
		SOCKET new_sock = accept(s_, 0, 0);
		if (new_sock == INVALID_SOCKET) {
			int rc = WSAGetLastError();
			if (rc == WSAEWOULDBLOCK) {
				// non-blocking call, no request pending
				return 0;
			} else {
				throw string("Invalid Socket");
			}
		}
		r = new Socket_Base(new_sock, bTCP);
	} else {
		r = new Socket_Base(s_, bTCP);
	}

	return r;
}

SocketClient::SocketClient(const string& host, int port) : Socket_Base() {
	string error;
	char buf[BUFSIZ];

	hostent *he;
	if ((he = gethostbyname(host.c_str())) == 0) {
		strerror_s(buf, BUFSIZ, errno);
		error = buf;
		throw error;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((in_addr *)he->h_addr);
	memset(&(addr.sin_zero), 0, 8);

	if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr))) {
		strerror_s(buf, BUFSIZ, WSAGetLastError());
		error = buf;
		throw error;
	}
}

SocketSelect::SocketSelect(Socket_Base const * const s1, Socket_Base const * const s2, TypeSocket type) {
	FD_ZERO(&fds_);
	FD_SET(const_cast<Socket_Base*>(s1)->s_, &fds_);
	if (s2) {
		FD_SET(const_cast<Socket_Base*>(s2)->s_, &fds_);
	}

	TIMEVAL tval;
	tval.tv_sec = 0;
	tval.tv_usec = 1;

	TIMEVAL *ptval;
	if (type == NonBlockingSocket) {
		ptval = &tval;
	} else {
		ptval = 0;
	}

	if (select(0, &fds_, (fd_set*)0, (fd_set*)0, ptval) == SOCKET_ERROR)
		throw string("Error in select");
}

bool SocketSelect::Readable(Socket_Base const* const s) {
	if (FD_ISSET(s->s_, &fds_)) return true;
	return false;
}
