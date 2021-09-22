#include "CSftmTaskAllocator.h"
#include "CSftmWorker.h"

CSftmTaskAllocator::CSftmTaskAllocator() noexcept :m_pData(CSftmWorker::GetCurrentThreadWorker())
{

}

CSftmTaskAllocator::~CSftmTaskAllocator()
{

}

void* CSftmTaskAllocator::operator new(size_t nSize) noexcept
{
	return CSftmWorker::GetCurrentThreadWorker()->m_privateHeapManager.Allocate(nSize);
}

void CSftmTaskAllocator::operator delete(void* p) noexcept
{
	((CSftmWorker*)((CSftmTaskAllocator*)p)->m_pData)->m_privateHeapManager.Free(p);
}
