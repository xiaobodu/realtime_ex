#pragma once
#include "libBaseCommon/singleton.h"

#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <map>

namespace core
{
	/**
	@brief: 消息注册器
	*/
	class CMessageRegistry :
		public base::CSingleton<CMessageRegistry>
	{
	public:
		CMessageRegistry();
		~CMessageRegistry();

		bool init();

		/**
		@brief: 注册普通服务消息
		*/
		void registerServiceCallback(uint16_t nMessageID, ServiceCallback callback);
		/**
		@brief: 注册经网关服务转发客户端的消息
		*/
		void registerGateForwardCallback(uint16_t nMessageID, GateForwardCallback callback);
		/**
		@brief: 添加服务之间的前置过滤器
		*/
		void addGlobalBeforeFilter(ServiceGlobalFilter callback);
		/**
		@brief: 添加服务之间的后置过滤器
		*/
		void addGlobalAfterFilter(ServiceGlobalFilter callback);
	};
}