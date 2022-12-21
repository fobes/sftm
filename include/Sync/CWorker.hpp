#pragma once

#include "../Common/CConcurrentPtrQueue.hpp"
#include "CTask.hpp"

#include <thread>
#include <functional>
#include <condition_variable>

namespace sftm
{
	class CWorker
	{
		friend class CTaskManager;

		struct COwnerData
		{
			CWorker*		m_pWorkers		= nullptr;
			std::uint32_t	m_nWorkerCount	= 0;

			std::condition_variable*	m_pCvWorkerIdle		= nullptr;
			std::mutex*					m_pMutWorkerIdle	= nullptr;
		};

	public:
		CWorker() noexcept {}
		~CWorker() {}

	public:
		CWorker(const CWorker&)			= delete;
		void operator=(const CWorker&)	= delete;
		CWorker(CWorker&&)				= delete;
		CWorker& operator=(CWorker&&)	= delete;

	public:
		template<class TItem>
		bool PushTask(TItem* pItems, std::uint32_t nCount) noexcept
		{
			bool bResult = m_taskQueue.Push(pItems, nCount);

			m_ownerData.m_pCvWorkerIdle->notify_all();

			return bResult;
		}

		void WorkUntil(CChainController& chainController) noexcept
		{
			while (!chainController.IsFinished())
			{
				do
				{
					while (CTask* pTask = PopTask())
					{
						ExecuteTask(pTask);

						if (chainController.IsFinished())
							return;
					}
				} while (FindWork());
			}
		}

	private:
		void Start() noexcept
		{
			std::thread thread = std::thread(std::bind(&CWorker::ThreadFunc, this));
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
				DoWork();

				Idle();
			}

			m_bFinished = true;
		}
		void DoWork() noexcept
		{
			do
			{
				while (CTask* pTask = PopTask())
				{
					ExecuteTask(pTask);
				}
			} while (FindWork());
		}

		CTask* PopTask() noexcept
		{
			CTask* pTask = m_taskQueue.Pop();

			return pTask;
		}
		void ExecuteTask(CTask* pTask) noexcept
		{
			pTask->Execute(*this);

			pTask->m_chainController.Reduce();
		}

		bool FindWork() noexcept
		{
			const std::uint32_t nOffset = static_cast<std::uint32_t>((this - m_ownerData.m_pWorkers + static_cast<std::uint32_t>(std::rand() % 64)) % m_ownerData.m_nWorkerCount);

			for (std::uint32_t nWorker = 0; nWorker <= m_ownerData.m_nWorkerCount; nWorker++)
			{
				CWorker* pWorker = m_ownerData.m_pWorkers + (nWorker + nOffset) % m_ownerData.m_nWorkerCount;
				if (pWorker == this)
					continue;

				if (m_taskQueue.TrySteal(pWorker->m_taskQueue))
					return true;
			}

			return false;
		}
		void Idle() noexcept
		{
			std::unique_lock<std::mutex> lock(*m_ownerData.m_pMutWorkerIdle);
			m_ownerData.m_pCvWorkerIdle->wait(lock);
		}

	private:
		COwnerData m_ownerData;

		volatile bool	m_bFinished = false;
		volatile bool	m_bStopping = false;

		CConcurrentPtrQueue<CTask>	m_taskQueue;
	};
}