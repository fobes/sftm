#pragma once
#include "Export.h"
#include <windows.h>

class TM_API CSftmPrivateHeapManager
{
public:
	CSftmPrivateHeapManager() noexcept;
	CSftmPrivateHeapManager(const CSftmPrivateHeapManager&) = delete;
	void operator=(const CSftmPrivateHeapManager&) = delete;
	CSftmPrivateHeapManager(CSftmPrivateHeapManager&&) = delete;
	CSftmPrivateHeapManager& operator=(CSftmPrivateHeapManager&&) = delete;
	~CSftmPrivateHeapManager();

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
