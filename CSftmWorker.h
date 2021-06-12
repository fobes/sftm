#pragma once
#include <thread>
#include "CSftmConcurrentPtrQueue.h"
#include "CSftmPrivateHeapManager.h"
#include "CSftmRawMemoryManager.h"
#include "CSftmTask.h"

class CSftmTaskManager;

class TM_API CSftmWorker
{
	friend class CSftmTaskManager;
	friend class CSftmRawMemoryManager::CSftmRawMemory;

public:
	CSftmWorker() noexcept;
	~CSftmWorker();

public:
	bool Start(CSftmTaskManager* pTaskManager) noexcept;
	void Stop() noexcept;
	void ReleaseResources() noexcept;

public:
	CSftmTaskManager* GetManager() const noexcept;

	static CSftmWorker* GetCurrentThreadWorker() noexcept;
	int GetWorkerIndex() const noexcept;
	bool IsFinished() const noexcept;

public:
	bool PushTask(CSftmTask* pTask) noexcept;
	CSftmChainController* GetCurrentChainController() noexcept;

	void WorkUntil(CSftmChainController& chainController) noexcept;

public:
	CSftmPrivateHeapManager& GetPrivateHeapManager() noexcept;
	CSftmRawMemoryManager& GetRawMemoryManager() noexcept;

private:
	bool Init(CSftmTaskManager* pTaskManager) noexcept;

	bool FindWork() noexcept;

	void ThreadFunc() noexcept;
	void DoWork(CSftmChainController* pChainController) noexcept;
	void Idle() noexcept;

private:
	CSftmTaskManager* m_pTaskManager = { nullptr };

	std::thread		m_thread;
	volatile bool	m_bFinished = { false };
	volatile bool	m_bStopping = { true };

	CSftmConcurrentPtrQueue<CSftmTask>	m_taskQueue;
	CSftmChainController*				m_pCurrentChainController = { nullptr };

	CSftmPrivateHeapManager	m_privateHeapManager;
	CSftmRawMemoryManager	m_rawMemoryManager;
};
