#include "CSftmTaskManager.h"
#include <filesystem>

DWORD CSftmTaskManager::m_nTlsWorker;

CSftmTaskManager::CSftmTaskManager() noexcept :m_nNumberOfWorkers(0)
{

}

CSftmTaskManager::~CSftmTaskManager()
{
	Stop();

#ifdef _PROFILE
	SaveFileProfiler();
#endif
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

#ifdef _PROFILE
	CProfiler::CollectNullTime();
#endif

	return true;
}

void CSftmTaskManager::Stop() noexcept
{
	for (unsigned short nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
		m_workers[nWorker].Stop();

	for (unsigned short nWorker = 1; nWorker < m_nNumberOfWorkers; nWorker++)
	{
		while (!m_workers[nWorker].IsFinished())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			m_cvWorkerIdle.notify_all();
		}
	}

	TlsFree(m_nTlsWorker);
	m_nTlsWorker = TLS_OUT_OF_INDEXES;
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

unsigned CSftmTaskManager::GetWorkersCount() const noexcept
{
	return m_nNumberOfWorkers;
}

#ifdef _PROFILE
void CSftmTaskManager::SaveFileProfiler() noexcept
{
	const auto outFilePath = std::filesystem::current_path() / "sftm_profiler.xml";

	std::ofstream file;
	file.open(outFilePath);
	file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	file << "\n<root>";

	for (unsigned nWorker = 0; nWorker < m_nNumberOfWorkers; nWorker++)
	{
		m_workers[nWorker].m_profiler.Save(file, nWorker);
	}

	file << "<\n/root>";

	file.close();
}
#endif
