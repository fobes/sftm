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
		CPrivateHeapManager();

	public:
		bool Create();

		void* Alloc(size_t nSize);
		void Free(void* pData);

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
			CRawMemory();
			~CRawMemory();

		public:
			bool AllocBytes(size_t sz);

		public:
			void* m_pData;
			size_t m_Size;
		};

	public:
		CRawMemoryManager();

	public:
		bool Create();

		void* Alloc(size_t nSize);
		void Free(size_t nSize);

	private:
		BYTE* m_pMemory;
		size_t m_nUsedCount;
	};

private:
	class CTaskQueue
	{
	public:
		CTaskQueue();
		~CTaskQueue();

	public:
		bool PushTask(CTask *pTask);
		CTask* PopTask();

		bool TryStealTasks(CTaskQueue &srcQueue);

		bool TasksIsExist();

	public:
		CTask* m_pTasks[MAX_WORKER_TASKS_QUEUE];
		unsigned m_TaskCount;

		std::mutex m_mutex;
	};

public:
	CWorker();
	~CWorker();

public:
	bool Start(CTaskManager* pTaskManager);

public:
	static CWorker* GetCurrentThreadWorker();
	int GetWorkerIndex() const;
	bool IsFinished() const;

public:
	bool PushTask(CTask *pTask);
	CTaskCounter* GetCurrentTaskCounter();

	void WorkUntil(CTaskCounter &taskCounter);

public:
	CPrivateHeapManager& GetPrivateHeapManager();
	CRawMemoryManager& GetRawMemoryManager();

private:
	bool Init(CTaskManager* pTaskManager);

	bool FindWork();

	void ThreadFunc();
	void DoWork();

private:
	CTaskManager *m_pTaskManager;

	std::thread m_thread;
	volatile bool m_bFinished;
	
	CTaskQueue m_taskQueue;
	CTaskCounter *m_pCurrentTaskCounter;

	CPrivateHeapManager m_privateHeapManager;
	CRawMemoryManager m_rawMemoryManager;
};
