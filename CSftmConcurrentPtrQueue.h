#pragma once
#include "CSftmSyncPrimitive.h"

#define QUEUE_PHYSICAL_SIZE 512

template<class T>
class CSftmConcurrentPtrQueue
{
public:
	CSftmConcurrentPtrQueue() noexcept {}
	CSftmConcurrentPtrQueue(const CSftmConcurrentPtrQueue&) = delete;
	void operator=(const CSftmConcurrentPtrQueue&) = delete;
	CSftmConcurrentPtrQueue(CSftmConcurrentPtrQueue&&) = delete;
	CSftmConcurrentPtrQueue& operator=(CSftmConcurrentPtrQueue&&) = delete;
	~CSftmConcurrentPtrQueue() {}

public:
	bool Push(T* pItem) noexcept;
	T* Pop() noexcept;

	bool TrySteal(CSftmConcurrentPtrQueue& srcQueue) noexcept;

private:
	T* m_pItems[QUEUE_PHYSICAL_SIZE] = { nullptr };
	unsigned m_nCount = { 0 };

	CSftmSyncPrimitive m_lock;
};

template<class T>
bool CSftmConcurrentPtrQueue<T>::TrySteal(CSftmConcurrentPtrQueue& srcQueue) noexcept
{
	if (!srcQueue.m_nCount)
		return false;

	T* pItem = nullptr;

	{
		std::lock_guard<CSftmSyncPrimitive> srcLock(srcQueue.m_lock);
		if (!srcQueue.m_nCount)
			return false;

		pItem = srcQueue.m_pItems[srcQueue.m_nCount - 1];
		srcQueue.m_pItems[srcQueue.m_nCount - 1] = nullptr;
		srcQueue.m_nCount--;
	}

	std::lock_guard<CSftmSyncPrimitive> currentLock(m_lock);

	m_pItems[m_nCount++] = pItem;

	return true;
}

template<class T>
T* CSftmConcurrentPtrQueue<T>::Pop() noexcept
{
	if (!m_nCount)
		return nullptr;

	std::lock_guard<CSftmSyncPrimitive> lock(m_lock);

	if (!m_nCount)
		return nullptr;

	T* pTask = m_pItems[m_nCount - 1];
	m_nCount--;

	return pTask;
}

template<class T>
bool CSftmConcurrentPtrQueue<T>::Push(T* pItem) noexcept
{
	std::lock_guard<CSftmSyncPrimitive> lock(m_lock);

	if (m_nCount >= QUEUE_PHYSICAL_SIZE)
		return false;

	m_pItems[m_nCount++] = pItem;

	return true;
}
