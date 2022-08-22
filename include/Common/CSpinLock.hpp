#pragma once
#include <atomic>

namespace sftm
{
	class CSpinLock 
	{
    public:
        void lock() noexcept
	{
		for (;;) 
		{
			if (!m_bLock.exchange(true, std::memory_order_acquire))
				break;

			while (m_bLock.load(std::memory_order_relaxed));
		}
        }

        void unlock() noexcept
	{
		m_bLock.store(false, std::memory_order_release);
        }

    private:
	std::atomic<bool> m_bLock = { false };
    };
}
