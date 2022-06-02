#pragma once
#include "CWorker.hpp"
#include <array>
#include "configs.h"

namespace sftm
{
	class CTaskManager
	{
	public:
		CTaskManager() noexcept;
		~CTaskManager();

	public:
		CTaskManager(const CTaskManager&)		= delete;
		void operator=(const CTaskManager&)		= delete;
		CTaskManager(CTaskManager&&)			= delete;
		CTaskManager& operator=(CTaskManager&&) = delete;

	public:
		bool Start(int nInitWorkers) noexcept;
		void Stop() noexcept;

		CWorker* GetCurrentThreadWorker() noexcept;

		bool AddWorker() noexcept;
		void RemoveWorker() noexcept;

		int GetWorkersCount() const noexcept;

	private:
		std::array<CWorker, MAX_WORKERS> m_workers;
		int m_nWorkerCount = 0;

	private:
		std::condition_variable m_cvWorkerIdle;
		std::mutex				m_mtxWorkerIdle;
	};

	inline CTaskManager::CTaskManager() noexcept
	{

	}

	inline CTaskManager::~CTaskManager()
	{
		Stop();
	}

	inline bool CTaskManager::Start(int nInitWorkers) noexcept
	{
		if (nInitWorkers < 2)
			nInitWorkers = 2;
		if (nInitWorkers > MAX_WORKERS)
			nInitWorkers = MAX_WORKERS;

		CWorker::COwnerData data = { &m_workers[0], &m_nWorkerCount, &m_cvWorkerIdle, &m_mtxWorkerIdle };

		if (!m_workers[0].Init(data))
			return false;

		for (int nWorker = 1; nWorker <= nInitWorkers; nWorker++)
		{
			if (!AddWorker())
				return false;
		}

		return true;
	}

	inline void CTaskManager::Stop() noexcept
	{
		for (int nWorker = 1; nWorker < m_nWorkerCount; nWorker++)
			m_workers[nWorker].Stop();

		for (int nWorker = 1; nWorker < m_nWorkerCount; nWorker++)
		{
			while (!m_workers[nWorker].IsFinished())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				m_cvWorkerIdle.notify_all();
			}
		}
	}

	inline bool CTaskManager::AddWorker() noexcept
	{
		if (m_nWorkerCount + 1 >= MAX_WORKERS)
			return false;

		CWorker::COwnerData data = { &m_workers[0], &m_nWorkerCount, &m_cvWorkerIdle, &m_mtxWorkerIdle };

		if (!m_workers[m_nWorkerCount + 1].Start(data))
			return false;

		++m_nWorkerCount;

		return true;
	}

	inline void CTaskManager::RemoveWorker() noexcept
	{
		m_workers[m_nWorkerCount].Stop();

		while (!m_workers[m_nWorkerCount].IsFinished())
		{
		}

		m_workers[m_nWorkerCount].ReleaseResources();

		--m_nWorkerCount;
	}

	inline int CTaskManager::GetWorkersCount() const noexcept
	{
		return m_nWorkerCount;
	}

	inline CWorker* CTaskManager::GetCurrentThreadWorker() noexcept
	{
		auto idThread = std::this_thread::get_id();

		for (int n = 0; n < m_nWorkerCount; ++n)
		{
			if (m_workers[n].m_idThread == idThread)
				return &m_workers[n];
		}

		return nullptr;
	}
}