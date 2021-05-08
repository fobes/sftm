#include "CSftmTask.h"
#include "CSftmWorker.h"

CSftmTask::CSftmTask(CSftmChainController *pChainController) noexcept :m_pChainController(pChainController), m_eType(pChainController == nullptr ? EAsync : ESync)
{

}

CSftmTask::~CSftmTask()
{

}

CSftmTask::ETaskType CSftmTask::GetType() const noexcept
{
	return m_eType;
}

CSftmChainController* CSftmTask::GetChainController() const noexcept
{
	return m_pChainController;
}
