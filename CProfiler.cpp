#include "CProfiler.h"
#include <thread>
#include <string>

#ifdef _PROFILE

CProfiler::CItem::TimeStamp CProfiler::m_nullTime = 0;

CProfiler::CProfiler() noexcept
{

}

CProfiler::~CProfiler()
{

}

void CProfiler::InsertItem(const CItem& item) noexcept
{
	m_items.push_back(item);
}

void CProfiler::Save(std::ofstream& file, unsigned nThread) noexcept
{
	file << "\n<thread id = \"" + std::to_string(nThread) + "\">";

	for (size_t n = 0; n < m_items.size(); n++)
	{
		CItem& item = m_items[n];

		file << "\n<item type=\"" + std::to_string((int)item.m_type) + "\" ";
		file << "tm_s=\"" + std::to_string(item.m_nStartTime - m_nullTime) + "\" ";
		file << "tm_e=\"" + std::to_string(item.m_nEndTime - m_nullTime) + "\"/>";
	}

	file << "\n</thread>";
}

void CProfiler::CollectNullTime() noexcept
{
	COLLECT_NULL_TIME();
}

#endif
