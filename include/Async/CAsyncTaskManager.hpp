#pragma once
#include "../Common/configs.h"
#include "CAsyncWorker.hpp"
#include "../Common/CConcurrentPtrQueue.hpp"

#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace sftm
{
	class CAsyncTaskManager
	{
	public:
		CAsyncTaskManager() noexcept {}
		~CAsyncTaskManager()
		{
			Stop();
		}

	public:
		CAsyncTaskManager(const CAsyncTaskManager&)       = delete;
		void operator=(const CAsyncTaskManager&)          = delete;
		CAsyncTaskManager(CAsyncTaskManager&&)            = delete;
		CAsyncTaskManager& operator=(CAsyncTaskManager&&) = delete;

	public:
		void Start(std::uint32_t nInitWorkers = 1) noexcept
		{
			if (nInitWorkers > MAX_WORKERS)
				nInitWorkers = MAX_WORKERS - 1;
			m_nWorkerCount = nInitWorkers;

			for (std::uint32_t nWorker = 0; nWorker < nInitWorkers; nWorker++)
			{
				m_workers[nWorker].m_ownerData.m_pCvWorkerIdle	= &m_cvWorkerIdle;
				m_workers[nWorker].m_ownerData.m_pMutWorkerIdle = &m_mtxWorkerIdle;
				m_workers[nWorker].m_ownerData.m_pTaskQueue		= &m_taskQueue;

				m_workers[nWorker].Start();
			}
		}
		void Stop() noexcept
		{
			for (std::uint32_t nWorker = 0; nWorker < m_nWorkerCount; nWorker++)
				m_workers[nWorker].Stop();

			for (std::uint32_t nWorker = 0; nWorker < m_nWorkerCount; nWorker++)
			{
				while (!m_workers[nWorker].IsFinished())
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					m_cvWorkerIdle.notify_all();
				}
			}
		}
		
		void PushTask(CAsyncTask* task) noexcept
		{
			std::unique_lock<std::mutex> lock(m_mtxWorkerIdle);
			m_taskQueue.Push(task);

			m_cvWorkerIdle.notify_one();
		}

		std::uint32_t GetWorkersCount() const noexcept
		{
			return m_nWorkerCount;
		}

	private:
		std::condition_variable m_cvWorkerIdle;
		std::mutex				m_mtxWorkerIdle;

	private:
		std::array<CAsyncWorker, MAX_WORKERS> m_workers;
		std::uint32_t                         m_nWorkerCount = 0;

		CConcurrentPtrQueue<CAsyncTask>       m_taskQueue;
	};

}
