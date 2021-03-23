#include "CTaskCounter.h"

CTaskCounter::CTaskCounter() noexcept :m_nCount(0)
{

}

CTaskCounter::~CTaskCounter()
{

}

bool CTaskCounter::IsEmpty() const noexcept
{
	return m_nCount == 0;
}

void CTaskCounter::Increase() noexcept
{
	m_nCount++;
}

void CTaskCounter::Reduce() noexcept
{
	m_nCount--;
}
