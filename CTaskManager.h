#pragma once
#include "CWorker.h"
#include "Export.h"
#include <array>

#define MAX_WORKERS	128

class TM_API CTaskManager
{
private:
	CTaskManager() noexcept;
	~CTaskManager();

public:
	static CTaskManager& GetInstance() noexcept;

public:
	bool Start(int nNumberOfWorkers) noexcept;
	void Stop() noexcept;

	unsigned short GetWorkersCount() const;

public:
	std::array<CWorker, MAX_WORKERS> m_workers;
	unsigned short m_nNumberOfWorkers;

	bool m_bStopping;

public:
	static DWORD m_nTlsWorker;
}; 


