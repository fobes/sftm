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

	float fManagerTaskTime = 0;
	{
		static std::atomic<std::uint32_t> nExecutedTasks = 0;

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
				sftm::CChainController chainController(nEndTaskCount);
				for (std::uint32_t n = 0; n < nEndTaskCount; n++)
					new(pAddress + n) CSyncEndTask(chainController);

				worker.PushTask(pAddress, nEndTaskCount);
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
				sftm::CChainController chainController(nMidTaskCount);
				for (std::uint32_t n = 0; n < nMidTaskCount; ++n)
					new(pAddress + n) CSyncMidTask(chainController);

				worker.PushTask(pAddress, nMidTaskCount);
				worker.WorkUntil(chainController);

				nExecutedTasks++;
			}
		};

		auto pTaskManager(std::make_unique<sftm::CTaskManager>());
		
		pTaskManager->Start(std::thread::hardware_concurrency() - 1);
		std::cout << "Sync manager started on " << pTaskManager->GetWorkersCount() << " threads\n";

		auto& currentWorker = pTaskManager->GetMainWorker();

		std::cout << "	Tests:			" << nTestCount << "	iterations\n";

		std::uint32_t nAverageTime = 0, nMaxTime = 0, nMinTime = 0xFFFFFFFF;

		for (std::uint32_t n = 0; n < nTestCount; ++n)
		{
			nExecutedTasks.store(0);

			auto t1 = std::chrono::high_resolution_clock::now();

			STACK_MEMORY(CSyncStartTask, nStartTaskCount);
			sftm::CChainController chainController(nStartTaskCount);
			for (std::uint32_t n = 0; n < nStartTaskCount; ++n)
				new(pAddress + n) CSyncStartTask(chainController);

			currentWorker.PushTask(pAddress, nStartTaskCount);
			currentWorker.WorkUntil(chainController);

			auto t2 = std::chrono::high_resolution_clock::now();
			auto nUsCount = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
			if(nTaskPlanCount != nExecutedTasks)
				std::cout << n << "	Planned: " << nTaskPlanCount << "; Execued: " << nExecutedTasks << ";\n";

			nAverageTime += nUsCount;
			if (nMaxTime < nUsCount)
				nMaxTime = nUsCount;
			if (nMinTime > nUsCount)
				nMinTime = nUsCount;
		}

		auto nAvarageTime		= nAverageTime / nTestCount;
		fManagerTaskTime = (float)nAvarageTime / (float)nTaskPlanCount * 1000.0f;
		std::cout << "	Max time:		"		<< nMaxTime				<<	"	uS\n";
		std::cout << "	Avg time:		"		<< nAvarageTime			<<	"	uS\n";
		std::cout << "	Min time:		"		<< nMinTime				<<	"	uS\n";
		std::cout << "	Avg time on task:	"	<< fManagerTaskTime		<< "	nS\n";
	
		pTaskManager->Stop();
	}

	float fSingleThreadTime = 0;
	{
		std::cout << "\nSingle thread the same task start\n";

		std::cout << "	Tests:			" << nTestCount << "	iterations\n";

		static std::atomic<std::uint32_t> nExecutedTasks = 0;
		std::uint32_t nAverageTime = 0, nMaxTime = 0, nMinTime = 0xFFFFFFFF;

		for (std::uint32_t n = 0; n < nTestCount; ++n)
		{
			nExecutedTasks.store(0);

			auto tl1 = std::chrono::high_resolution_clock::now();

			for (std::uint32_t n = 0; n < nTaskPlanCount; ++n)
				++nExecutedTasks;

			auto tl2 = std::chrono::high_resolution_clock::now();
			auto nUsCount = std::chrono::duration_cast<std::chrono::microseconds>(tl2 - tl1).count();

			nAverageTime += nUsCount;
			if (nMaxTime < nUsCount)
				nMaxTime = nUsCount;
			if (nMinTime > nUsCount)
				nMinTime = nUsCount;
		}

		auto nAvarageTime = nAverageTime / nTestCount;
		fSingleThreadTime = (float)nAvarageTime / (float)nTaskPlanCount * 1000.0f;
		std::cout << "	Max time:		"		<< nMaxTime				<< "	uS\n";
		std::cout << "	Avg time:		"		<< nAvarageTime			<< "	uS\n";
		std::cout << "	Min time:		"		<< nMinTime				<< "	uS\n";
		std::cout << "	Avg time on task:	"	<< fSingleThreadTime	<< "	nS\n";
	}

	std::cout << "	\n	Average task overhead:	" << fManagerTaskTime - fSingleThreadTime <<"	nS\n";

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
