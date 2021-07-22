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

void CSftmSpinLock::unlock() noexcept
{
	m_lock.store(false, std::memory_order_release);
}
