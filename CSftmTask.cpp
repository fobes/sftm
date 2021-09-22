#include "CSftmTask.h"
#include "CSftmWorker.h"

CSftmTask::CSftmTask(CSftmChainController *pChainController) noexcept :m_pChainController(pChainController) 
{

}

CSftmTask::~CSftmTask()
{

}

CSftmChainController* CSftmTask::GetChainController() const noexcept
{
	return m_pChainController;
}
