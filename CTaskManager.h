#pragma once
#include "CWorker.h"
#include "Export.h"
#include <array>

#define MAX_WORKERS	128

class TM_API CTaskManager
{
private:
	CTaskManager() noexcept;
	CTaskManager(const CTaskManager&) = delete;
	void operator=(const CTaskManager&) = delete;
	CTaskManager(CTaskManager&&) = delete;
	CTaskManager& operator=(CTaskManager&&) = delete;
	~CTaskManager();

public:
	static CTaskManager& GetInstance() noexcept;

public:
	bool Start(unsigned short nNumberOfWorkers) noexcept;
	void Stop() noexcept;

	bool AddWorker();
	void RemoveWorker();

	unsigned short GetWorkersCount() const;

public:
	std::array<CWorker, MAX_WORKERS> m_workers;
	unsigned short m_nNumberOfWorkers;

public:
	static DWORD m_nTlsWorker;
}; 


