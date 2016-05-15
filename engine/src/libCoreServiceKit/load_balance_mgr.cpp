#include "stdafx.h"
#include "load_balance_mgr.h"
#include "load_balance.h"

namespace core
{
	CLoadBalanceMgr::CLoadBalanceMgr()
	{

	}

	CLoadBalanceMgr::~CLoadBalanceMgr()
	{

	}

	bool CLoadBalanceMgr::init()
	{
		this->registerLoadBalance(new CRandomLoadBalance());
		return true;
	}

	void CLoadBalanceMgr::registerLoadBalance(ILoadBalance* pLoadBalance)
	{
		DebugAst(pLoadBalance != nullptr);

		this->m_mapLoadBalance[pLoadBalance->getName()] = pLoadBalance;
	}

	ILoadBalance* CLoadBalanceMgr::getLoadBalance(const std::string& szName) const
	{
		if (szName == "*")
			return this->getLoadBalance("random");
		
		auto iter = this->m_mapLoadBalance.find(szName);
		if (iter == this->m_mapLoadBalance.end())
			return nullptr;

		return iter->second;
	}
}