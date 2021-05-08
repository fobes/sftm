#include "CSftmPrivateHeapManager.h"

#define HEAP_PHYSICAL_SIZE (16*1024*1024)

CSftmPrivateHeapManager::CSftmPrivateHeapManager() noexcept
{

}

CSftmPrivateHeapManager::~CSftmPrivateHeapManager()
{
	Release();
}

bool CSftmPrivateHeapManager::Create() noexcept
{
	m_hHeap = HeapCreate(0, 0, HEAP_PHYSICAL_SIZE);

	InitializeSListHead(&m_FreeList);

	return m_hHeap != NULL;
}

void CSftmPrivateHeapManager::Release() noexcept
{
	if (m_hHeap != nullptr && HeapDestroy(m_hHeap))
		m_hHeap = nullptr;
}

void* CSftmPrivateHeapManager::Allocate(size_t nSize) noexcept
{
	PSLIST_ENTRY pFree = InterlockedPopEntrySList(&m_FreeList);
	while (pFree)
	{
		HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pFree);
		pFree = InterlockedPopEntrySList(&m_FreeList);
	}

	return HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, nSize);
}

void CSftmPrivateHeapManager::Free(void* pData) noexcept
{
	InterlockedPushEntrySList(&m_FreeList, (SLIST_ENTRY*)pData);
}
