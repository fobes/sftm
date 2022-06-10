#pragma once

#include "../Common/CConcurrentPtrQueue.hpp"
#include "CAsyncTask.hpp"

#include <functional>
#include <thread>
#include <mutex>

namespace sftm
{
	class CAsyncWorker
	{
		friend class CAsyncTaskManager;

		struct COwnerData
		{
			std::condition_variable*	m_pCvWorkerIdle		= nullptr;
			std::mutex*					m_pMutWorkerIdle	= nullptr;

			CConcurrentPtrQueue<CAsyncTask>* m_pTaskQueue	= nullptr;
		};

	public:
		CAsyncWorker() {}
		~CAsyncWorker() {}

	private:
		void Start() noexcept
		{
			std::thread thread = std::thread(&CAsyncWorker::ThreadFunc, this);
			thread.detach();
		}
		void Stop() noexcept
		{
			m_bStopping = true;
		}
		bool IsFinished() const noexcept
		{
			return m_bFinished;
		}

		void ThreadFunc() noexcept
		{
			while (!m_bStopping)
			{
				CAsyncTask* pTask = m_ownerData.m_pTaskQueue->Pop();
				if (pTask)
					pTask->Execute();

				std::unique_lock<std::mutex> lock(*m_ownerData.m_pMutWorkerIdle);
				m_ownerData.m_pCvWorkerIdle->wait(lock);
			}

			m_bFinished = true;
		}

	private:
		COwnerData m_ownerData;

	private:
		volatile bool m_bStopping = false;
		volatile bool m_bFinished = false;
	};
}