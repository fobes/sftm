#include "../include/CTaskManager.hpp"
#include "../include/Utils.hpp"

#include <iostream>
#include <chrono>

const std::uint32_t nSyncStartTaskCount	= 100;
const std::uint32_t nSyncMidTaskCount		= 100;
const std::uint32_t nSyncEndTaskCount		= 100;
const std::uint32_t nSyncTaskPlanCount	= nSyncStartTaskCount * nSyncMidTaskCount * nSyncEndTaskCount + nSyncMidTaskCount * nSyncEndTaskCount + nSyncEndTaskCount;

const std::uint32_t nTestCount = 100;

std::atomic<std::uint32_t> nExecutedSyncTasks = { 0 };

class CSyncEndTask : public sftm::CTask
{
public:
	CSyncEndTask(sftm::CChainController& chainController) :sftm::CTask(chainController) {}
	virtual ~CSyncEndTask() {}

public:
	virtual void Execute(sftm::CWorker& worker) noexcept override
	{
		nExecutedSyncTasks++;
	}
};
class CSyncMidTask : public sftm::CTask
{
public:
	CSyncMidTask(sftm::CChainController& chainController) :sftm::CTask(chainController) {}
	virtual ~CSyncMidTask() {}

public:
	virtual void Execute(sftm::CWorker& worker) noexcept override
	{
		STACK_MEMORY(CSyncEndTask, nSyncEndTaskCount);
		sftm::CChainController chainController;

		for (std::uint32_t n = 0; n < nSyncMidTaskCount; n++)
		{
			if (!worker.PushTask(new(pAddress++) CSyncEndTask(chainController)))
			{
				std::cout << "	Task not taken\n";
				break;
			}
		}

		worker.WorkUntil(chainController);

		nExecutedSyncTasks++;
	}
};

class CSyncStartTask : public sftm::CTask
{
public:
	CSyncStartTask(sftm::CChainController& chainController) :sftm::CTask(chainController) {}
	virtual ~CSyncStartTask() {}

public:
	virtual void Execute(sftm::CWorker& worker) noexcept override
	{
		STACK_MEMORY(CSyncMidTask, nSyncMidTaskCount);
		sftm::CChainController chainController;

		for (std::uint32_t n = 0; n < nSyncMidTaskCount; n++)
		{
			if (!worker.PushTask(new(pAddress++) CSyncMidTask(chainController)))
			{
				std::cout << "	Task not taken\n";
				break;
			}
		}

		worker.WorkUntil(chainController);

		nExecutedSyncTasks++;
	}
};

int main()
{
	auto pTaskManager(std::make_unique<sftm::CTaskManager>());

	if (pTaskManager->Start(std::thread::hardware_concurrency()))
		std::cout << "Manager started on " << pTaskManager->GetWorkersCount() << " threads\n";
	else
		std::cout << "Manager could not start\n";

	auto pCurrentWorker = pTaskManager->GetCurrentThreadWorker();

	if(!pCurrentWorker)
		std::cout << "	Current thread worker not received\n";

	std::cout << "Tests:\n";

	std::uint32_t nAverageTime = 0, nMaxTime = 0, nMinTime = 0xFFFFFFFF;

	for (std::uint32_t n = 0; n < nTestCount; ++n)
	{
		nExecutedSyncTasks.store(0);

		auto t1 = std::chrono::high_resolution_clock::now();

		STACK_MEMORY(CSyncStartTask, nSyncStartTaskCount);
		sftm::CChainController chainController;

		for (std::uint32_t n = 0; n < nSyncStartTaskCount; n++)
		{
			if (!pCurrentWorker->PushTask(new(pAddress++) CSyncStartTask(chainController)))
			{
				std::cout << "	Task not taken\n";
				break;
			}
		}
		pCurrentWorker->WorkUntil(chainController);

		auto t2 = std::chrono::high_resolution_clock::now();

		std::cout << n << ":	Planned: " << nSyncTaskPlanCount << "; Execued: " << nExecutedSyncTasks << ";";

		auto nUsCount = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		std::cout << " Time: " << nUsCount << " uS;\n";

		nAverageTime += nUsCount;
		if (nMaxTime < nUsCount)
			nMaxTime = nUsCount;
		if (nMinTime > nUsCount)
			nMinTime = nUsCount;
	}

	auto nAvarageTime = nAverageTime / nTestCount;
	std::cout << "\n	Max time:		" << nMaxTime << "	uS";
	std::cout << "\n	Avg time:		" << nAvarageTime << "	uS";
	std::cout << "\n	Min time:		" << nMinTime << "	uS";
	std::cout << "\n	Avg time on task:	" << (float)nAvarageTime / (float)nSyncTaskPlanCount * 1000.0f << "	nS\n";

	
	pTaskManager->Stop();
	std::cout << "\n	Manager stopped\n";

	return 0;
}
