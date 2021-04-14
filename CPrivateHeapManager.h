#pragma once
#include "Export.h"
#include <windows.h>

class TM_API CPrivateHeapManager
{
public:
	CPrivateHeapManager() noexcept;
	CPrivateHeapManager(const CPrivateHeapManager&) = delete;
	void operator=(const CPrivateHeapManager&) = delete;
	CPrivateHeapManager(CPrivateHeapManager&&) = delete;
	CPrivateHeapManager& operator=(CPrivateHeapManager&&) = delete;
	~CPrivateHeapManager();

public:
	bool Create() noexcept;
	void Release() noexcept;

public:
	void* Allocate(size_t nSize) noexcept;
	void Free(void* pData) noexcept;

private:
	SLIST_HEADER m_FreeList;
	HANDLE m_hHeap = { nullptr };
};


