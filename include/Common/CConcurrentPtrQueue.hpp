#pragma once
#include "configs.h"
#include "CSyncPrimitive.hpp"

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
		bool Push(T* pItem) noexcept
		{
			std::lock_guard<CSyncPrimitive> lock(m_lock);

			if (m_nCount >= QUEUE_PHYSICAL_SIZE)
				return false;

			m_pItems[m_nCount++] = pItem;

			return true;
		}
		T* Pop() noexcept
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

		bool TrySteal(CConcurrentPtrQueue& srcQueue) noexcept
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
				--srcQueue.m_nCount;
			}

			std::lock_guard<CSyncPrimitive> currentLock(m_lock);

			m_pItems[m_nCount++] = pItem;

			return true;
		}

		bool Empty() const noexcept
		{
			std::lock_guard<CSyncPrimitive> lock(m_lock);

			return m_nCount == 0;
		}

	private:
		T*	m_pItems[QUEUE_PHYSICAL_SIZE]	= { nullptr };
		std::uint32_t	m_nCount			= 0;

		mutable CSyncPrimitive m_lock;
	};
}