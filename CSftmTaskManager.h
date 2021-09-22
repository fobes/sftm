#pragma once
#include "CSftmWorker.h"
#include <array>
#include <functional>

#define MAX_WORKERS	128

class TM_API CSftmTaskManager
{
	friend CSftmWorker;

public:
	CSftmTaskManager() noexcept;
	CSftmTaskManager(const CSftmTaskManager&) = delete;
	void operator=(const CSftmTaskManager&) = delete;
	CSftmTaskManager(CSftmTaskManager&&) = delete;
	CSftmTaskManager& operator=(CSftmTaskManager&&) = delete;
	~CSftmTaskManager();

public:
	static CSftmTaskManager& GetInstance() noexcept;

public:
	bool Start(unsigned short nNumberOfWorkers) noexcept;
	void Stop() noexcept;

	bool AddWorker() noexcept;
	void RemoveWorker() noexcept;

	unsigned GetWorkersCount() const noexcept;

	void RunProfiling() noexcept;

private:
#ifdef _PROFILE
	void SaveFileProfiler() noexcept;
#endif

private:
	std::array<CSftmWorker, MAX_WORKERS> m_workers;
	unsigned m_nWorkerCount = 0;

private:
	std::condition_variable m_cvWorkerIdle;
	std::mutex m_mtxWorkerIdle;

private:
	static DWORD m_nTlsWorker;
};
