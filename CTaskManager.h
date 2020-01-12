#pragma once
#include "CWorker.h"
#include "Export.h"

#define MAX_WORKERS	32

class TM_API CTaskManager
{
public:
	CTaskManager();
	~CTaskManager();

public:
	bool Start(int nNumberOfWorkers);
	void Stop();

	unsigned short GetWorkersCount();

public:
	CWorker m_workers[MAX_WORKERS];
	unsigned short m_nNumberOfWorkers;

	bool m_bStopping;

public:
	static DWORD m_nTlsWorker;
}; 


