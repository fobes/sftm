#include "CSftmTaskManager.h"

DWORD CSftmTaskManager::m_nTlsWorker;

CSftmTaskManager::CSftmTaskManager() noexcept :m_nNumberOfWorkers(0)
{

}

CSftmTaskManager::~CSftmTaskManager()
{
	Stop();
}

CSftmTaskManager& CSftmTaskManager::GetInstance() noexcept
{
	static CSftmTaskManager manager;
	return manager;
}

bool CSftmTaskManager::Start(unsigned short nNumberOfWorkers, CWorkerFirstFunc&& workerFirstFunc) noexcept
{
	if (nNumberOfWorkers > MAX_WORKERS)
		nNumberOfWorkers = MAX_WORKERS;
	if (nNumberOfWorkers < 1)
		nNumberOfWorkers = 1;

	m_workerFirstFunc = std::move(workerFirstFunc);

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

void CSftmTaskManager::Stop() noexcept
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

	m_nNumberOfWorkers = 0;
}

bool CSftmTaskManager::AddWorker() noexcept
{
	if (m_nNumberOfWorkers + 1 >= MAX_WORKERS)
		return false;

	if (!m_workers[m_nNumberOfWorkers + 1].Start(this))
		return false;

	m_nNumberOfWorkers++;

	return true;
}

void CSftmTaskManager::RemoveWorker() noexcept
{
	m_workers[m_nNumberOfWorkers].Stop();

	while (!m_workers[m_nNumberOfWorkers].IsFinished())
	{
	}

	m_workers[m_nNumberOfWorkers].ReleaseResources();

	m_nNumberOfWorkers--;
}

unsigned short CSftmTaskManager::GetWorkersCount() const noexcept
{
	return m_nNumberOfWorkers;
}
