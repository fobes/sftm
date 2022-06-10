#include "../include/Sync/CTaskManager.hpp"
#include "../include/Async/CAsyncTaskManager.hpp"
#include "../include/Common/Utils.hpp"

#include <iostream>
#include <chrono>

void SyncTest() noexcept
{
	constexpr std::uint32_t nStartTaskCount = 100;
	constexpr std::uint32_t nMidTaskCount	= 100;
	constexpr std::uint32_t nEndTaskCount	= 100;
	constexpr std::uint32_t nTaskPlanCount	= nStartTaskCount * nMidTaskCount * nEndTaskCount + nMidTaskCount * nEndTaskCount + nEndTaskCount;
	constexpr std::uint32_t nTestCount		= 100;

	static std::atomic<std::uint32_t> nExecutedTasks = { 0 };

	class CSyncEndTask : public sftm::CTask
	{
	public:
		CSyncEndTask(sftm::CChainController& chainController) :sftm::CTask(chainController) {}
		virtual ~CSyncEndTask() {}

	public:
		virtual void Execute(sftm::CWorker& worker) noexcept override
		{
			nExecutedTasks++;
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
			STACK_MEMORY(CSyncEndTask, nEndTaskCount);
			sftm::CChainController chainController;

			for (std::uint32_t n = 0; n < nMidTaskCount; n++)
			{
				if (!worker.PushTask(new(pAddress++) CSyncEndTask(chainController)))
				{
					std::cout << "	Task not taken\n";
					break;
				}
			}

			worker.WorkUntil(chainController);

			nExecutedTasks++;
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
			STACK_MEMORY(CSyncMidTask, nMidTaskCount);
			sftm::CChainController chainController;

			for (std::uint32_t n = 0; n < nMidTaskCount; n++)
			{
				if (!worker.PushTask(new(pAddress++) CSyncMidTask(chainController)))
				{
					std::cout << "	Task not taken\n";
					break;
				}
			}

			worker.WorkUntil(chainController);

			nExecutedTasks++;
		}
	};

	auto pTaskManager(std::make_unique<sftm::CTaskManager>());

	pTaskManager->Start(std::thread::hardware_concurrency());
	std::cout << "Sync manager started on " << pTaskManager->GetWorkersCount() << " threads\n";

	auto& currentWorker = pTaskManager->GetMainWorker();

	std::cout << "Tests:\n";

	std::uint32_t nAverageTime = 0, nMaxTime = 0, nMinTime = 0xFFFFFFFF;

	for (std::uint32_t n = 0; n < nTestCount; ++n)
	{
		nExecutedTasks.store(0);

		auto t1 = std::chrono::high_resolution_clock::now();

		STACK_MEMORY(CSyncStartTask, nStartTaskCount);
		sftm::CChainController chainController;

		for (std::uint32_t n = 0; n < nStartTaskCount; n++)
		{
			if (!currentWorker.PushTask(new(pAddress++) CSyncStartTask(chainController)))
			{
				std::cout << "	Task not taken\n";
				break;
			}
		}
		currentWorker.WorkUntil(chainController);

		auto t2 = std::chrono::high_resolution_clock::now();

		std::cout << n << ":	Planned: " << nTaskPlanCount << "; Execued: " << nExecutedTasks << ";";

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
	std::cout << "\n	Avg time on task:	" << (float)nAvarageTime / (float)nTaskPlanCount * 1000.0f << "	nS\n";


	pTaskManager->Stop();
	std::cout << "\n	Sync manager stopped\n";
}
void AsyncTest() noexcept
{
	constexpr std::uint32_t nTaskPlanCount = 100;

	static std::atomic<uint32_t> nExecutedTasks = { 0 };

	class CTestTask : public sftm::CAsyncTask
	{
	public:
		virtual ~CTestTask() {}

	public:
		void Execute() noexcept override
		{
			++nExecutedTasks;

			delete this;
		}
	};

	auto pTaskManager = std::make_unique<sftm::CAsyncTaskManager>();
	pTaskManager->Start(std::thread::hardware_concurrency());
	std::cout << "\n\n\nAsync manager started on " << pTaskManager->GetWorkersCount() << " threads\n";

	auto t1 = std::chrono::high_resolution_clock::now();

	for(std::uint32_t n = 0; n < nTaskPlanCount; ++n)
	{
		pTaskManager->PushTask(new CTestTask);
	}

	while(nExecutedTasks != nTaskPlanCount){}

	auto t2 = std::chrono::high_resolution_clock::now();
	auto nUsCount = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "	Planned tasks: " << nTaskPlanCount << "; Execued tasks: " << nExecutedTasks << ";";
	std::cout << "\n	Execution time:		" << nUsCount << "	uS";

	pTaskManager->Stop();
	std::cout << "\n	Async manager stopped\n";
}

int main()
{
	SyncTest();
	AsyncTest();

	system("pause");

	return 0;
}
