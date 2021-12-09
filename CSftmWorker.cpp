#include "CSftmWorker.h"
#include "CSftmTaskManager.h"
#include <functional>
#include <thread>

CSftmWorker::CSftmWorker() noexcept
{

}

CSftmWorker::~CSftmWorker()
{
	ReleaseResources();
}

bool CSftmWorker::Start(CSftmTaskManager *pTaskManager) noexcept
{
	if (!Init(pTaskManager))
		return false;

	m_bStopping = false;

	std::thread thread = std::thread(std::bind(&CSftmWorker::ThreadFunc, this));
	thread.detach();

	return true;
}

void CSftmWorker::Stop() noexcept
{
	m_bStopping = true;
}

void CSftmWorker::ReleaseResources() noexcept
{
	m_pTaskManager = nullptr;

	m_bFinished = false;
	m_bStopping = true;

	m_privateHeapManager.Release();
	m_rawMemoryManager.Release();
}

CSftmWorker* CSftmWorker::GetCurrentThreadWorker() noexcept
{
	return (CSftmWorker*)TlsGetValue(CSftmTaskManager::m_nTlsWorker);
}

bool CSftmWorker::IsFinished() const noexcept
{
	return m_bFinished;
}

void CSftmWorker::RunProfiling() noexcept
{
#ifdef _PROFILE
	m_profiler.Run();
#endif
}

void CSftmWorker::ThreadFunc() noexcept
{
	TlsSetValue(m_pTaskManager->m_nTlsWorker, this);

	while (!m_bStopping)
	{
		DoWork();
		
		Idle();
	}
		
	m_bFinished = true;
}

bool CSftmWorker::Init(CSftmTaskManager *pTaskManager) noexcept
{
	m_pTaskManager = pTaskManager;
	m_bFinished = false;

	return m_privateHeapManager.Create() && m_rawMemoryManager.Create();
}

bool CSftmWorker::PushTask(CSftmTask *pTask) noexcept
{
	pTask->m_chainController.Increase();

	bool bResult = m_taskQueue.Push(pTask);

	m_pTaskManager->m_cvWorkerIdle.notify_one();

	return bResult;
}

bool CSftmWorker::FindWork() noexcept
{
#ifdef _PROFILE
	START_PROFILE(CProfiler::CItem::EType::ETaskFinding, 0);
#endif

	const int nOffset = (int(this - &m_pTaskManager->m_workers[0]) + GetTickCount64()) % m_pTaskManager->m_nWorkerCount;

	for (unsigned nWorker = 0; nWorker < m_pTaskManager->m_nWorkerCount; nWorker++)
	{
		CSftmWorker* pWorker = &m_pTaskManager->m_workers[(nWorker + nOffset) % m_pTaskManager->m_nWorkerCount];
		if (pWorker == this) 
			continue;

		if (m_taskQueue.TrySteal(pWorker->m_taskQueue))
		{
#ifdef _PROFILE
	END_PROFILE();
#endif

			return true;
		}
	}

#ifdef _PROFILE
	END_PROFILE();
#endif

	return false;
}

void CSftmWorker::WorkUntil(CSftmChainController &chainController) noexcept
{
	while (!chainController.IsFinished())
	{
		do
		{
			while (CSftmTask* pTask = PopTask())
			{
				ExecuteTask(pTask);

				if (chainController.IsFinished())
					return;
			}
		} 
		while (FindWork());
	}
}

void CSftmWorker::DoWork() noexcept
{
	do
	{
		while (CSftmTask *pTask = PopTask())
		{
			ExecuteTask(pTask);
		}
	} 
	while (FindWork());
}

CSftmTask* CSftmWorker::PopTask() noexcept
{
#ifdef _PROFILE
	START_PROFILE(CProfiler::CItem::EType::ETaskPop, 0);
#endif

	CSftmTask* pTask = m_taskQueue.Pop();

#ifdef _PROFILE
	END_PROFILE();
#endif

	return pTask;
}

void CSftmWorker::ExecuteTask(CSftmTask* pTask) noexcept
{
#ifdef _PROFILE
	START_PROFILE(CProfiler::CItem::EType::ETaskExecution, pTask->GetUniqueIndex());
#endif

	pTask->Execute(*this);

	pTask->m_chainController.Reduce();

#ifdef _PROFILE
	END_PROFILE();
#endif
}

void CSftmWorker::Idle() noexcept
{
#ifdef _PROFILE
	START_PROFILE(CProfiler::CItem::EType::EIdle, 0);
#endif

	std::unique_lock<std::mutex> lock(m_pTaskManager->m_mtxWorkerIdle);
	m_pTaskManager->m_cvWorkerIdle.wait(lock);

#ifdef _PROFILE
	END_PROFILE();
#endif
}
