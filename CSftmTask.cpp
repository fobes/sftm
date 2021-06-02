#include "CSftmTask.h"
#include "CSftmWorker.h"

CSftmTask::CSftmTask(CSftmChainController *pChainController) noexcept :m_pChainController(pChainController) 
{
	if(m_pChainController)
		m_eType = ETaskType::ESync;
	else
		m_eType = ETaskType::EAsync;
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
