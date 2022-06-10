#pragma once
#include "../Common/configs.h"
#include "CWorker.hpp"
#include <array>


namespace sftm
{
	class CTaskManager
	{
	public:
		CTaskManager() noexcept {}
		~CTaskManager()
		{
			Stop();
		}

	public:
		CTaskManager(const CTaskManager&) = delete;
		void operator=(const CTaskManager&) = delete;
		CTaskManager(CTaskManager&&) = delete;
		CTaskManager& operator=(CTaskManager&&) = delete;

	public:
		void Start(std::uint32_t nInitWorkers) noexcept
		{
			if (nInitWorkers > MAX_WORKERS)
				nInitWorkers = MAX_WORKERS - 1;
			m_nWorkerCount = nInitWorkers;

			for (std::uint32_t nWorker = 0; nWorker <= nInitWorkers; nWorker++)
			{
				m_workers[nWorker].m_ownerData.m_pWorkers		= &m_workers[0];
				m_workers[nWorker].m_ownerData.m_nWorkerCount	= nInitWorkers;
				m_workers[nWorker].m_ownerData.m_pCvWorkerIdle	= &m_cvWorkerIdle;
				m_workers[nWorker].m_ownerData.m_pMutWorkerIdle = &m_mtxWorkerIdle;
			}

			for (std::uint32_t nWorker = 1; nWorker <= nInitWorkers; nWorker++)
			{
				m_workers[nWorker].Start();
			}
		}
		void Stop() noexcept
		{
			for (std::uint32_t nWorker = 1; nWorker <= m_nWorkerCount; nWorker++)
				m_workers[nWorker].Stop();

			for (std::uint32_t nWorker = 1; nWorker <= m_nWorkerCount; nWorker++)
			{
				while (!m_workers[nWorker].IsFinished())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					m_cvWorkerIdle.notify_all();
				}
			}
		}

		CWorker& GetMainWorker() noexcept
		{
			return m_workers[0];
		}
		std::uint32_t GetWorkersCount() const noexcept
		{
			return m_nWorkerCount;
		}

	private:
		std::array<CWorker, MAX_WORKERS>	m_workers;
		std::uint32_t						m_nWorkerCount = 0;

	private:
		std::condition_variable m_cvWorkerIdle;
		std::mutex				m_mtxWorkerIdle;
	};
}