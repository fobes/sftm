#pragma once

#include "Export.h"
#include "atomic"

class TM_API CTaskCounter
{
public:
	CTaskCounter();
	~CTaskCounter();

public:
	bool IsEmpty() const;

	void Increase();
	void Reduce();

private:
	std::atomic<size_t> m_nCount;
};
