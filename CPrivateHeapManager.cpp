#include "CPrivateHeapManager.h"

#define HEAP_PHYSICAL_SIZE (16*1024*1024)

CPrivateHeapManager::CPrivateHeapManager() noexcept
{

}

CPrivateHeapManager::~CPrivateHeapManager()
{

}

bool CPrivateHeapManager::Create() noexcept
{
	m_hHeap = HeapCreate(0, 0, HEAP_PHYSICAL_SIZE);

	InitializeSListHead(&m_FreeList);

	return m_hHeap != NULL;
}

void* CPrivateHeapManager::Allocate(size_t nSize) noexcept
{
	PSLIST_ENTRY pFree = InterlockedPopEntrySList(&m_FreeList);
	while (pFree)
	{
		HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pFree);
		pFree = InterlockedPopEntrySList(&m_FreeList);
	}

	return HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, nSize);
}

void CPrivateHeapManager::Free(void* pData) noexcept
{
	InterlockedPushEntrySList(&m_FreeList, (SLIST_ENTRY*)pData);
}
