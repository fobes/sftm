#include "CWorker.h"
#include "CTaskManager.h"
#include "CTask.h"
#include <functional>
#include <thread>

#define WORKER_STACK_SIZE 64*1024

CWorker::CWorker() noexcept :m_pTaskManager(NULL), m_bFinished(false), m_pCurrentTaskCounter(NULL)
{

}

CWorker::~CWorker()
{

}

bool CWorker::Start(CTaskManager *pTaskManager) noexcept
{
	if (!Init(pTaskManager))
		return false;

	m_thread = std::thread(std::bind(&CWorker::ThreadFunc, this));

	return true;
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
	TlsSetValue(m_pTaskManager->m_nTlsWorker, this);

	while (!m_pTaskManager->m_bStopping)
	{
		DoWork();
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
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
	pTask->m_taskCounter.Increase();

	bool bResult = m_taskQueue.Push(pTask);
	if (!bResult)
		pTask->m_taskCounter.Reduce();

	return bResult;
}

CTaskCounter* CWorker::GetCurrentTaskCounter() noexcept
{
	return m_pCurrentTaskCounter;
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

void CWorker::WorkUntil(CTaskCounter& taskCounter) noexcept
{
	while (!taskCounter.IsEmpty())
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
		while (CTask * pTask = m_taskQueue.Pop())
		{
			CTaskCounter* pLastCompletion = m_pCurrentTaskCounter;
			m_pCurrentTaskCounter = &pTask->m_taskCounter;
			
			pTask->Execute(*this);
			
			m_pCurrentTaskCounter->Reduce();
			m_pCurrentTaskCounter = pLastCompletion;
		}
	} 
	while (FindWork());
}
