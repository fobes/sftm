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
		enum class EType { EAsyncTaskExecution = 0, ESyncTaskExecution = 1, EIdle = 2, ETaskFinding = 3 };

		EType		m_type;
		TimeStamp	m_nStartTime;
		TimeStamp	m_nEndTime;
	};

public:
	CProfiler() noexcept;
	~CProfiler();

public:
	void InsertItem(const CItem& item) noexcept;

	void Save(std::ofstream& file, unsigned nThread) noexcept;

public:
	static void CollectNullTime() noexcept;

private:
	static CProfiler::CItem::TimeStamp m_nullTime;

	std::vector<CItem>	m_items;
};

#define START_PROFILE(EType)																														\
	CProfiler::CItem item;																															\
	item.m_type = EType;																															\
	item.m_nStartTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

#define END_PROFILE()																																\
	item.m_nEndTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();	\
	m_profiler.InsertItem(item);

#define COLLECT_NULL_TIME()																															\
	m_nullTime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();

#endif
