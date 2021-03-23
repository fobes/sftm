#include "CRawMemoryManager.h"
#include "CWorker.h"

#define WORKER_RAW_MEMORY_SIZE (16*1024*1024)

CRawMemoryManager::CRawMemoryManager() noexcept
{

}

CRawMemoryManager::~CRawMemoryManager()
{

}

bool CRawMemoryManager::Create() noexcept
{
	m_pMemory = new char[WORKER_RAW_MEMORY_SIZE];
	m_nUsedCount = 0;

	return m_pMemory != NULL;
}

void* CRawMemoryManager::Allocate(size_t nSize) noexcept
{
	if (m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return NULL;

	char* ptr = m_pMemory + m_nUsedCount;
	m_nUsedCount += nSize;

	return ptr;
}

void CRawMemoryManager::Free(size_t nSize) noexcept
{
	m_nUsedCount -= nSize;
}

CRawMemoryManager::CRawMemory::CRawMemory() noexcept
{

}

CRawMemoryManager::CRawMemory::~CRawMemory()
{
	if (m_pData)
	{
		CWorker* pThread = CWorker::GetCurrentThreadWorker();

		pThread->m_rawMemoryManager.m_nUsedCount -= m_Size;
	}
}

bool CRawMemoryManager::CRawMemory::Allocate(size_t nSize) noexcept
{
	CWorker* pThread = CWorker::GetCurrentThreadWorker();
	if (m_pData || pThread->m_rawMemoryManager.m_nUsedCount + nSize > WORKER_RAW_MEMORY_SIZE)
		return false;

	m_Size = nSize;
	m_pData = pThread->m_rawMemoryManager.m_pMemory + pThread->m_rawMemoryManager.m_nUsedCount;
	pThread->m_rawMemoryManager.m_nUsedCount += nSize;

	return true;
}
