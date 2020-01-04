#pragma once
#include "Export.h"
#include <mutex>
#include <thread>

#define MAX_WORKER_TASKS_QUEUE		256				//Максимальная длина очереди задач рабочего
#define WORKER_PRIVATE_HEAP_SIZE	(16*1024*1024)	//Размер частной кучи рабочего
#define WORKER_RAW_MEMORY_SIZE		(16*1024*1024)	//Резерв памяти рабочего

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
	//Начать работать
	bool Start(CTaskManager* pTaskManager);

public:
	//Получить рабочего текущего потока
	static CWorker* GetCurrentThreadWorker();
	//Получить индекс рабочего
	int GetWorkerIndex() const;
	//Закончил ли рабочий
	bool IsFinished() const;

public:
	//Поместить задачу в очередь
	bool PushTask(CTask *pTask);
	//Получить текущий счетчик задач
	CTaskCounter* GetCurrentTaskCounter();

	//Работаем пока счетчик не пуст
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
	//Родитель
	CTaskManager *m_pTaskManager;

	//Работа с потоком
	std::thread m_thread;
	volatile bool m_bFinished;
	
	//Очередь задачь
	CTaskQueue m_taskQueue;
	//Текущий счетчик задач
	CTaskCounter *m_pCurrentTaskCounter;

	//Частная куча
	CPrivateHeapManager m_privateHeapManager;
	//Зарезервированная память из системы
	CRawMemoryManager m_rawMemoryManager;
};
