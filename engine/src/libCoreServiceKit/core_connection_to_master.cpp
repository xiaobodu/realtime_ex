#include "stdafx.h"
#include "core_connection_to_master.h"
#include "proto_system.h"
#include "core_service_app_impl.h"

#include "libCoreCommon/base_app.h"
#include "libCoreCommon/base_connection_mgr.h"

namespace core
{
	CCoreConnectionToMaster::CCoreConnectionToMaster()
	{

	}

	CCoreConnectionToMaster::~CCoreConnectionToMaster()
	{

	}

	bool CCoreConnectionToMaster::init(const std::string& szContext)
	{
		return true;
	}

	uint32_t CCoreConnectionToMaster::getType() const
	{
		return eBCT_ConnectionToMaster;
	}

	void CCoreConnectionToMaster::release()
	{
		delete this;
	}

	void CCoreConnectionToMaster::onConnect()
	{
		// 连接master的连接只能有一个
		std::vector<CBaseConnection*> vecBaseConnection = CBaseApp::Inst()->getBaseConnectionMgr()->getBaseConnection(this->getType());
		if (vecBaseConnection.size() > 1)
		{
			this->shutdown(false, "dup master connection");
			return;
		}

		smt_register_node_base_info netMsg;
		netMsg.sNodeBaseInfo = CCoreServiceAppImpl::Inst()->getNodeBaseInfo();

		base::CWriteBuf& writeBuf = CBaseApp::Inst()->getWriteBuf();
		netMsg.pack(writeBuf);
		
		this->send(eMT_SYSTEM, writeBuf.getBuf(), (uint16_t)writeBuf.getCurSize());

		CCoreServiceAppImpl::Inst()->setCoreConnectionToMaster(this);

		CCoreServiceAppImpl::Inst()->getCoreMessageRegistry()->onConnectToMaster();
	}

	void CCoreConnectionToMaster::onDisconnect()
	{
		CCoreServiceAppImpl::Inst()->setCoreConnectionToMaster(nullptr);
	}

	void CCoreConnectionToMaster::onDispatch(uint8_t nMessageType, const void* pData, uint16_t nSize)
	{
		DebugAst(nMessageType == eMT_SYSTEM);

		const core::message_header* pHeader = reinterpret_cast<const core::message_header*>(pData);
		DebugAst(nSize > sizeof(core::message_header));

		if (pHeader->nMessageID == eSMT_sync_node_base_info)
		{
			smt_sync_node_base_info netMsg;
			netMsg.unpack(pData, nSize);
			
			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->addNodeBaseInfo(netMsg.sNodeBaseInfo);
		}
		else if (pHeader->nMessageID == eSMT_remove_node_base_info)
		{
			smt_remove_node_base_info netMsg;
			netMsg.unpack(pData, nSize);

			CCoreServiceAppImpl::Inst()->getCoreOtherNodeProxy()->delNodeBaseInfo(netMsg.nNodeID, !!netMsg.nForce);
		}
	}
}