#include "stdafx.h"
#include "core_connection_to_service.h"
#include "core_app.h"
#include "base_connection_mgr.h"
#include "proto_system.h"
#include "base_app.h"
#include "message_dispatcher.h"

namespace core
{

	DEFINE_OBJECT(CCoreConnectionToService, 100)

	CCoreConnectionToService::CCoreConnectionToService()
	{

	}

	CCoreConnectionToService::~CCoreConnectionToService()
	{

	}

	void CCoreConnectionToService::onConnect(const std::string& szContext)
	{
		// szContext�д���Ƿ�������
		this->m_szServiceName = szContext;

		// ͬ����������
		smt_notify_service_base_info netMsg;
		netMsg.szServiceName = this->m_szServiceName;
		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);

		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreApp::Inst()->getServiceMgr()->addConnectionToService(this);
	}

	void CCoreConnectionToService::onDisconnect()
	{
		if (!this->m_szServiceName.empty())
			CCoreApp::Inst()->getServiceMgr()->delConnectionToService(this->m_szServiceName);
	}

	void CCoreConnectionToService::onDispatch(uint32_t nMessageType, const void* pData, uint16_t nSize)
	{
		if (nMessageType == eMT_SYSTEM)
		{
			// empty
		}
		else
		{
			CMessageDispatcher::Inst()->dispatch(this->getServiceName(), nMessageType, pData, nSize);
		}
	}

	const std::string& CCoreConnectionToService::getServiceName() const
	{
		return this->m_szServiceName;
	}

}