#include "CTaskManager.h"

DWORD   CTaskManager::m_nTlsWorker;

CTaskManager::CTaskManager() noexcept :m_nNumberOfWorkers(0)
{

}

CTaskManager::~CTaskManager()
{
	Stop();
}

CTaskManager& CTaskManager::GetInstance() noexcept
{
	static CTaskManager manager;
	return manager;
}

bool CTaskManager::Start(unsigned short nNumberOfWorkers) noexcept
{
	if (nNumberOfWorkers > MAX_WORKERS)
		nNumberOfWorkers = MAX_WORKERS;
	if (nNumberOfWorkers < 1)
		nNumberOfWorkers = 1;

	m_nTlsWorker = TlsAlloc();
	if (m_nTlsWorker == TLS_OUT_OF_INDEXES)
		return false;

	if (!m_workers[0].Init(this) || !TlsSetValue(this->m_nTlsWorker, this))
		return false;

	for (unsigned nWorker = 1; nWorker <= nNumberOfWorkers; nWorker++)
	{
		if (!AddWorker())
			return false;
	}

	return true;
}

void CTaskManager::Stop() noexcept
{
	for (unsigned short nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
		m_workers[nWorker].Stop();

	m_cvWorkerIdle.notify_all();

	for (unsigned short nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
	{
		while (!m_workers[nWorker].IsFinished())
		{

		}
	}

	TlsFree(m_nTlsWorker);
	m_nTlsWorker = TLS_OUT_OF_INDEXES;
}

bool CTaskManager::AddWorker() noexcept
{
	if (m_nNumberOfWorkers + 1 >= MAX_WORKERS)
		return false;

	if (!m_workers[m_nNumberOfWorkers + 1].Start(this))
		return false;

	m_nNumberOfWorkers++;

	return true;
}

void CTaskManager::RemoveWorker() noexcept
{
	m_workers[m_nNumberOfWorkers].Stop();

	while (!m_workers[m_nNumberOfWorkers].IsFinished())
	{
	}

	m_workers[m_nNumberOfWorkers].ReleaseResources();

	m_nNumberOfWorkers--;
}

unsigned short CTaskManager::GetWorkersCount() const noexcept
{
	return m_nNumberOfWorkers;
}

