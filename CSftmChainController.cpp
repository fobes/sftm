#include "CSftmChainController.h"

CSftmChainController::CSftmChainController () noexcept
{

}

CSftmChainController::~CSftmChainController ()
{

}

bool CSftmChainController::IsFinished() const noexcept
{
	return m_nCount == 0;
}

void CSftmChainController::Increase() noexcept
{
	m_nCount++;
}

void CSftmChainController::Reduce() noexcept
{
	m_nCount--;
}
