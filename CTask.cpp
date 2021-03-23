#include "CTask.h"
#include "CWorker.h"

CTask::CTask(CTaskCounter& taskCounter) noexcept :m_taskCounter(taskCounter)
{

}

CTask::~CTask()
{

}

