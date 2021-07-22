#pragma once
#include "Export.h"
#include <atomic>

class TM_API CSftmSpinLock
{
public:
	void lock() noexcept;
	void unlock() noexcept;

private:
	std::atomic<bool> m_lock = { 0 };
};
