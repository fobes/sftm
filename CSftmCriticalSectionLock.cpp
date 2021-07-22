#include "CSftmCriticalSectionLock.h"

CSftmCriticalSectionLock::CSftmCriticalSectionLock()
{
	InitializeCriticalSection(&m_criticalSection);
}

CSftmCriticalSectionLock::~CSftmCriticalSectionLock()
{
	DeleteCriticalSection(&m_criticalSection);
}

void CSftmCriticalSectionLock::lock() noexcept
{
	EnterCriticalSection(&m_criticalSection);
}

void CSftmCriticalSectionLock::unlock() noexcept
{
	LeaveCriticalSection(&m_criticalSection);
}
