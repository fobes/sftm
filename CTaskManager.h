#pragma once
#include "CWorker.h"
#include "Export.h"

#define MAX_WORKERS	32

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
	CWorker m_workers[MAX_WORKERS];
	unsigned short m_nNumberOfWorkers;

	bool m_bStopping;

public:
	static DWORD m_nTlsWorker;
}; 


