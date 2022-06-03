#pragma once
#include <Windows.h>
#include "configs.h"

namespace sftm
{
	class CPrivateHeapManager
	{
	public:
		CPrivateHeapManager() noexcept;
		~CPrivateHeapManager();

	public:
		CPrivateHeapManager(const CPrivateHeapManager&)			= delete;
		void operator=(const CPrivateHeapManager&)				= delete;
		CPrivateHeapManager(CPrivateHeapManager&&)				= delete;
		CPrivateHeapManager& operator=(CPrivateHeapManager&&)	= delete;

	public:
		bool Create() noexcept;
		void Release() noexcept;

	public:
		void* Allocate(size_t nSize) noexcept;
		void Free(void* pData) noexcept;

	private:
		SLIST_HEADER m_FreeList{};
		HANDLE m_hHeap = { nullptr };
	};

	inline CPrivateHeapManager::CPrivateHeapManager() noexcept
	{

	}

	inline CPrivateHeapManager::~CPrivateHeapManager()
	{
		Release();
	}

	inline bool CPrivateHeapManager::Create() noexcept
	{
		m_hHeap = HeapCreate(0, 0, HEAP_PHYSICAL_SIZE);

		InitializeSListHead(&m_FreeList);

		return m_hHeap != nullptr;
	}

	inline void CPrivateHeapManager::Release() noexcept
	{
		if (m_hHeap != nullptr && HeapDestroy(m_hHeap))
			m_hHeap = nullptr;
	}

	inline void* CPrivateHeapManager::Allocate(size_t nSize) noexcept
	{
		PSLIST_ENTRY pFree = InterlockedPopEntrySList(&m_FreeList);
		while (pFree)
		{
			HeapFree(m_hHeap, HEAP_NO_SERIALIZE, pFree);
			pFree = InterlockedPopEntrySList(&m_FreeList);
		}

		return HeapAlloc(m_hHeap, HEAP_NO_SERIALIZE, nSize);
	}

	inline void CPrivateHeapManager::Free(void* pData) noexcept
	{
		InterlockedPushEntrySList(&m_FreeList, (SLIST_ENTRY*)pData);
	}
}