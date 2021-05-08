#include "CSftmSpinLock.h"

void CSftmSpinLock::lock() noexcept
{
	for (;;)
	{
		if (!m_lock.exchange(true, std::memory_order_acquire))
			return;

		while (m_lock.load(std::memory_order_relaxed))
		{

		}
	}
}

bool CSftmSpinLock::try_lock() noexcept
{
	return !m_lock.load(std::memory_order_relaxed) && !m_lock.exchange(true, std::memory_order_acquire);
}

void CSftmSpinLock::unlock() noexcept
{
	m_lock.store(false, std::memory_order_release);
}
