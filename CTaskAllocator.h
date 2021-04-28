#pragma once
#include "Export.h"

class TM_API CTaskAllocator
{
public:
	CTaskAllocator() noexcept;
	CTaskAllocator(const CTaskAllocator&) = delete;
	void operator=(const CTaskAllocator&) = delete;
	CTaskAllocator(CTaskAllocator&&) = delete;
	CTaskAllocator& operator=(CTaskAllocator&&) = delete;
	virtual ~CTaskAllocator();

public:
	void* operator new(size_t nSize) noexcept;
	void operator delete(void* p) noexcept;

	void* operator new[](size_t) = delete;
	void operator delete[](void*) = delete;

private:
	void* m_pData;
};
