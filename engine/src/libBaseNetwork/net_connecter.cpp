#include "stdafx.h"
#include "net_connecter.h"
#include "net_event_loop.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/profiling.h"

namespace base
{
	void CNetConnecter::printInfo(const char* szMsg)
	{
		if (nullptr == szMsg)
			return;

		PrintNW("%s connect state: %d close type：%d local addr: %s %d remote addr: %s %d socket_id: %d error code[%d] send_index: %d send_count: %d", szMsg, this->m_eConnecterState, this->m_nCloseType, this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID(), getLastError(), this->m_nSendConnecterIndex, this->m_pNetEventLoop->getSendConnecterCount());
	}

	void CNetConnecter::onEvent(uint32_t nEvent)
	{
		if (nEvent&eNET_Error)
		{
#ifdef _WIN32
			if (getLastError() != NW_EWOULDBLOCK && getLastError() != 0)
#else
			if (getLastError() != NW_EINPROGRESS && getLastError() != NW_EAGAIN && getLastError() != NW_EWOULDBLOCK)
#endif
			{
				this->shutdown(true, "socket eNET_Error");
				return;
			}

			int32_t err;
			socklen_t len = sizeof(err);
#ifdef _WIN32
			int32_t code = getsockopt(this->GetSocketID(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), (socklen_t*)&len);
#else
			int32_t code = getsockopt(this->GetSocketID(), SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&err), (socklen_t*)&len);
#endif
			if (code < 0 || err != 0)
			{
				this->shutdown(true, "eNET_Error and SO_ERROR");
				return;
			}
		}
		if (eNCS_Connecting == this->m_eConnecterState)
		{
			if (eNET_Send&nEvent)
				this->onConnect();
		}
		else if (eNCS_Connected == this->m_eConnecterState
			|| eNCS_Disconnecting == this->m_eConnecterState)
		{
			if (nEvent&eNET_Recv)
			{
				if (this->m_nCloseType == eCT_None || (this->m_nCloseType&eCT_Recv) == 0)
					this->onRecv();
				else
					this->printInfo("socket is close recv data");
			}

			if (nEvent&eNET_Send)
				this->eventSend();
		}
		else
		{
			char szMsg[256] = { 0 };
			base::crt::snprintf(szMsg, _countof(szMsg), "state error，event：%d", nEvent);
			this->printInfo(szMsg);
		}
	}

	void CNetConnecter::forceClose()
	{
		this->m_eConnecterState = eNCS_Disconnected;
		if (this->m_pHandler != nullptr)
		{
			PrintNW("disconnect local addr: %s %d remote addr: %s %d socket_id: %d send_index: %d send_count: %d", this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID(), this->m_nSendConnecterIndex, this->m_pNetEventLoop->getSendConnecterCount());
			this->m_pHandler->onDisconnect();
			this->m_pHandler = nullptr;
		}
		this->m_pNetEventLoop->delSendConnecter(this);
		CNetSocket::forceClose();

		delete this;
	}

	bool CNetConnecter::init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop)
	{
		if (!CNetSocket::init(nSendBufferSize, nRecvBufferSize, pNetEventLoop))
			return false;

		this->m_pRecvBuffer = new CNetRecvBuffer();
		if (!this->m_pRecvBuffer->init(_RECV_BUF_SIZE))
			return false;

		this->m_pSendBuffer = new CNetSendBuffer();
		if (!this->m_pSendBuffer->init(_SEND_BUF_SIZE))
			return false;

		return true;
	}

	void CNetConnecter::onConnect()
	{
		if (eNCS_Connecting == this->m_eConnecterState)
		{
			/*
			如果只可写，说明连接成功，可以进行下面的操作。
			如果描述符既可读又可写，分为两种情况，
			第一种情况是socket连接出现错误（不要问为什么，这是系统规定的，可读可写时候有可能是connect连接成功后远程主机断开了连接close(socket)），
			第二种情况是connect连接成功，socket读缓冲区得到了远程主机发送的数据。需要通过connect连接后返回给errno的值来进行判定，或者通过调用 getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&len); 函数返回值来判断是否发生错误
			*/
			if (this->m_eConnecterType == eNCT_Initiative)
			{
				int32_t err;
				socklen_t len = sizeof(err);
#ifdef _WIN32
				int32_t code = getsockopt(this->GetSocketID(), SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), (socklen_t*)&len);
#else
				int32_t code = getsockopt(this->GetSocketID(), SOL_SOCKET, SO_ERROR, reinterpret_cast<void*>(&err), (socklen_t*)&len);
#endif
				if (code < 0 || err != 0)
				{
					this->shutdown(true, "SO_ERROR");
					return;
				}
			}
			this->m_eConnecterState = eNCS_Connected;
			this->setLocalAddr();
			// 防止自己连自己的情况出现
			if (this->getRemoteAddr() == this->getLocalAddr())
			{
				this->shutdown(true, "connect owner");
				return;
			}
			PrintNW("connect local addr: %s %d remote addr: %s %d socket_id: %d", this->getLocalAddr().szHost, this->getLocalAddr().nPort, this->getRemoteAddr().szHost, this->getRemoteAddr().nPort, this->GetSocketID());
			// 开始时为了获得连接成功的消息而监听的写事件，现在把他注销掉
			this->disableSend();
			if (nullptr != this->m_pHandler)
				this->m_pHandler->onConnect();
		}
		else
			this->shutdown(true, "connection state error");
	}

	void CNetConnecter::onRecv()
	{
		PROFILING_GUARD(CNetConnecter::onRecv)
		while (true)
		{
			uint32_t nBufSize = this->m_pRecvBuffer->getFreeSize();
			if (0 == nBufSize)
			{
				this->m_pRecvBuffer->resize(this->m_pRecvBuffer->getBufferSize() * 2);
				nBufSize = this->m_pRecvBuffer->getFreeSize();
				PrintNW("not have enought recv buffer append size to: %d", this->m_pRecvBuffer->getBufferSize());
			}
			char* pBuf = this->m_pRecvBuffer->getFreeBuffer();
#ifdef _WIN32
			int32_t nRet = ::recv(this->m_nSocketID, pBuf, (int32_t)nBufSize, MSG_NOSIGNAL);
#else
			int32_t nRet = ::read(this->m_nSocketID, pBuf, nBufSize);
#endif
			// 对端已经关闭
			if (0 == nRet)
			{
				this->printInfo("remote connection is close");
				this->m_nCloseType |= eCT_Recv;
				// 为了防止出现连接一边关闭了，但是尚有数据未发送完，一直触发可读事件
				this->disableRecv();
				if (this->m_eConnecterState == eNCS_Connected)
					this->shutdown(false, "remote connection is close");
				
				break;
			}
			else if (nRet < 0)
			{
				if (getLastError() == NW_EAGAIN || getLastError() == NW_EWOULDBLOCK)
					break;

				if (getLastError() == NW_EINTR)
					continue;

				this->shutdown(true, "recv data error");
				break;
			}
			this->m_pRecvBuffer->push(nRet);
			if (nullptr != this->m_pHandler)
				this->m_pRecvBuffer->pop(this->m_pHandler->onRecv(this->m_pRecvBuffer->getDataBuffer(), this->m_pRecvBuffer->getDataSize()));

			// epoll水平触发模式下，即使此时恰好对端关闭了，等下还是会触发recv事件的，不会导致关闭事件丢失
			if (nRet != (int32_t)nBufSize)
				break;
		}
	}

	void CNetConnecter::onSend()
	{
		PROFILING_GUARD(CNetConnecter::onSend)
		while (true)
		{
			uint32_t nBufSize = 0;
			char* pData = this->m_pSendBuffer->getDataBuffer(nBufSize);
			if (0 == nBufSize)
				break;
#ifdef _WIN32
			int32_t nRet = ::send(this->m_nSocketID, pData, (int32_t)nBufSize, MSG_NOSIGNAL);
#else
			int32_t nRet = ::write(this->m_nSocketID, pData, nBufSize);
#endif
			if (SOCKET_ERROR == nRet)
			{
				if (getLastError() == NW_EAGAIN || getLastError() == NW_EWOULDBLOCK)
					break;

				if (getLastError() == NW_EINTR)
					continue;

				this->shutdown(true, "send data error");
				break;
			}
			this->m_pSendBuffer->pop(nRet);
			if (nRet != (int32_t)nBufSize)
				break;
		}
		uint32_t nBufSize = 0;
		this->m_pSendBuffer->getDataBuffer(nBufSize);
		if (0 == nBufSize)
		{
			if (eNCS_Disconnecting == this->m_eConnecterState)
				this->close();
		}
	}

	// 主动调用的
	void CNetConnecter::activeSend()
	{
		this->onSend();
		// 必须要加上正在断开的情况，不然有可能有些链接正在断开，但是这个时候发送缓存区中的数据比较大一次发送不完，那么留下的数据可能再也发送不了了，这个情况发送在服务器主动踢人的情况，很有可能这个时候后面的几个消息到达不了客户端了
		if ((this->m_eConnecterState == eNCS_Connected || this->m_eConnecterState == eNCS_Disconnecting) && !this->m_pSendBuffer->isEmpty())
			this->enableSend();
	}

	// 由模型驱动的
	void CNetConnecter::eventSend()
	{
		this->onSend();
		if ((this->m_eConnecterState == eNCS_Connected || this->m_eConnecterState == eNCS_Disconnecting) && this->m_pSendBuffer->isEmpty())
			this->disableSend();
	}

	CNetConnecter::CNetConnecter()
		: m_eConnecterType(eNCT_Unknown)
		, m_eConnecterState(eNCS_Disconnected)
		, m_nCloseType(eCT_None)
		, m_pHandler(nullptr)
		, m_nSendConnecterIndex(_Invalid_SendConnecterIndex)
		, m_pRecvBuffer(nullptr)
		, m_pSendBuffer(nullptr)
	{
	}

	CNetConnecter::~CNetConnecter()
	{
		DebugAst(this->getConnecterState() == eNCS_Disconnected);
		SAFE_DELETE(this->m_pSendBuffer);
		SAFE_DELETE(this->m_pRecvBuffer);
	}

	int32_t CNetConnecter::getSendConnecterIndex() const
	{
		return this->m_nSendConnecterIndex;
	}

	void CNetConnecter::setSendConnecterIndex(int32_t nIndex)
	{
		this->m_nSendConnecterIndex = nIndex;
	}

	bool CNetConnecter::connect(const SNetAddr& sNetAddr)
	{
		if (!this->open())
			return false;

		if (!this->nonBlock())
			return false;
		
		if (!this->setBufferSize())
			return false;

		this->m_sRemoteAddr = sNetAddr;

		struct sockaddr_in romateAddr;
		romateAddr.sin_family = AF_INET;
		// 不能用::htons https://bbs.archlinux.org/viewtopic.php?id=53751
		romateAddr.sin_port = htons(this->m_sRemoteAddr.nPort);
		romateAddr.sin_addr.s_addr = inet_addr(this->m_sRemoteAddr.szHost);
		memset(romateAddr.sin_zero, 0, sizeof(romateAddr.sin_zero));
		int32_t nRet = 0;
		do
		{
			nRet = ::connect(this->m_nSocketID, (sockaddr*)&romateAddr, sizeof(romateAddr));
		} while (nRet != 0 && getLastError() == NW_EINTR);

		if (0 != nRet)
		{
#ifdef _WIN32
			if (getLastError() == NW_EWOULDBLOCK)
#else
			if (getLastError() == NW_EINPROGRESS)
#endif
			{
				// 这个由事件循环回调可写事件通知连接建立
				this->m_eConnecterType = eNCT_Initiative;
				this->m_eConnecterState = eNCS_Connecting;

				this->m_pNetEventLoop->addSocket(this);
				this->enableSend();
			}
			else
			{
				this->shutdown(true, "connect socket error");
				return false;
			}
		}
		else
		{
			this->m_eConnecterType = eNCT_Initiative;
			this->m_eConnecterState = eNCS_Connected;
			this->m_pNetEventLoop->addSocket(this);
			this->onConnect();
		}

		return true;
	}

	void CNetConnecter::send(const void* pData, uint32_t nDataSize)
	{
		if (eNCS_Connecting == this->m_eConnecterState
		|| eNCS_Disconnected == this->m_eConnecterState
		|| (eNCS_Disconnecting == this->m_eConnecterState && (eCT_Send&this->m_nCloseType) != 0))
			return;

		PROFILING_GUARD(CNetConnecter::send)

		this->m_pSendBuffer->push(reinterpret_cast<const char*>(pData), nDataSize);
		uint32_t nSendDataSize = this->m_pSendBuffer->getDataSize();

		if (nSendDataSize >= this->m_nSendBufferSize)
			this->onEvent(eNET_Send);
		else
			this->m_pNetEventLoop->addSendConnecter(this);
	}

	void CNetConnecter::sendp(const void* pData, uint32_t nDataSize)
	{

	}

	/*	关于连接关闭:
	进程退出时, 对端返回RST（如果RST发送的足够快，同机器下），不然就先收到FIN，触发Recv事件，大小为0
	在主机崩溃时，发送数据会一直不可达，直到超时，在这段时间内，主机重启了，由于此时整个连接的信息都没了，对方在发送数据后会收到RST指令
	网络断开时，发送数据会一直不可达，直到超时，在这段时间内网络正常了那么此时还是正常的。
	直接closesocket时，此时发送跟接受都不行，对端会触发recv事件，数据大小为0
	showdown时,此时发送不行，接收是可以的，对端会触发recv事件，数据大小为0
	*/
	void CNetConnecter::shutdown(bool bForce, const char* szMsg)
	{
		if (this->m_eConnecterState == eNCS_Disconnected)
			return;

		char szBuf[256] = { 0 };
		if (szMsg != NULL)
			base::crt::snprintf(szBuf, _countof(szBuf), "request shutdown connection msg: %s", szMsg);
		else
			base::crt::snprintf(szBuf, _countof(szBuf), "request shutdown connection");

		this->printInfo(szBuf);

		this->m_eConnecterState = eNCS_Disconnecting;
		this->m_nCloseType |= eCT_Send;
		if (bForce || this->m_pSendBuffer->isEmpty())
			this->close();
	}

	void CNetConnecter::setHandler(INetConnecterHandler* pHandler)
	{
		this->m_pHandler = pHandler;
		if (pHandler != nullptr)
			pHandler->setNetConnecter(this);
	}

	const SNetAddr& CNetConnecter::getLocalAddr() const
	{
		return this->m_sLocalAddr;
	}

	const SNetAddr& CNetConnecter::getRemoteAddr() const
	{
		return this->m_sRemoteAddr;
	}

	ENetConnecterType CNetConnecter::getConnecterType() const
	{
		return this->m_eConnecterType;
	}

	ENetConnecterState CNetConnecter::getConnecterState() const
	{
		return this->m_eConnecterState;
	}

	uint32_t CNetConnecter::getSendDataSize() const
	{
		return this->m_pSendBuffer->getDataSize();
	}

	uint32_t CNetConnecter::getRecvDataSize() const
	{
		return this->m_pRecvBuffer->getDataSize();
	}

	void CNetConnecter::setConnecterType(ENetConnecterType eConnecterType)
	{
		this->m_eConnecterType = eConnecterType;
	}

	void CNetConnecter::setConnecterState(ENetConnecterState eConnecterState)
	{
		this->m_eConnecterState = eConnecterState;
	}
}