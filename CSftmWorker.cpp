#include "CSftmWorker.h"
#include "CSftmTaskManager.h"
#include <functional>
#include <thread>

#define WORKER_STACK_SIZE 64*1024

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

	m_thread = std::thread(std::bind(&CSftmWorker::ThreadFunc, this));
	m_thread.detach();

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

	m_pCurrentChainController = nullptr;

	m_privateHeapManager.Release();
	m_rawMemoryManager.Release();
}

CSftmTaskManager* CSftmWorker::GetManager() const noexcept
{
	return m_pTaskManager;
}

CSftmWorker* CSftmWorker::GetCurrentThreadWorker() noexcept
{
	return (CSftmWorker*)TlsGetValue(CSftmTaskManager::m_nTlsWorker);
}

int CSftmWorker::GetWorkerIndex() const noexcept
{
	return int(this - &m_pTaskManager->m_workers[0]);
}

bool CSftmWorker::IsFinished() const noexcept
{
	return m_bFinished;
}

void CSftmWorker::ThreadFunc() noexcept
{
	m_pTaskManager->m_workerFirstFunc();

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
	if (!pTask)
		return false;

	bool bResult = m_taskQueue.Push(pTask, [](CSftmTask* pTask) noexcept {
		auto pController = pTask->GetChainController();
		if (pController)
			pController->Increase();
	});

	m_pTaskManager->m_cvWorkerIdle.notify_one();

	return bResult;
}

CSftmChainController* CSftmWorker::GetCurrentChainController() noexcept
{
	return m_pCurrentChainController;
}

bool CSftmWorker::FindWork() noexcept
{
	int nOffset = (GetWorkerIndex() + GetTickCount64()) % m_pTaskManager->m_nNumberOfWorkers;

	for (unsigned nWorker = 0; nWorker < m_pTaskManager->m_nNumberOfWorkers; nWorker++)
	{
		CSftmWorker* pWorker = &m_pTaskManager->m_workers[(nWorker + nOffset) % m_pTaskManager->m_nNumberOfWorkers];
		if (pWorker == this) 
			continue;

		if (m_taskQueue.TrySteal(pWorker->m_taskQueue))
			return true;

		if (!m_taskQueue.IsEmpty())
			return true;
	}

	return false;
}

void CSftmWorker::WorkUntil(CSftmChainController &chainController) noexcept
{
	while (!chainController.IsFinished())
		DoWork();
}

CSftmPrivateHeapManager& CSftmWorker::GetPrivateHeapManager() noexcept
{
	return m_privateHeapManager;
}

CSftmRawMemoryManager& CSftmWorker::GetRawMemoryManager() noexcept
{
	return m_rawMemoryManager;
}

void CSftmWorker::DoWork() noexcept
{
	do
	{
		while (CSftmTask *pTask = m_taskQueue.Pop())
		{
			CSftmChainController* pLastController = m_pCurrentChainController;

			if (pTask->GetType() == CSftmTask::ETaskType::ESync)
			{
				m_pCurrentChainController = pTask->GetChainController();

				pTask->Execute(*this);

				m_pCurrentChainController->Reduce();
			}
			else
			{
				m_pCurrentChainController = nullptr;

				pTask->Execute(*this);
			}

			m_pCurrentChainController = pLastController;
		}
	} 
	while (FindWork());
}

void CSftmWorker::Idle() noexcept
{
	std::unique_lock<std::mutex> lock(m_pTaskManager->m_mtxWorkerIdle);
	m_pTaskManager->m_cvWorkerIdle.wait(lock);
}
