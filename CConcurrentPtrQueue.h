#pragma once
#include "CSpinLock.h"
#include <mutex>

#define QUEUE_PHYSICAL_SIZE 512
using CSyncPrimitive = CSpinLock;

template<class T> 
class CConcurrentPtrQueue
{
public:
	CConcurrentPtrQueue() noexcept {}
	CConcurrentPtrQueue(const CConcurrentPtrQueue&) = delete;
	void operator=(const CConcurrentPtrQueue&) = delete;
	CConcurrentPtrQueue(CConcurrentPtrQueue&&) = delete;
	CConcurrentPtrQueue& operator=(CConcurrentPtrQueue&&) = delete;
	~CConcurrentPtrQueue() {}

public:
	bool Push(T* pItem) noexcept;
	T* Pop() noexcept;

	bool TrySteal(CConcurrentPtrQueue& srcQueue) noexcept;

	bool IsEmpty() noexcept;

private:
	T* m_pItems[QUEUE_PHYSICAL_SIZE] = { nullptr };
	unsigned short m_nCount = { 0 };

	CSyncPrimitive m_lock;
};

template<class T>
bool CConcurrentPtrQueue<T>::IsEmpty() noexcept
{
	return m_nCount == 0;
}

template<class T>
bool CConcurrentPtrQueue<T>::TrySteal(CConcurrentPtrQueue& srcQueue) noexcept
{
	if (!srcQueue.m_nCount || m_nCount)
		return false;

	std::lock_guard<CSyncPrimitive> lock(srcQueue.m_lock);
	std::lock_guard<CSyncPrimitive> lockIdleThread(m_lock);

	if (!srcQueue.m_nCount || m_nCount)
		return false;

	unsigned nGrabCount = (srcQueue.m_nCount + 1) / 2;


	unsigned nTask;
	T** p = m_pItems;
	for (nTask = 0; nTask < nGrabCount; nTask++)
	{
		*p++ = srcQueue.m_pItems[nTask];
		srcQueue.m_pItems[nTask] = NULL;
	}
	m_nCount = nGrabCount;

	p = srcQueue.m_pItems;
	for (; nTask < srcQueue.m_nCount; nTask++)
		*p++ = srcQueue.m_pItems[nTask];

	srcQueue.m_nCount -= nGrabCount;

	return true;
}

template<class T>
T* CConcurrentPtrQueue<T>::Pop() noexcept
{
	std::lock_guard<CSyncPrimitive> lock(m_lock);

	if (!m_nCount)
		return NULL;

	T* pTask = m_pItems[m_nCount - 1];
	m_nCount--;

	return pTask;
}

template<class T>
bool CConcurrentPtrQueue<T>::Push(T* pItem) noexcept
{
	if (!pItem)
		return false;

	{
		std::lock_guard<CSyncPrimitive> lock(m_lock);

		if (m_nCount >= QUEUE_PHYSICAL_SIZE)
			return false;

		m_pItems[m_nCount++] = pItem;
	}

	return true;
}
