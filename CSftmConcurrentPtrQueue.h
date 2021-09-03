#pragma once
#include "CSftmSpinLock.h"
#include "CSftmCriticalSectionLock.h"
#include <mutex>
#include <functional>

#define QUEUE_PHYSICAL_SIZE 512
using CSyncPrimitive = CSftmCriticalSectionLock;

template<class T>
class CSftmConcurrentPtrQueue
{
public:
	using CAfterPushFunc = void (*)(T* item);

public:
	CSftmConcurrentPtrQueue() noexcept {}
	CSftmConcurrentPtrQueue(const CSftmConcurrentPtrQueue&) = delete;
	void operator=(const CSftmConcurrentPtrQueue&) = delete;
	CSftmConcurrentPtrQueue(CSftmConcurrentPtrQueue&&) = delete;
	CSftmConcurrentPtrQueue& operator=(CSftmConcurrentPtrQueue&&) = delete;
	~CSftmConcurrentPtrQueue() {}

public:
	bool Push(T* pItem, CAfterPushFunc pFunc) noexcept;
	T* Pop() noexcept;

	bool TrySteal(CSftmConcurrentPtrQueue& srcQueue) noexcept;

	bool IsEmpty() noexcept;

private:
	T* m_pItems[QUEUE_PHYSICAL_SIZE] = { nullptr };
	unsigned m_nCount = { 0 };

	CSyncPrimitive m_lock;
};

template<class T>
bool CSftmConcurrentPtrQueue<T>::IsEmpty() noexcept
{
	return m_nCount == 0;
}

template<class T>
bool CSftmConcurrentPtrQueue<T>::TrySteal(CSftmConcurrentPtrQueue& srcQueue) noexcept
{
	if (!srcQueue.m_nCount || m_nCount)
		return false;

	std::lock_guard<CSyncPrimitive> lock(srcQueue.m_lock);
	std::lock_guard<CSyncPrimitive> lockIdleThread(m_lock);

	if (!srcQueue.m_nCount || m_nCount)
		return false;

	const unsigned nGrabCount = 1;

	unsigned nTask;
	T** p = m_pItems;
	for (nTask = 0; nTask < nGrabCount; nTask++)
	{
		*p++ = srcQueue.m_pItems[nTask];
		srcQueue.m_pItems[nTask] = nullptr;
	}
	m_nCount = nGrabCount;

	p = srcQueue.m_pItems;
	for (; nTask < srcQueue.m_nCount; nTask++)
		*p++ = srcQueue.m_pItems[nTask];

	srcQueue.m_nCount -= nGrabCount;

	return true;
}

template<class T>
T* CSftmConcurrentPtrQueue<T>::Pop() noexcept
{
	std::lock_guard<CSyncPrimitive> lock(m_lock);

	if (!m_nCount)
		return nullptr;

	T* pTask = m_pItems[m_nCount - 1];
	m_nCount--;

	return pTask;
}

template<class T>
bool CSftmConcurrentPtrQueue<T>::Push(T* pItem, CAfterPushFunc pFunc) noexcept
{
	if (!pItem)
		return false;

	{
		std::lock_guard<CSyncPrimitive> lock(m_lock);

		if (m_nCount >= QUEUE_PHYSICAL_SIZE)
			return false;

		m_pItems[m_nCount++] = pItem;

		pFunc(pItem);
	}

	return true;
}
