#include "stdafx.h"
#include "CWorker.h"
#include "CTaskManager.h"
#include "CTask.h"
#include <functional>

#define WORKER_STACK_SIZE 64*1024

CWorker::CWorker():m_pTaskManager(NULL), m_bFinished(false), m_pCurrentTaskCounter(NULL)
{

}

CWorker::~CWorker()
{

}

bool CWorker::Start(CTaskManager *pTaskManager)
{
	if (!Init(pTaskManager))
		return false;

	m_thread = std::thread(std::bind(&CWorker::ThreadFunc, this));

	return true;
}

CWorker* CWorker::GetCurrentThreadWorker()
{
	return (CWorker*)TlsGetValue(CTaskManager::m_nTlsWorker);
}

int CWorker::GetWorkerIndex() const
{
	return int(this - m_pTaskManager->m_workers);
}

bool CWorker::IsFinished() const
{
	return m_bFinished;
}

void CWorker::ThreadFunc()
{
	//Сохраним в локальную память потока инфу о себе
	TlsSetValue(m_pTaskManager->m_nTlsWorker, this);

	while (!m_pTaskManager->m_bStopping)
		DoWork();

	m_bFinished = true;
}

bool CWorker::Init(CTaskManager *pTaskManager)
{
	m_pTaskManager = pTaskManager;
	m_bFinished = false;

	return m_privateHeapManager.Create() && m_rawMemoryManager.Create();
}

bool CWorker::PushTask(CTask *pTask)
{
	return m_taskQueue.PushTask(pTask);
}

CTaskCounter* CWorker::GetCurrentTaskCounter()
{
	return m_pCurrentTaskCounter;
}

bool CWorker::FindWork()
{
	//Считаем смещение чтобы не не пытаться забрать задачи у самого себя
	//Можно получить дедлок
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

void CWorker::WorkUntil(CTaskCounter& taskCounter)
{
	while (!taskCounter.IsEmpty())
		DoWork();
}

CWorker::CWorker::CPrivateHeapManager& CWorker::GetPrivateHeapManager()
{
	return m_privateHeapManager;
}

CWorker::CRawMemoryManager& CWorker::GetRawMemoryManager()
{
	return m_rawMemoryManager;
}

void CWorker::DoWork()
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

CWorker::CPrivateHeapManager::CPrivateHeapManager() :m_hHeap(NULL)
{

}

bool CWorker::CPrivateHeapManager::Create()
{
	m_hHeap = HeapCreate(0, 0, WORKER_PRIVATE_HEAP_SIZE);

	InitializeSListHead(&m_FreeList);

	return m_hHeap != NULL;
}

void* CWorker::CPrivateHeapManager::Alloc(size_t nSize)
{
	PSLIST_ENTRY pFree = InterlockedPopEntrySList(&m_FreeList);
	while (pFree)
	{
		HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pFree);
		pFree = InterlockedPopEntrySList(&m_FreeList);
	}

	return HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, nSize);
}

void CWorker::CPrivateHeapManager::Free(void* pData)
{
	InterlockedPushEntrySList(&m_FreeList, (SLIST_ENTRY*)pData);
}

CWorker::CRawMemoryManager::CRawMemoryManager():m_pMemory(NULL), m_nUsedCount(0)
{

}

bool CWorker::CRawMemoryManager::Create()
{
	m_pMemory = new BYTE[WORKER_RAW_MEMORY_SIZE];
	m_nUsedCount = 0;

	return m_pMemory != NULL;
}

void* CWorker::CRawMemoryManager::Alloc(size_t nSize)
{
	if (m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return NULL;

	BYTE* ptr = m_pMemory + m_nUsedCount;
	m_nUsedCount += nSize;

	return ptr;
}

void CWorker::CRawMemoryManager::Free(size_t nSize)
{
	m_nUsedCount -= nSize;
}

CWorker::CTaskQueue::CTaskQueue(): m_TaskCount(0), m_pTasks{ NULL }
{

}

CWorker::CTaskQueue::~CTaskQueue()
{

}

bool CWorker::CTaskQueue::PushTask(CTask* pTask)
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

CTask* CWorker::CTaskQueue::PopTask()
{
	std::lock_guard<std::mutex> lock(m_mutex);

	if (!m_TaskCount)
		return NULL;

	CTask* pTask = m_pTasks[m_TaskCount - 1];
	m_TaskCount--;

	return pTask;
}

bool CWorker::CTaskQueue::TryStealTasks(CTaskQueue& srcQueue)
{
	std::lock_guard<std::mutex> lock(srcQueue.m_mutex);

	if (!srcQueue.m_TaskCount)
		return false;

	std::lock_guard<std::mutex> lockIdleThread(m_mutex);

	if (m_TaskCount)
		return false;

	/* grab half the remaining tasks (rounding up) */
	unsigned nGrabCount = (srcQueue.m_TaskCount + 1) / 2;

	/* copy old tasks to my list */
	unsigned nTask;
	CTask** p = m_pTasks;
	for (nTask = 0; nTask < nGrabCount; nTask++)
	{
		*p++ = srcQueue.m_pTasks[nTask];
		srcQueue.m_pTasks[nTask] = NULL;
	}
	m_TaskCount = nGrabCount;

	/* move remaining tasks down */
	p = srcQueue.m_pTasks;
	for (; nTask < srcQueue.m_TaskCount; nTask++)
		*p++ = srcQueue.m_pTasks[nTask];

	srcQueue.m_TaskCount -= nGrabCount;

	return true;
}

bool CWorker::CTaskQueue::TasksIsExist()
{
	return m_TaskCount > 0;
}
