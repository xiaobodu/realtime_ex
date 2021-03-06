#include "stdafx.h"
#include "cluster_invoker.h"
#include "core_service_app.h"
#include "core_service_app_impl.h"
#include "core_service_kit_define.h"
#include "service_base.h"
#include "base_actor.h"

#include "libBaseCommon/debug_helper.h"
#include "libCoreCommon/coroutine.h"
#include "libBaseCommon/base_time.h"

namespace core
{

	CClusterInvoker::CClusterInvoker()
	{

	}

	CClusterInvoker::~CClusterInvoker()
	{

	}

	bool CClusterInvoker::init()
	{
		return true;
	}

	bool CClusterInvoker::invoke(uint16_t nNodeID, const void* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.nSessionID = 0;
		sRequestMessageInfo.nFromActorID = 0;
		sRequestMessageInfo.nToActorID = 0;

		return CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nNodeID, sRequestMessageInfo);
	}

	void CClusterInvoker::response(const void* pData)
	{
		DebugAst(pData != nullptr);

		this->response(CCoreServiceAppImpl::Inst()->getTransporter()->getNodeSessionInfo(), pData);
	}

	void CClusterInvoker::response(const SNodeSessionInfo& sNodeSessionInfo, const void* pData)
	{
		DebugAst(pData != nullptr);

		SResponseMessageInfo sResponseMessageInfo;
		sResponseMessageInfo.nSessionID = sNodeSessionInfo.nSessionID;
		sResponseMessageInfo.pData = pData;
		sResponseMessageInfo.nResult = eRRT_OK;
		sResponseMessageInfo.nFromActorID = 0;
		sResponseMessageInfo.nToActorID = 0;

		bool bRet = CCoreServiceAppImpl::Inst()->getTransporter()->response(sNodeSessionInfo.nNodeID, sResponseMessageInfo);
		DebugAst(bRet);
	}

	SNodeSessionInfo CClusterInvoker::getServiceSessionInfo()
	{
		return CCoreServiceAppImpl::Inst()->getTransporter()->getNodeSessionInfo();
	}

	bool CClusterInvoker::send(const SClientSessionInfo& sClientSessionInfo, const void* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SGateMessageInfo sGateMessageInfo;
		sGateMessageInfo.nSessionID = sClientSessionInfo.nSessionID;
		sGateMessageInfo.pData = pData;

		return CCoreServiceAppImpl::Inst()->getTransporter()->send(sClientSessionInfo.nGateNodeID, sGateMessageInfo);
	}

	bool CClusterInvoker::forward(uint16_t nNodeID, uint64_t nSessionID, const void* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nActorID = 0;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pData = pData;

		return CCoreServiceAppImpl::Inst()->getTransporter()->forward(nNodeID, sGateMessageInfo);
	}

	bool CClusterInvoker::forward_a(uint64_t nActorID, uint64_t nSessionID, const void* pData)
	{
		DebugAstEx(pData != nullptr, false);

		SGateForwardMessageInfo sGateMessageInfo;
		sGateMessageInfo.nActorID = nActorID;
		sGateMessageInfo.nSessionID = nSessionID;
		sGateMessageInfo.pData = pData;

		uint16_t nNodeID = CBaseActor::getNodeID(nActorID);
		return CCoreServiceAppImpl::Inst()->getTransporter()->forward(nNodeID, sGateMessageInfo);
	}

	bool CClusterInvoker::broadcast(const std::vector<SClientSessionInfo>& vecClientSessionInfo, const void* pData)
	{
		DebugAstEx(pData != nullptr, false);

		std::map<uint16_t, std::vector<uint64_t>> mapClientSessionInfo;
		for (size_t i = 0; i < vecClientSessionInfo.size(); ++i)
		{
			mapClientSessionInfo[vecClientSessionInfo[i].nGateNodeID].push_back(vecClientSessionInfo[i].nSessionID);
		}

		bool bRet = true;
		for (auto iter = mapClientSessionInfo.begin(); iter != mapClientSessionInfo.end(); ++iter)
		{
			SGateBroadcastMessageInfo sGateBroadcastMessageInfo;
			sGateBroadcastMessageInfo.vecSessionID = iter->second;
			sGateBroadcastMessageInfo.pData = pData;
			if (!CCoreServiceAppImpl::Inst()->getTransporter()->broadcast(iter->first, sGateBroadcastMessageInfo))
				bRet = false;
		}

		return bRet;
	}

	bool CClusterInvoker::invokeImpl(uint16_t nNodeID, const void* pData, const std::function<void(CMessagePtr<char>, uint32_t)>& callback)
	{
		SRequestMessageInfo sRequestMessageInfo;
		sRequestMessageInfo.pData = pData;
		sRequestMessageInfo.nSessionID = CCoreServiceAppImpl::Inst()->getTransporter()->genSessionID();
		sRequestMessageInfo.nFromActorID = 0;
		sRequestMessageInfo.nToActorID = 0;

		if (!CCoreServiceAppImpl::Inst()->getTransporter()->invoke(nNodeID, sRequestMessageInfo))
			return false;

		SResponseWaitInfo* pResponseWaitInfo = CCoreServiceAppImpl::Inst()->getTransporter()->addResponseWaitInfo(sRequestMessageInfo.nSessionID);
		DebugAstEx(pResponseWaitInfo != nullptr, false);
		pResponseWaitInfo->nTraceID = CCoreServiceAppImpl::Inst()->getInvokerTrace()->getCurTraceID();
		pResponseWaitInfo->nToID = nNodeID;
		//pResponseWaitInfo->nMessageID = pData->nMessageID;
		pResponseWaitInfo->nBeginTime = base::getGmtTime();

		pResponseWaitInfo->callback = callback;
		return true;
	}
}