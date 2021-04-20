#pragma once
#include "Export.h"
#include <thread>
#include "CConcurrentPtrQueue.h"
#include "CPrivateHeapManager.h"
#include "CRawMemoryManager.h"
#include "CTask.h"

class CTaskManager;

class TM_API CWorker
{
	friend class CTaskManager;
	friend class CRawMemoryManager::CRawMemory;

public:
	CWorker() noexcept;
	~CWorker();

public:
	bool Start(CTaskManager* pTaskManager) noexcept;
	void Stop() noexcept;
	void ReleaseResources() noexcept;

public:
	static CWorker* GetCurrentThreadWorker() noexcept;
	int GetWorkerIndex() const noexcept;
	bool IsFinished() const noexcept;

public:
	bool PushTask(CTask *pTask) noexcept;
	CTaskCounter* GetCurrentTaskCounter() noexcept;

	void WorkUntil(CTaskCounter &taskCounter) noexcept;

public:
	CPrivateHeapManager& GetPrivateHeapManager() noexcept;
	CRawMemoryManager& GetRawMemoryManager() noexcept;

private:
	bool Init(CTaskManager* pTaskManager) noexcept;

	bool FindWork() noexcept;

	void ThreadFunc() noexcept;
	void DoWork() noexcept;
	void Idle() noexcept;

private:
	CTaskManager* m_pTaskManager = { nullptr };

	std::thread m_thread;
	volatile bool m_bFinished = { false };
	volatile bool m_bStopping = { true };
	
	CConcurrentPtrQueue<CTask> m_taskQueue;
	CTaskCounter* m_pCurrentTaskCounter = { nullptr };

	CPrivateHeapManager m_privateHeapManager;
	CRawMemoryManager m_rawMemoryManager;
};
