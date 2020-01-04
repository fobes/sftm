#include "stdafx.h"
#include "CTaskCounter.h"

CTaskCounter::CTaskCounter() :m_nCount(0)
{

}

CTaskCounter::~CTaskCounter()
{

}

bool CTaskCounter::IsEmpty() const
{
	return m_nCount == 0;
}

void CTaskCounter::Increase()
{
	m_nCount++;
}

void CTaskCounter::Reduce()
{
	m_nCount--;
}
