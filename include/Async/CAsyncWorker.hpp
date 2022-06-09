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
				CAsyncTask* task = nullptr;

				{
					std::unique_lock<std::mutex> lock(*m_pMutWorkerIdle);
					m_pCvWorkerIdle->wait(lock, [this] { return !m_pTaskQueue->Empty(); });

					task = m_pTaskQueue->Pop();
				}

				if (task)
					task->Execute();
			}

			m_bFinished = true;
		}

	private:
		std::condition_variable*         m_pCvWorkerIdle    = nullptr;
		std::mutex*                      m_pMutWorkerIdle	= nullptr;
		CConcurrentPtrQueue<CAsyncTask>* m_pTaskQueue		= nullptr;

	private:
		volatile bool m_bStopping = false;
		volatile bool m_bFinished = false;
	};
}