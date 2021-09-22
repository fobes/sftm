#include "CSftmTaskManager.h"
#include <filesystem>

DWORD CSftmTaskManager::m_nTlsWorker;

CSftmTaskManager::CSftmTaskManager() noexcept
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

bool CSftmTaskManager::Start(unsigned short nNumberOfWorkers) noexcept
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

void CSftmTaskManager::Stop() noexcept
{
	for (unsigned short nWorker = 1; nWorker < m_nWorkerCount; nWorker++)
		m_workers[nWorker].Stop();

	for (unsigned short nWorker = 1; nWorker < m_nWorkerCount; nWorker++)
	{
		while (!m_workers[nWorker].IsFinished())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			m_cvWorkerIdle.notify_all();
		}
	}

	TlsFree(m_nTlsWorker);
	m_nTlsWorker = TLS_OUT_OF_INDEXES;

#ifdef _PROFILE
	SaveFileProfiler();
#endif
}

bool CSftmTaskManager::AddWorker() noexcept
{
	if (m_nWorkerCount + 1 >= MAX_WORKERS)
		return false;

	if (!m_workers[m_nWorkerCount + 1].Start(this))
		return false;

	m_nWorkerCount++;

	return true;
}

void CSftmTaskManager::RemoveWorker() noexcept
{
	m_workers[m_nWorkerCount].Stop();

	while (!m_workers[m_nWorkerCount].IsFinished())
	{
	}

	m_workers[m_nWorkerCount].ReleaseResources();

	m_nWorkerCount--;
}

unsigned CSftmTaskManager::GetWorkersCount() const noexcept
{
	return m_nWorkerCount;
}

void CSftmTaskManager::RunProfiling() noexcept
{
#ifdef _PROFILE
	for (unsigned n = 0; n < m_nWorkerCount; n++)
	{
		m_workers[n].RunProfiling();
	}

#endif
}

#ifdef _PROFILE
void CSftmTaskManager::SaveFileProfiler() noexcept
{
	const auto outFilePath = std::filesystem::current_path() / "sftm_profiler.xml";

	std::ofstream file;
	file.open(outFilePath);
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	file << "\n<root>";

	for (unsigned nWorker = 0; nWorker < m_nWorkerCount; nWorker++)
	{
		m_workers[nWorker].m_profiler.Save(file, nWorker);
	}

	file << "\n</root>";

	file.close();
}
#endif
