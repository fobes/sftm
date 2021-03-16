#pragma once

#include "Export.h"
#include "atomic"

class TM_API CTaskCounter
{
public:
	CTaskCounter() noexcept;
	CTaskCounter(const CTaskCounter&) = delete;
	void operator=(const CTaskCounter&) = delete;
	CTaskCounter(CTaskCounter&&) = delete;
	CTaskCounter& operator=(CTaskCounter&&) = delete;
	~CTaskCounter();

public:
	bool IsEmpty() const noexcept;

	void Increase() noexcept;
	void Reduce() noexcept;

private:
	std::atomic<size_t> m_nCount;
};
