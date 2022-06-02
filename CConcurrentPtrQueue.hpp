#pragma once
#include "CSyncPrimitive.hpp"
#include "configs.h"

namespace sftm
{
	template<class T>
	class CConcurrentPtrQueue
	{
	public:
		CConcurrentPtrQueue() noexcept {}
		~CConcurrentPtrQueue() {}

	public:
		CConcurrentPtrQueue(const CConcurrentPtrQueue&)			= delete;
		void operator=(const CConcurrentPtrQueue&)				= delete;
		CConcurrentPtrQueue(CConcurrentPtrQueue&&)				= delete;
		CConcurrentPtrQueue& operator=(CConcurrentPtrQueue&&)	= delete;

	public:
		bool Push(T* pItem) noexcept;
		T* Pop() noexcept;

		bool TrySteal(CConcurrentPtrQueue& srcQueue) noexcept;

	private:
		T*	m_pItems[QUEUE_PHYSICAL_SIZE]	= { nullptr };
		int	m_nCount						= 0;

		CSyncPrimitive m_lock;
	};

	template<class T>
	bool CConcurrentPtrQueue<T>::TrySteal(CConcurrentPtrQueue& srcQueue) noexcept
	{
		if (!srcQueue.m_nCount)
			return false;

		T* pItem = nullptr;

		{
			std::lock_guard<CSyncPrimitive> srcLock(srcQueue.m_lock);
			if (!srcQueue.m_nCount)
				return false;

			pItem = srcQueue.m_pItems[srcQueue.m_nCount - 1];
			srcQueue.m_pItems[srcQueue.m_nCount - 1] = nullptr;
			srcQueue.m_nCount--;
		}

		std::lock_guard<CSyncPrimitive> currentLock(m_lock);

		m_pItems[m_nCount++] = pItem;

		return true;
	}

	template<class T>
	T* CConcurrentPtrQueue<T>::Pop() noexcept
	{
		if (!m_nCount)
			return nullptr;

		std::lock_guard<CSyncPrimitive> lock(m_lock);

		if (!m_nCount)
			return nullptr;

		T* pTask = m_pItems[m_nCount - 1];
		m_nCount--;

		return pTask;
	}

	template<class T>
	bool CConcurrentPtrQueue<T>::Push(T* pItem) noexcept
	{
		std::lock_guard<CSyncPrimitive> lock(m_lock);

		if (m_nCount >= QUEUE_PHYSICAL_SIZE)
			return false;

		m_pItems[m_nCount++] = pItem;

		return true;
	}
}