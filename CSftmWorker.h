#pragma once
#include <thread>
#include "CSftmConcurrentPtrQueue.h"
#include "CSftmPrivateHeapManager.h"
#include "CSftmRawMemoryManager.h"
#include "CSftmTask.h"
#ifdef _PROFILE
#include "CProfiler.h"
#endif


class CSftmTaskManager;

class TM_API CSftmWorker
{
	friend class CSftmTaskManager;
	friend class CSftmTaskAllocator;
	friend class CSftmRawMemoryManager::CSftmRawMemory;

public:
	CSftmWorker() noexcept;
	~CSftmWorker();

public:
	bool Start(CSftmTaskManager* pTaskManager) noexcept;
	void Stop() noexcept;
	bool IsFinished() const noexcept;

	bool PushTask(CSftmTask* pTask) noexcept;
	void WorkUntil(CSftmChainController& chainController) noexcept;
	
public:
	static CSftmWorker* GetCurrentThreadWorker() noexcept;

	void RunProfiling() noexcept;

private:
	bool Init(CSftmTaskManager* pTaskManager) noexcept;
	void ReleaseResources() noexcept;

	void ThreadFunc() noexcept;
	void DoWork() noexcept;
	CSftmTask* PopTask() noexcept;
	void ExecuteTask(CSftmTask* pTask) noexcept;
	bool FindWork() noexcept;
	void Idle() noexcept;

private:
	CSftmTaskManager* m_pTaskManager = nullptr;

	volatile bool	m_bFinished =  false;
	volatile bool	m_bStopping =  true;

	CSftmConcurrentPtrQueue<CSftmTask>	m_taskQueue;

	CSftmPrivateHeapManager	m_privateHeapManager;
	CSftmRawMemoryManager	m_rawMemoryManager;

#ifdef _PROFILE
	CProfiler m_profiler;
#endif
};
