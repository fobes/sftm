#include "CTaskAllocator.h"
#include "CWorker.h"

CTaskAllocator::CTaskAllocator() noexcept :m_pData(CWorker::GetCurrentThreadWorker())
{

}

CTaskAllocator::~CTaskAllocator()
{

}

void* CTaskAllocator::operator new(size_t nSize) noexcept
{
	return CWorker::GetCurrentThreadWorker()->GetPrivateHeapManager().Allocate(nSize);
}

void CTaskAllocator::operator delete(void* p) noexcept
{
	((CWorker*)((CTaskAllocator*)p)->m_pData)->GetPrivateHeapManager().Free(p);
}
