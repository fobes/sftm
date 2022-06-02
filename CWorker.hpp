#pragma once
#include "CConcurrentPtrQueue.hpp"
#include "CTask.hpp"
#include <thread>
#include <functional>

namespace sftm
{
	class CWorker
	{
		friend class CTaskManager;

		struct COwnerData
		{
			CWorker*	m_pWorkers		= nullptr;
			int*		m_pWorkerCount	= nullptr;

			std::condition_variable*	m_pCvWorkerIdle		= nullptr;
			std::mutex*					m_pMutWorkerIdle	= nullptr;
		};

	public:
		CWorker() noexcept;
		~CWorker();

	public:
		CWorker(const CWorker&)			= delete;
		void operator=(const CWorker&)	= delete;
		CWorker(CWorker&&)				= delete;
		CWorker& operator=(CWorker&&)	= delete;

	public:
		bool Start(COwnerData& data) noexcept;
		void Stop() noexcept;
		bool IsFinished() const noexcept;

		bool PushTask(CTask* pTask) noexcept;

		void WorkUntil(CChainController& chainController) noexcept;

	private:
		bool Init(COwnerData& data) noexcept;
		void ReleaseResources() noexcept;

		void ThreadFunc() noexcept;
		void DoWork() noexcept;

		CTask* PopTask() noexcept;
		void ExecuteTask(CTask* pTask) noexcept;

		bool FindWork() noexcept;
		void Idle() noexcept;

	private:
		std::thread::id m_idThread;

		COwnerData m_ownerData;

		volatile bool	m_bFinished = false;
		volatile bool	m_bStopping = true;

		CConcurrentPtrQueue<CTask>	m_taskQueue;
	};

	inline CWorker::CWorker() noexcept
	{

	}

	inline CWorker::~CWorker()
	{
		ReleaseResources();
	}

	inline bool CWorker::Start(COwnerData& data) noexcept
	{
		if (!Init(data))
			return false;

		m_bStopping = false;

		std::thread thread = std::thread(std::bind(&CWorker::ThreadFunc, this));
		thread.detach();

		return true;
	}

	inline void CWorker::Stop() noexcept
	{
		m_bStopping = true;
	}

	inline bool CWorker::IsFinished() const noexcept
	{
		return m_bFinished;
	}

	inline bool CWorker::Init(COwnerData& data) noexcept
	{
		m_ownerData = data;

		m_bFinished = false;

		m_idThread = std::this_thread::get_id();

		return true;
	}

	inline void CWorker::ReleaseResources() noexcept
	{
		m_ownerData.m_pWorkers = nullptr;
		m_ownerData.m_pWorkerCount = nullptr;
		m_ownerData.m_pCvWorkerIdle = nullptr;
		m_ownerData.m_pMutWorkerIdle = nullptr;

		m_bFinished = false;
		m_bStopping = true;
	}

	inline void CWorker::WorkUntil(CChainController& chainController) noexcept
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

	inline void CWorker::DoWork() noexcept
	{
		do
		{
			while (CTask* pTask = PopTask())
			{
				ExecuteTask(pTask);
			}
		} while (FindWork());
	}

	inline CTask* CWorker::PopTask() noexcept
	{
		CTask* pTask = m_taskQueue.Pop();

		return pTask;
	}

	inline void CWorker::ExecuteTask(CTask* pTask) noexcept
	{
		pTask->Execute(*this);

		pTask->m_chainController.Reduce();
	}

	inline void CWorker::ThreadFunc() noexcept
	{
		m_idThread = std::this_thread::get_id();

		while (!m_bStopping)
		{
			DoWork();

			Idle();
		}

		m_bFinished = true;
	}

	inline bool CWorker::PushTask(CTask* pTask) noexcept
	{
		pTask->m_chainController.Increase();

		bool bResult = m_taskQueue.Push(pTask);

		m_ownerData.m_pCvWorkerIdle->notify_all();

		return bResult;
	}

	inline bool CWorker::FindWork() noexcept
	{
		const std::size_t nOffset = static_cast<std::size_t>((this - m_ownerData.m_pWorkers + std::rand() % 100) % (*m_ownerData.m_pWorkerCount));

		for (std::size_t nWorker = 0; nWorker < (*m_ownerData.m_pWorkerCount); nWorker++)
		{
			CWorker* pWorker = m_ownerData.m_pWorkers + (nWorker + nOffset) % (*m_ownerData.m_pWorkerCount);
			if (pWorker == this)
				continue;

			if (m_taskQueue.TrySteal(pWorker->m_taskQueue))
				return true;
		}

		return false;
	}

	inline void CWorker::Idle() noexcept
	{
		std::unique_lock<std::mutex> lock(*m_ownerData.m_pMutWorkerIdle);
		m_ownerData.m_pCvWorkerIdle->wait(lock);
	}
}