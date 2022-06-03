#pragma once
#include <Windows.h>

namespace sftm
{
	class CCriticalSectionLock
	{
	public:
		CCriticalSectionLock();
		~CCriticalSectionLock();

	public:
		void lock() noexcept;
		void unlock() noexcept;

	private:
		CRITICAL_SECTION m_criticalSection;
	};

	inline CCriticalSectionLock::CCriticalSectionLock()
	{
		InitializeCriticalSection(&m_criticalSection);
	}

	inline CCriticalSectionLock::~CCriticalSectionLock()
	{
		DeleteCriticalSection(&m_criticalSection);
	}

	inline void CCriticalSectionLock::lock() noexcept
	{
		EnterCriticalSection(&m_criticalSection);
	}

	inline void CCriticalSectionLock::unlock() noexcept
	{
		LeaveCriticalSection(&m_criticalSection);
	}
}