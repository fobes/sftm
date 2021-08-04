#include "CSftmRawMemoryManager.h"
#include "CSftmWorker.h"

#define WORKER_RAW_MEMORY_SIZE (16*1024*1024)

CSftmRawMemoryManager::CSftmRawMemoryManager() noexcept
{

}

CSftmRawMemoryManager::~CSftmRawMemoryManager()
{
	Release();
}

bool CSftmRawMemoryManager::Create() noexcept
{
	m_pMemory = new char[WORKER_RAW_MEMORY_SIZE];
	m_nUsedCount = 0;

	return m_pMemory != nullptr;
}

void CSftmRawMemoryManager::Release() noexcept
{
	delete[] m_pMemory;
	m_pMemory = nullptr;

	m_nUsedCount = 0;
}

void* CSftmRawMemoryManager::Allocate(size_t nSize) noexcept
{
	if (m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return nullptr;

	char* ptr = m_pMemory + m_nUsedCount;
	m_nUsedCount += nSize;

	return ptr;
}

void CSftmRawMemoryManager::Free(size_t nSize) noexcept
{
	m_nUsedCount -= nSize;
}

CSftmRawMemoryManager::CSftmRawMemory::CSftmRawMemory() noexcept
{

}

CSftmRawMemoryManager::CSftmRawMemory::~CSftmRawMemory()
{
	if (m_pData)
	{
		CSftmWorker* pThread = CSftmWorker::GetCurrentThreadWorker();

		pThread->m_rawMemoryManager.m_nUsedCount -= m_Size;
	}
}

bool CSftmRawMemoryManager::CSftmRawMemory::Allocate(size_t nSize) noexcept
{
	CSftmWorker* pThread = CSftmWorker::GetCurrentThreadWorker();
	if (m_pData || pThread->m_rawMemoryManager.m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return false;

	m_Size = nSize;
	m_pData = pThread->m_rawMemoryManager.m_pMemory + pThread->m_rawMemoryManager.m_nUsedCount;
	pThread->m_rawMemoryManager.m_nUsedCount += nSize;

	return true;
}
