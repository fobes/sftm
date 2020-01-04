#include "stdafx.h"
#include "CTaskAllocator.h"
#include "CWorker.h"

CTaskAllocator::CTaskAllocator():m_pData(CWorker::GetCurrentThreadWorker())
{

}

CTaskAllocator::~CTaskAllocator()
{

}

void* CTaskAllocator::operator new(size_t nSize)
{
	return CWorker::GetCurrentThreadWorker()->GetPrivateHeapManager().Alloc(nSize);
}

void CTaskAllocator::operator delete(void* p)
{
	((CWorker*)((CTaskAllocator*)p)->m_pData)->GetPrivateHeapManager().Free(p);
}

