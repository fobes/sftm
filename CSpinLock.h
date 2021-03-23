#pragma once
#include <atomic>

class CSpinLock
{
public:
	void lock() noexcept;
	bool try_lock() noexcept;

	void unlock() noexcept;

private:
	std::atomic<bool> m_lock = { 0 };
};

