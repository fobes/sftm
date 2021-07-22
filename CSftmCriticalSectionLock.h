#pragma once
#include "Export.h"
#include "windows.h"

class TM_API CSftmCriticalSectionLock
{
public:
	CSftmCriticalSectionLock();
	~CSftmCriticalSectionLock();

public:
	void lock() noexcept;
	void unlock() noexcept;

private:
	CRITICAL_SECTION m_criticalSection;
};
