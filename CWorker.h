#pragma once
#include "Export.h"
#include <mutex>
#include <thread>

#define MAX_WORKER_TASKS_QUEUE		256
#define WORKER_PRIVATE_HEAP_SIZE	(16*1024*1024)
#define WORKER_RAW_MEMORY_SIZE		(16*1024*1024)

class CTaskManager;
class CTask;
class CTaskCounter;

class TM_API CWorker
{
	friend class CTaskManager;

public:
	class TM_API CPrivateHeapManager
	{
	public:
		CPrivateHeapManager() noexcept;

	public:
		bool Create() noexcept;

		void* Alloc(size_t nSize) noexcept;
		void Free(void* pData) noexcept;

	private:
		SLIST_HEADER m_FreeList;
		HANDLE m_hHeap;
	};
	class TM_API CRawMemoryManager
	{
	public:
		class TM_API CRawMemory
		{
		public:
			CRawMemory() noexcept;
			~CRawMemory();

		public:
			bool AllocBytes(size_t nSize) noexcept;

		public:
			void* m_pData;
			size_t m_Size;
		};

	public:
		CRawMemoryManager() noexcept;

	public:
		bool Create() noexcept;

		void* Alloc(size_t nSize) noexcept;
		void Free(size_t nSize) noexcept;

	private:
		BYTE* m_pMemory;
		size_t m_nUsedCount;
	};

private:
	class CTaskQueue
	{
	public:
		CTaskQueue() noexcept;
		~CTaskQueue();

	public:
		bool PushTask(CTask *pTask) noexcept;
		CTask* PopTask() noexcept;

		bool TryStealTasks(CTaskQueue &srcQueue) noexcept;

		bool TasksIsExist() noexcept;

	public:
		CTask* m_pTasks[MAX_WORKER_TASKS_QUEUE];
		unsigned m_TaskCount;

		std::mutex m_mutex;
	};

public:
	CWorker() noexcept;
	~CWorker();

public:
	bool Start(CTaskManager* pTaskManager) noexcept;

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

private:
	CTaskManager *m_pTaskManager;

	std::thread m_thread;
	volatile bool m_bFinished;
	
	CTaskQueue m_taskQueue;
	CTaskCounter *m_pCurrentTaskCounter;

	CPrivateHeapManager m_privateHeapManager;
	CRawMemoryManager m_rawMemoryManager;
};
