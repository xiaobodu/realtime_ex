#include "stdafx.h"
#include "core_connection.h"
#include "core_connection_mgr.h"
#include "base_connection_mgr.h"
#include "core_app.h"
#include "base_app.h"
#include "core_common.h"
#include "base_connection.h"

#include "libBaseCommon/debug_helper.h"
#include "libBaseCommon/base_time.h"
#include "libBaseCommon/profiling.h"

namespace core
{
	
#pragma pack(push,1)

	struct connection_heart_beat
	{
		uint8_t	nContext;
	};

	// 消息头
	struct core_message_header
	{
		uint16_t	nMessageSize;	// 包括消息头的
		uint8_t		nMessageType;
	};

#pragma pack(pop)

	CCoreConnection::CCoreConnection()
		: m_pBaseConnection(nullptr)
		, m_pHeartbeat(nullptr)
		, m_bHeartbeat(false)
		, m_nSendHeartbeatCount(0)
		, m_nID(0)
		, m_nType(0)
	{
	}
	
	CCoreConnection::~CCoreConnection()
	{
		DebugAst(this->m_pNetConnecter == nullptr);
	}

	bool CCoreConnection::init(CBaseConnection* pBaseConnection, uint64_t nID, uint32_t nType)
	{
		DebugAstEx(pBaseConnection != nullptr, false);

		this->m_nType = nType;
		this->m_nID = nID;
		this->m_pBaseConnection = pBaseConnection;
		this->m_pBaseConnection->m_pCoreConnection = this;
		return true;
	}

	uint32_t CCoreConnection::onRecv(const char* pData, uint32_t nDataSize)
	{
		this->m_nSendHeartbeatCount = 0;
		uint32_t nRecvSize = 0;
		try
		{
			do
			{
				uint16_t nMessageSize = 0;

				if (this->m_messageParser != nullptr)
				{
					uint8_t nMessageType = 0;
					int32_t nParserSize = this->m_messageParser(pData, nDataSize, nMessageType);
					if (nParserSize == 0)
						break;

					if (nParserSize <= 0 || nParserSize > UINT16_MAX)
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "parser message error %d", nParserSize);
						this->shutdown(true, szBuf);
						break;
					}

					nMessageSize = (uint16_t)nParserSize;
					this->onDispatch(nMessageType, pData, nMessageSize);
				}
				else
				{
					// 都不够消息头
					if (nDataSize < sizeof(core_message_header))
						break;

					const core_message_header* pHeader = reinterpret_cast<const core_message_header*>(pData);
					if (pHeader->nMessageSize < sizeof(message_header))
					{
						char szBuf[256] = { 0 };
						base::crt::snprintf(szBuf, _countof(szBuf), "message size error message_type[%d]", pHeader->nMessageSize);
						this->shutdown(true, szBuf);
						break;
					}

					// 不是完整的消息
					if (nDataSize < pHeader->nMessageSize)
						break;

					nMessageSize = pHeader->nMessageSize;

					this->onDispatch(pHeader->nMessageType, pHeader + 1, pHeader->nMessageSize - sizeof(core_message_header));
				}

				nRecvSize += nMessageSize;
				nDataSize -= nMessageSize;
				pData += nMessageSize;

			} while (true);
		}
		catch (...)
		{
			this->shutdown(true, "dispatch data error");
		}

		return nRecvSize;
	}

	void CCoreConnection::onConnect()
	{
		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(this->m_pBaseConnection != nullptr);

		this->m_pBaseConnection->onConnect();
		CBaseApp::Inst()->getBaseConnectionMgr()->onDisconnect(this->m_pBaseConnection);

		this->m_pHeartbeat = new CTicker();
		this->m_pHeartbeat->setCallback(std::bind(&CCoreConnection::onHeartbeat, this, std::placeholders::_1));

		CCoreApp::Inst()->registerTicker(this->m_pHeartbeat, CCoreApp::Inst()->getHeartbeatTime() * 1000, CCoreApp::Inst()->getHeartbeatTime() * 1000, 0);
	}

	void CCoreConnection::onDisconnect()
	{
		DebugAst(this->m_pBaseConnection != nullptr);

		this->m_pBaseConnection->onDisconnect();

		CBaseApp::Inst()->getBaseConnectionMgr()->onDisconnect(this->m_pBaseConnection);

		SAFE_DELETE(this->m_pHeartbeat);

		this->m_pNetConnecter = nullptr;

		// 销毁对象
		CCoreApp::Inst()->getCoreConnectionMgr()->destroyCoreConnection(this);
	}

	uint64_t CCoreConnection::getID() const
	{
		return this->m_nID;
	}

	uint32_t CCoreConnection::getType() const
	{
		return this->m_nType;
	}

	void CCoreConnection::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_HEARTBEAT)
			return;

		CCoreApp::Inst()->incQPS();

		this->m_monitor.onRecv(nSize);

		this->m_pBaseConnection->onDispatch(nMessageType, pData, nSize);
	}

	void CCoreConnection::onHeartbeat(uint64_t nContext)
	{
		if (this->m_bHeartbeat)
			return;
		
		if (this->m_nSendHeartbeatCount > CCoreApp::Inst()->getHeartbeatLimit())
		{
			this->shutdown(true, "heart beat time out");
			return;
		}
		
		++this->m_nSendHeartbeatCount;
		connection_heart_beat netMsg;
		this->send(eMT_HEARTBEAT, &netMsg, sizeof(netMsg));
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0);

		this->m_monitor.onSend(nSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			this->m_pNetConnecter->send(pData, nSize);
			break;

		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_GATE_FORWARD:
		case eMT_TO_GATE:
			{
				core_message_header header;
				header.nMessageSize = sizeof(header) + nSize;
				header.nMessageType = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
			}
			break;
		}
	}

	void CCoreConnection::send(uint8_t nMessageType, const void* pData, uint16_t nSize, const void* pExtraBuf, uint16_t nExtraSize)
	{
		PROFILING_GUARD(CCoreConnection::send)

		DebugAst(this->m_pNetConnecter != nullptr);
		DebugAst(pData != nullptr && nSize > 0 && pExtraBuf != nullptr && nExtraSize > 0);

		this->m_monitor.onSend(nSize + nExtraSize);

		switch (nMessageType)
		{
		case eMT_CLIENT:
		case eMT_HEARTBEAT:
			{
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
			}
			break;

		case eMT_REQUEST:
		case eMT_RESPONSE:
		case eMT_SYSTEM:
		case eMT_GATE_FORWARD:
		case eMT_TO_GATE:
			{
				core_message_header header;
				header.nMessageSize = sizeof(header) + nSize + nExtraSize;
				header.nMessageType = nMessageType;
				this->m_pNetConnecter->send(&header, sizeof(header));
				this->m_pNetConnecter->send(pData, nSize);
				this->m_pNetConnecter->send(pExtraBuf, nExtraSize);
			}
			break;
		}
	}

	void CCoreConnection::shutdown(bool bForce, const std::string& szMsg)
	{
		if (this->m_pNetConnecter)
			return;

		if (this->m_pNetConnecter != nullptr)
			this->m_pNetConnecter->shutdown(bForce, szMsg.c_str());
	}

	uint32_t CCoreConnection::getSendDataSize() const
	{
		if (nullptr == this->m_pNetConnecter)
			return 0;

		return this->m_pNetConnecter->getSendDataSize();
	}

	uint32_t CCoreConnection::getRecvDataSize() const
	{
		if (nullptr == this->m_pNetConnecter)
			return 0;

		return this->m_pNetConnecter->getRecvDataSize();
	}

	const SNetAddr& CCoreConnection::getLocalAddr() const
	{
		if (this->m_pNetConnecter == nullptr)
		{
			static SNetAddr s_Default;
			return s_Default;
		}
		return this->m_pNetConnecter->getLocalAddr();
	}

	const SNetAddr& CCoreConnection::getRemoteAddr() const
	{
		if (this->m_pNetConnecter == nullptr)
		{
			static SNetAddr s_Default;
			return s_Default;
		}

		return this->m_pNetConnecter->getRemoteAddr();
	}

	void CCoreConnection::setMessageParser(MessageParser& parser)
	{
		this->m_messageParser = parser;
	}

	void CCoreConnection::enableHeartbeat(bool bEnable)
	{
		this->m_bHeartbeat = bEnable;
	}

	CBaseConnection* CCoreConnection::getBaseConnection() const
	{
		return this->m_pBaseConnection;
	}
}