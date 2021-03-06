#pragma once

#include "net_event_loop.h"
#include "network.h"

namespace base
{
	/**
	@brief: 网络基础类
	*/
	class INetBase
	{
	public:
		virtual ~INetBase() { }

		/**
		@brief: 事件回调
		*/
		virtual void	onEvent(uint32_t nEvent) = 0;
	};

	class CNetSocket :
		public INetBase
	{
		friend class CNetEventLoop;

	public:
		CNetSocket();
		~CNetSocket();

		virtual bool	init(uint32_t nSendBufferSize, uint32_t nRecvBufferSize, CNetEventLoop* pNetEventLoop);
		virtual void	onEvent(uint32_t nEvent) = 0;
		virtual void	forceClose();

		bool			open();
		bool			nonBlock();
		bool			reuseAddr();
		bool			setBufferSize();
		void			setRemoteAddr();
		void			setLocalAddr();
		void			setSocketID(int32_t nSocketID);
		int32_t			GetSocketID() const;
		void			setSocketIndex(int32_t nIndex);
		int32_t			getSocketIndex() const;
		void			enableSend();
		void			disableSend();
		void			disableRecv();
		uint32_t		getEvent() const;

	protected:
		void			close(bool bCloseSend = true);

	protected:
		CNetEventLoop*	m_pNetEventLoop;
		int32_t			m_nSocketID;
		SNetAddr		m_sLocalAddr;
		SNetAddr		m_sRemoteAddr;
		uint32_t		m_nEvent;
		int32_t			m_nSocketIndex;
		uint32_t		m_nSendBufferSize;
		uint32_t		m_nRecvBufferSize;
		bool			m_bWaitClose;
	};
}