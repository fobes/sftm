#include "CWorker.h"
#include "CTaskManager.h"
#include <functional>
#include <thread>

#define WORKER_STACK_SIZE 64*1024

CWorker::CWorker() noexcept
{

}

CWorker::~CWorker()
{
	ReleaseResources();
}

bool CWorker::Start(CTaskManager *pTaskManager) noexcept
{
	if (!Init(pTaskManager))
		return false;

	m_bStopping = false;

	m_thread = std::thread(std::bind(&CWorker::ThreadFunc, this));
	m_thread.detach();

	return true;
}

void CWorker::Stop() noexcept
{
	m_bStopping = true;
}

void CWorker::ReleaseResources() noexcept
{
	m_pTaskManager = nullptr;

	m_bFinished = false;
	m_bStopping = true;

	m_pCurrentChainController = nullptr;

	m_privateHeapManager.Release();
	m_rawMemoryManager.Release();
}

CWorker* CWorker::GetCurrentThreadWorker() noexcept
{
	return (CWorker*)TlsGetValue(CTaskManager::m_nTlsWorker);
}

int CWorker::GetWorkerIndex() const noexcept
{
	return int(this - &m_pTaskManager->m_workers[0]);
}

bool CWorker::IsFinished() const noexcept
{
	return m_bFinished;
}

void CWorker::ThreadFunc() noexcept
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

bool CWorker::Init(CTaskManager *pTaskManager) noexcept
{
	m_pTaskManager = pTaskManager;
	m_bFinished = false;

	return m_privateHeapManager.Create() && m_rawMemoryManager.Create();
}

bool CWorker::PushTask(CTask *pTask) noexcept
{
	if (!pTask)
		return false;

	bool bResult = m_taskQueue.Push(pTask, [](CTask* pTask) noexcept {
		auto pController = pTask->GetChainController();
		if (pController)
			pController->Increase();
	});

	m_pTaskManager->m_cvWorkerIdle.notify_one();

	return bResult;
}

CChainController* CWorker::GetCurrentChainController() noexcept
{
	return m_pCurrentChainController;
}

bool CWorker::FindWork() noexcept
{
	int nOffset = (GetWorkerIndex() + GetTickCount64()) % m_pTaskManager->m_nNumberOfWorkers;

	for (unsigned nWorker = 0; nWorker < m_pTaskManager->m_nNumberOfWorkers; nWorker++)
	{
		CWorker* pWorker = &m_pTaskManager->m_workers[(nWorker + nOffset) % m_pTaskManager->m_nNumberOfWorkers];
		if (pWorker == this) 
			continue;

		if (m_taskQueue.TrySteal(pWorker->m_taskQueue))
			return true;

		if (!m_taskQueue.IsEmpty())
			return true;
	}

	return false;
}

void CWorker::WorkUntil(CChainController &chainController) noexcept
{
	while (!chainController.IsFinished())
		DoWork();
}

CPrivateHeapManager& CWorker::GetPrivateHeapManager() noexcept
{
	return m_privateHeapManager;
}

CRawMemoryManager& CWorker::GetRawMemoryManager() noexcept
{
	return m_rawMemoryManager;
}

void CWorker::DoWork() noexcept
{
	do
	{
		while (CTask *pTask = m_taskQueue.Pop())
		{
			if (pTask->GetType() == CTask::ESync)
			{
				CChainController* pLastController = m_pCurrentChainController;
				m_pCurrentChainController = pTask->GetChainController();

				pTask->Execute(*this);

				m_pCurrentChainController->Reduce();
				m_pCurrentChainController = pLastController;
			}
			else
			{
				m_pCurrentChainController = nullptr;

				pTask->Execute(*this);
			}
		}
	} 
	while (FindWork());
}

void CWorker::Idle() noexcept
{
	std::unique_lock<std::mutex> lock(m_pTaskManager->m_mtxWorkerIdle);
	m_pTaskManager->m_cvWorkerIdle.wait(lock);
}
