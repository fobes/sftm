#include "CProfiler.h"
#include <thread>
#include <string>

#ifdef _PROFILE

CProfiler::CProfiler() noexcept
{

}

CProfiler::~CProfiler()
{

}

void CProfiler::Run() noexcept
{
	m_collect.store(!m_collect.load());
}

void CProfiler::InsertItem(const CItem& item) noexcept
{
	if (!m_collect.load())
		return;

	m_items.push_back(item);
}

void CProfiler::Save(std::ofstream& file, unsigned nThread) noexcept
{
	file << "\n<thread id = \"" + std::to_string(nThread) + "\">";

	for (size_t n = 0; n < m_items.size(); n++)
	{
		CItem& item = m_items[n];

		file << "\n<item type=\"" + std::to_string((int)item.m_type) + "\" ";
		file << "idx=\"" + std::to_string(item.m_nIndex) + "\" ";
		file << "tm_s=\"" + std::to_string(item.m_nStartTime) + "\" ";
		file << "tm_e=\"" + std::to_string(item.m_nEndTime) + "\"/>";
	}

	file << "\n</thread>";
}

#endif
