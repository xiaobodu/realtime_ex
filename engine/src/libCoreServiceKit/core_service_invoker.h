#pragma once
#include "libBaseCommon/noncopyable.h"
#include "libCoreCommon/core_common.h"

#include "core_service_kit_define.h"

#include <map>
#include <vector>

namespace core
{
	class CCoreServiceInvoker :
		public base::noncopyable
	{
	public:
		CCoreServiceInvoker();
		~CCoreServiceInvoker();

		bool							init();

		void							registerCallback(uint16_t nMessageID, const ServiceCallback& callback);
		void							registerCallback(uint16_t nMessageID, const GateForwardCallback& callback);
		ServiceCallback&				getCallback(uint32_t nMessageID);
		GateForwardCallback&			getGateClientCallback(uint32_t nMessageID);

		void							onConnectToMaster();

	private:
		std::map<uint32_t, ServiceCallback>				m_mapServiceCallback;
		std::map<uint32_t, GateForwardCallback>			m_mapGateClientCallback;
		std::map<uint32_t, std::string>					m_mapMessageName;
	};
}