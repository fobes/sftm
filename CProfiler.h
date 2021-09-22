#pragma once
#include <vector>
#include <fstream>

#ifdef _PROFILE

class CProfiler
{
public:
	struct CItem
	{
		using TimeStamp = long long;
		enum class EType { ETaskPop = 0, ETaskExecution = 1, ETaskFinding = 2, EIdle = 3 };

		EType		m_type;
		unsigned	m_nIndex;
		TimeStamp	m_nStartTime;
		TimeStamp	m_nEndTime;
	};

public:
	CProfiler() noexcept;
	~CProfiler();

public:
	void Run() noexcept;

	void InsertItem(const CItem& item) noexcept;

	void Save(std::ofstream& file, unsigned nThread) noexcept;

private:
	std::atomic<bool> m_collect = false;

	std::vector<CItem>	m_items;
};

#define START_PROFILE(EType, Idx)																														\
	CProfiler::CItem item;																																\
	item.m_type = EType;																																\
	item.m_nIndex = Idx;																																\
	item.m_nStartTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

#define END_PROFILE()																																\
	item.m_nEndTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();	\
	m_profiler.InsertItem(item);

#endif
