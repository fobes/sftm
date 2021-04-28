#include "CTask.h"
#include "CWorker.h"

CTask::CTask(CChainController *pChainController) noexcept :m_pChainController(pChainController), m_eType(pChainController == nullptr ? EAsync : ESync)
{

}

CTask::~CTask()
{

}

CTask::ETaskType CTask::GetType() const noexcept
{
	return m_eType;
}

CChainController* CTask::GetChainController() const noexcept
{
	return m_pChainController;
}
