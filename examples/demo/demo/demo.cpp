#include <iostream>
#include "CTaskManager.hpp"
#include "Utils.hpp"
#include <chrono>

constexpr int nSyncStartTaskCount	= 100;
constexpr int nSyncMidTaskCount		= 100;
constexpr int nSyncEndTaskCount		= 100;
constexpr int nSyncTaskPlanCount	= nSyncStartTaskCount * nSyncMidTaskCount * nSyncEndTaskCount + nSyncMidTaskCount * nSyncEndTaskCount + nSyncEndTaskCount;

constexpr int nTestCount = 100;

std::atomic<long long int> nExecutedSyncTasks = { 0 };

int main()
{
	class CSyncEndTask : public sftm::CTask
	{
	public:
		CSyncEndTask(sftm::CChainController& сhainController) :sftm::CTask(сhainController) {}
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
		CSyncMidTask(sftm::CChainController& сhainController) :sftm::CTask(сhainController) {}
		virtual ~CSyncMidTask() {}

	public:
		virtual void Execute(sftm::CWorker& worker) noexcept override
		{
			STACK_MEMORY(CSyncEndTask, nSyncEndTaskCount);
			sftm::CChainController chainController;
			for (int n = 0; n < nSyncMidTaskCount; n++)
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
		CSyncStartTask(sftm::CChainController& сhainController) :sftm::CTask(сhainController) {}
		virtual ~CSyncStartTask() {}

	public:
		virtual void Execute(sftm::CWorker& worker) noexcept override
		{
			STACK_MEMORY(CSyncMidTask, nSyncMidTaskCount);
			sftm::CChainController chainController;
			for (int n = 0; n < nSyncMidTaskCount; n++)
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

	std::unique_ptr<sftm::CTaskManager> pTaskManager(new sftm::CTaskManager());

	if (pTaskManager->Start(std::thread::hardware_concurrency()))
		std::cout << "Manager started on " << pTaskManager->GetWorkersCount() << " threads\n";
	else
		std::cout << "Manager could not start\n";

	auto pCurrentWorker = pTaskManager->GetCurrentThreadWorker();
	if(!pCurrentWorker)
		std::cout << "	Current thread worker not received\n";

	std::cout << "Tests:\n";

	long long nAverageTime = 0;

	for (int n = 0; n < nTestCount; ++n)
	{
		nExecutedSyncTasks.store(0);

		auto t1 = std::chrono::high_resolution_clock::now();

		STACK_MEMORY(CSyncStartTask, nSyncStartTaskCount);
		sftm::CChainController chainController;
		for (int n = 0; n < nSyncStartTaskCount; n++)
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
	}

	auto nAvarageTime = nAverageTime / nTestCount;
	std::cout << "\n	Average time:		" << nAvarageTime << "	uS\n";
	std::cout << "	Average time on task:	" << (float)nAvarageTime / (float)nSyncTaskPlanCount * 1000.0f << "	nS\n";

	
	pTaskManager->Stop();
	std::cout << "\n	Manager stopped\n";

	return 0;
}
