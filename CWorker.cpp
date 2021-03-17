#include "stdafx.h"
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
	return m_taskQueue.PushTask(pTask);
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

		if (m_taskQueue.TryStealTasks(pWorker->m_taskQueue))
			return true;

		if (m_taskQueue.TasksIsExist())
			return true;
	}

	return false;
}

void CWorker::WorkUntil(CTaskCounter& taskCounter) noexcept
{
	while (!taskCounter.IsEmpty())
		DoWork();
}

CWorker::CWorker::CPrivateHeapManager& CWorker::GetPrivateHeapManager() noexcept
{
	return m_privateHeapManager;
}

CWorker::CRawMemoryManager& CWorker::GetRawMemoryManager() noexcept
{
	return m_rawMemoryManager;
}

void CWorker::DoWork() noexcept
{
	do
	{
		while (CTask * pTask = m_taskQueue.PopTask())
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

CWorker::CPrivateHeapManager::CPrivateHeapManager() noexcept :m_hHeap(NULL)
{

}

bool CWorker::CPrivateHeapManager::Create() noexcept
{
	m_hHeap = HeapCreate(0, 0, WORKER_PRIVATE_HEAP_SIZE);

	InitializeSListHead(&m_FreeList);

	return m_hHeap != NULL;
}

void* CWorker::CPrivateHeapManager::Alloc(size_t nSize) noexcept
{
	PSLIST_ENTRY pFree = InterlockedPopEntrySList(&m_FreeList);
	while (pFree)
	{
		HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pFree);
		pFree = InterlockedPopEntrySList(&m_FreeList);
	}

	return HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, nSize);
}

void CWorker::CPrivateHeapManager::Free(void* pData) noexcept
{
	InterlockedPushEntrySList(&m_FreeList, (SLIST_ENTRY*)pData);
}

CWorker::CRawMemoryManager::CRawMemoryManager() noexcept :m_pMemory(NULL), m_nUsedCount(0)
{

}

bool CWorker::CRawMemoryManager::Create() noexcept
{
	m_pMemory = new BYTE[WORKER_RAW_MEMORY_SIZE];
	m_nUsedCount = 0;

	return m_pMemory != NULL;
}

void* CWorker::CRawMemoryManager::Alloc(size_t nSize) noexcept
{
	if (m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return NULL;

	BYTE* ptr = m_pMemory + m_nUsedCount;
	m_nUsedCount += nSize;

	return ptr;
}

void CWorker::CRawMemoryManager::Free(size_t nSize) noexcept
{
	m_nUsedCount -= nSize;
}

CWorker::CTaskQueue::CTaskQueue() noexcept : m_TaskCount(0), m_pTasks{ NULL }
{

}

CWorker::CTaskQueue::~CTaskQueue()
{

}

bool CWorker::CTaskQueue::PushTask(CTask* pTask) noexcept
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);

		if (m_TaskCount >= MAX_WORKER_TASKS_QUEUE)
			return false;

		pTask->m_taskCounter.Increase();
		m_pTasks[m_TaskCount++] = pTask;
	}

	return true;
}

CTask* CWorker::CTaskQueue::PopTask() noexcept
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_TaskCount)
		return NULL;

	CTask* pTask = m_pTasks[m_TaskCount - 1];
	m_TaskCount--;

	return pTask;
}

bool CWorker::CTaskQueue::TryStealTasks(CTaskQueue& srcQueue) noexcept
{
	std::lock_guard<std::mutex> lock(srcQueue.m_mutex);

	if (!srcQueue.m_TaskCount)
		return false;

	std::lock_guard<std::mutex> lockIdleThread(m_mutex);

	if (m_TaskCount)
		return false;

	unsigned nGrabCount = (srcQueue.m_TaskCount + 1) / 2;

	unsigned nTask;
	CTask** p = m_pTasks;
	for (nTask = 0; nTask < nGrabCount; nTask++)
	{
		*p++ = srcQueue.m_pTasks[nTask];
		srcQueue.m_pTasks[nTask] = NULL;
	}
	m_TaskCount = nGrabCount;

	p = srcQueue.m_pTasks;
	for (; nTask < srcQueue.m_TaskCount; nTask++)
		*p++ = srcQueue.m_pTasks[nTask];

	srcQueue.m_TaskCount -= nGrabCount;

	return true;
}

bool CWorker::CTaskQueue::TasksIsExist() noexcept
{
	return m_TaskCount > 0;
}

CWorker::CRawMemoryManager::CRawMemory::CRawMemory() noexcept
{
	m_pData = NULL;
	m_Size = 0;
}

CWorker::CRawMemoryManager::CRawMemory::~CRawMemory()
{
	if (m_pData)
	{
		CWorker* pThread = CWorker::GetCurrentThreadWorker();

		pThread->m_rawMemoryManager.m_nUsedCount -= m_Size;
	}
}

bool CWorker::CRawMemoryManager::CRawMemory::AllocBytes(size_t nSize) noexcept
{
	CWorker* pThread = CWorker::GetCurrentThreadWorker();
	if (m_pData || pThread->m_rawMemoryManager.m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return false;

	m_Size = nSize;
	m_pData = pThread->m_rawMemoryManager.m_pMemory + pThread->m_rawMemoryManager.m_nUsedCount;
	pThread->m_rawMemoryManager.m_nUsedCount += nSize;

	return true;
}
