#pragma once

#include "libBaseCommon/base_function.h"

#ifdef _WIN32
#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 22
#endif
#else
#include <netinet/in.h>
#endif

struct SNetAddr
{
	char		szHost[INET_ADDRSTRLEN];
	uint16_t	nPort;

	SNetAddr(const char* szIP, uint16_t nPort)
		: nPort(nPort)
	{
		base::crt::strcpy(this->szHost, _countof(this->szHost), szIP);
	}

	SNetAddr()
		: nPort(0)
	{
		base::crt::strcpy(this->szHost, _countof(this->szHost), "0.0.0.0");
	}

	bool operator == (const SNetAddr& rhs) const
	{
		return strncmp(this->szHost, rhs.szHost, INET_ADDRSTRLEN) == 0 && this->nPort == rhs.nPort;
	}

	bool isAnyIP() const
	{
		if (memcmp(this->szHost, "0.0.0.0", strnlen("0.0.0.0", INET_ADDRSTRLEN - 1)) == 0)
			return true;

		return false;
	}
};

namespace base
{
#ifdef _WIN32
#	ifdef __BUILD_BASE_NETWORK_DLL__
#		define __BASE_NETWORK_API__ __declspec(dllexport)
#	else
#		define __BASE_NETWORK_API__ __declspec(dllimport)
#	endif
#else
#	define __BASE_NETWORK_API__
#endif

	enum ENetConnecterState
	{
		eNCS_Connecting,		// 连接进程中
		eNCS_Connected,			// 连接完成
		eNCS_Disconnecting,		// 断开连接进行中
		eNCS_Disconnected,		// 断开连接完成

		eNCS_Unknown
	};

	enum ENetConnecterType
	{
		eNCT_Initiative,	// 主动连接
		eNCT_Passive,		// 被动连接

		eNCT_Unknown
	};

	class INetConnecter;
	/**
	@brief: 网络连接处理器
	连接的超时机制需要逻辑层去处理，如果数据一直发送不了，或者连接已经是断开连接中，确迟迟无法全部断开，很可能是数据一直发送不了导致的，这个时候应该踢掉连接
	*/
	class INetConnecterHandler
	{
	protected:
		INetConnecter* m_pNetConnecter;	// 连接对象

	public:
		INetConnecterHandler() : m_pNetConnecter(nullptr) {}
		virtual ~INetConnecterHandler() {}

		/**
		@brief: 设置连接处理器
		*/
		void				setNetConnecter(INetConnecter* pConnecter) { this->m_pNetConnecter = pConnecter; }
		/**
		@brief: 获取连接处理器
		*/
		INetConnecter*		getNetConnecter() const { return this->m_pNetConnecter; }
		/**
		@brief: 数据到达回调
		*/
		virtual uint32_t	onRecv(const char* pData, uint32_t nDataSize) = 0;
		/**
		@brief: 连接完成回调
		*/
		virtual void		onConnect() = 0;
		/**
		@brief: 连接断开回调
		*/
		virtual void		onDisconnect() = 0;
	};

	class INetAccepter;
	/**
	@brief: 网络连接监听处理器
	*/
	class INetAccepterHandler
	{
	protected:
		INetAccepter* m_pNetAccepter;	// 监听对象

	public:
		INetAccepterHandler() : m_pNetAccepter(nullptr) {}
		virtual ~INetAccepterHandler() {}

		/**
		@brief: 设置监听处理器
		*/
		void                          setNetAccepter(INetAccepter* pAccepter) { this->m_pNetAccepter = pAccepter; }
		/**
		@brief: 获取监听处理器
		*/
		INetAccepter*                 getNetAccepter() const { return this->m_pNetAccepter; }
		/**
		@brief: 监听的端口有连接进来，试着获取对应连接的连接处理器
		*/
		virtual INetConnecterHandler* onAccept(INetConnecter* pNetConnecter) = 0;
	};

	/**
	@brief: 网络监听器接口
	*/
	class INetAccepter
	{
	public:
		INetAccepter() { }
		virtual ~INetAccepter() {}

		/**
		@brief: 设置监听器处理器
		*/
		virtual void            setHandler(INetAccepterHandler* pHandler) = 0;
		/**
		@brief: 获取监听地址
		*/
		virtual const SNetAddr& getListenAddr() const = 0;
		/**
		@brief: 关闭监听器
		*/
		virtual void            shutdown() = 0;
	};

	/**
	@brief: 网络连接器接口
	*/
	class INetConnecter
	{
	public:
		INetConnecter() { }
		virtual ~INetConnecter() { }

		/**
		@brief: 发送数据, 数据是拷贝到发送缓冲区的
		*/
		virtual void				send(const void* pData, uint32_t nSize) = 0;
		/**
		@brief: 发送数据, 数据只整块内存挂载到发送缓冲区的，由网络层负责释放内存
		*/
		virtual void				sendp(const void* pData, uint32_t nDataSize) = 0;
		/**
		@brief: 关闭连接，如果连接当前没有建立最好设置下setHandler( nullptr ) 不然还是会调用INetConnecterHandler的onDisconnect
		*/
		virtual void				shutdown(bool bForce, const char* szMsg) = 0;
		/**
		@brief: 设置连接处理器
		*/
		virtual void				setHandler(INetConnecterHandler* pHandler) = 0;
		/**
		@brief: 获取连接本地地址
		*/
		virtual const SNetAddr&		getLocalAddr() const = 0;
		/**
		@brief: 获取连接远端地址
		*/
		virtual const SNetAddr&		getRemoteAddr() const = 0;
		/**
		@brief: 获取连接器类型
		*/
		virtual ENetConnecterType	getConnecterType() const = 0;
		/**
		@brief: 获取连接器状态
		*/
		virtual ENetConnecterState	getConnecterState() const = 0;
		/**
		@brief: 获取发送缓存区中的数据大小，这个一般缓存一帧数据，如果一帧内无法发送完数据还是会缓存
		*/
		virtual	uint32_t			getSendDataSize() const = 0;
		/**
		@brief: 获取接收缓存区大小
		*/
		virtual	uint32_t			getRecvDataSize() const = 0;
	};

	/**
	@brief: 网络事件循环器
	*/
	class INetEventLoop
	{
	public:
		INetEventLoop() { }
		virtual ~INetEventLoop() {}

		/**
		@brief: 初始化
		*/
		virtual bool	init(uint32_t nMaxSocketCount) = 0;
		/**
		@brief: 发起一个监听
		*/
		virtual bool	listen(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetAccepterHandler* pHandler) = 0;
		/**
		@brief: 发起一个连接
		*/
		virtual bool	connect(const SNetAddr& netAddr, uint32_t nSendBufferSize, uint32_t nRecvBufferSize, INetConnecterHandler* pHandler) = 0;
		/**
		@brief: 推动网络事件循环器
		*/
		virtual void	update(int64_t nTime) = 0;
		/**
		@brief: 在事件循环等待中唤醒等待
		*/
		virtual void	wakeup() = 0;
		/**
		@brief: 释放网络事件循环器
		*/
		virtual void	release() = 0;
	};

	__BASE_NETWORK_API__ bool			startupNetwork();
	__BASE_NETWORK_API__ void			cleanupNetwork();

	__BASE_NETWORK_API__ INetEventLoop*	createNetEventLoop();
}