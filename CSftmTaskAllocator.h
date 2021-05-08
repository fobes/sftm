#pragma once
#include "Export.h"

class TM_API CSftmTaskAllocator
{
public:
	CSftmTaskAllocator() noexcept;
	CSftmTaskAllocator(const CSftmTaskAllocator&) = delete;
	void operator=(const CSftmTaskAllocator&) = delete;
	CSftmTaskAllocator(CSftmTaskAllocator&&) = delete;
	CSftmTaskAllocator& operator=(CSftmTaskAllocator&&) = delete;
	virtual ~CSftmTaskAllocator();

public:
	void* operator new(size_t nSize) noexcept;
	void operator delete(void* p) noexcept;

	void* operator new[](size_t) = delete;
	void operator delete[](void*) = delete;

private:
	void* m_pData;
};
