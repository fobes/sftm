#include "CChainController.h"

CChainController::CChainController () noexcept
{

}

CChainController::~CChainController ()
{

}

bool CChainController::IsFinished() const noexcept
{
	return m_nCount == 0;
}

void CChainController::Increase() noexcept
{
	m_nCount++;
}

void CChainController::Reduce() noexcept
{
	m_nCount--;
}
