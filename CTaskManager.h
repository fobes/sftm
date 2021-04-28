#pragma once
#include "CWorker.h"
#include <array>
#include <functional>

#define MAX_WORKERS	128

class TM_API CTaskManager
{
	friend CWorker;

	using CWorkerFirstFunc = std::function<void(void)>;

public:
	CTaskManager() noexcept;
	CTaskManager(const CTaskManager&) = delete;
	void operator=(const CTaskManager&) = delete;
	CTaskManager(CTaskManager&&) = delete;
	CTaskManager& operator=(CTaskManager&&) = delete;
	~CTaskManager();

public:
	static CTaskManager& GetInstance() noexcept;

public:
	bool Start(unsigned short nNumberOfWorkers, CWorkerFirstFunc&& workerFirstFunc) noexcept;
	void Stop() noexcept;

	bool AddWorker() noexcept;
	void RemoveWorker() noexcept;

	unsigned short GetWorkersCount() const noexcept;

private:
	std::array<CWorker, MAX_WORKERS> m_workers;
	unsigned short m_nNumberOfWorkers;

	CWorkerFirstFunc m_workerFirstFunc;

private:
	std::condition_variable m_cvWorkerIdle;
	std::mutex m_mtxWorkerIdle;

private:
	static DWORD m_nTlsWorker;
};
