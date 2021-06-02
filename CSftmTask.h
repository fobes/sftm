#pragma once
#include "CSftmTaskAllocator.h"
#include "CSftmChainController.h"

class CSftmWorker;

class TM_API CSftmTask : public CSftmTaskAllocator
{
public:
	enum class ETaskType { ESync = 0, EAsync = 1 };

public:
	CSftmTask(CSftmChainController* pChainController) noexcept;
	CSftmTask() = delete;
	CSftmTask(const CSftmTask&) = delete;
	void operator=(const CSftmTask&) = delete;
	CSftmTask(CSftmTask&&) = delete;
	CSftmTask& operator=(CSftmTask&&) = delete;
	virtual ~CSftmTask();

public:
	ETaskType GetType() const noexcept;
	CSftmChainController* GetChainController() const noexcept;

public:
	virtual void Execute(CSftmWorker& worker) noexcept = 0;

protected:
	CSftmChainController* m_pChainController;
	ETaskType m_eType;
};
