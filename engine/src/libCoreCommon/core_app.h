#pragma once

#include "libBaseCommon/singleton.h"

#include "base_app.h"
#include "ticker_mgr.h"
#include "coroutine_mgr.h"
#include "core_connection_mgr.h"
#include "logic_message_queue.h"

namespace core
{
	/**
	@brief: ���Ŀ���࣬��������Ҫ������Ϸ�и���������
	*/
	class CCoreApp :
		public base::CSingleton<CCoreApp>
	{
	public:
		CCoreApp();
		~CCoreApp();

		/**
		@brief: �������
		*/
		bool				run(int32_t argc, char** argv, const char* szConfig);
		/**
		@brief: ע�ᶨʱ��
		nStartTime ��һ�δ�����ʱ����ʱ��
		nIntervalTime ��һ�δ�����ʱ�����������ʱ�������ļ��ʱ�䣬�����ֵ��0�ͱ�ʾ�����ʱ��ֻ����һ��
		*/
		void				registerTicker(CTicker* pTicker, uint64_t nStartTime, uint64_t nIntervalTime, uint64_t nContext);
		/**
		@brief: ��ע�ᶨʱ��
		*/
		void				unregisterTicker(CTicker* pTicker);
		/**
		@brief: ��ȡ��ǰ�߼�ʱ��
		*/
		int64_t				getLogicTime() const;
		/*
		@brief: ��ȡ���ӹ�����
		*/
		CBaseConnectionMgr*	getBaseConnectionMgr() const;
		/*
		@brief: ��ȡЯ�̹�����
		*/
		CCoroutineMgr*		getCoroutineMgr() const;
		/*
		@brief: ��ȡ�����ļ���
		*/
		const std::string&	getConfigFileName() const;
		/*
		@brief: ��ȡдbuf���󣬵���Ҫ������Ϣ���
		*/
		base::CWriteBuf&	getWriteBuf() const;
		/*
		@brief: �˳����
		*/
		void				doQuit();
		/*
		@brief: ��ȡ����������ʱ���޴���
		*/
		uint32_t			getHeartbeatLimit() const;
		/*
		@brief: ��ȡ��������ͬ��ʱ��
		*/
		uint32_t			getHeartbeatTime() const;
		/*
		@brief: ��ȡ����ʱ��
		*/
		uint32_t			getSamplingTime() const;
		/*
		@brief: ��ȡ��Ϣ����
		*/
		CLogicMessageQueue*	getMessageQueue() const;
		/*
		@brief: ��ȡQPS
		*/
		uint32_t			getQPS() const;
		/*
		@brief: ����QPS
		*/
		void				incQPS();
		/*
		@brief: ��Ƿ�æ�������߼��߳̿��Բ��ȴ���Ϣ���е�����
		*/
		void				busy();

	private:
		bool				onInit();
		bool				onProcess();
		void				onDestroy();
		void				onQPS(uint64_t nContext);

	protected:
		std::string				m_szConfig;
		std::string				m_szPID;
		CTickerMgr*				m_pTickerMgr;
		CBaseConnectionMgr*		m_pBaseConnectionMgr;
		CCoroutineMgr*			m_pCoroutineMgr;
		CLogicMessageQueue*		m_pMessageQueue;
		base::CWriteBuf			m_writeBuf;
		uint32_t				m_nCycleCount;
		int64_t					m_nTotalSamplingTime;
		uint32_t				m_nSamplingTime;
		volatile uint32_t		m_nRunState;
		uint32_t				m_nHeartbeatLimit;
		uint32_t				m_nHeartbeatTime;
		bool					m_bMarkQuit;	// �������������ֻ����һ��onQuit
		uint32_t				m_nQPS;
		CTicker					m_tickerQPS;
		bool					m_bBusy;
	};
}