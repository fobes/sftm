#pragma once
#include <atomic>

namespace sftm
{
	class CSftmSpinLock
	{
	public:
		void lock() noexcept;
		void unlock() noexcept;

	private:
		std::atomic<bool> m_lock = { 0 };
	};

	inline void CSftmSpinLock::lock() noexcept
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

	inline void CSftmSpinLock::unlock() noexcept
	{
		m_lock.store(false, std::memory_order_release);
	}
}