#pragma once
#include "CSftmTaskAllocator.h"
#include "CSftmChainController.h"

class CSftmWorker;

class TM_API CSftmTask : public CSftmTaskAllocator
{
public:
	CSftmTask(CSftmChainController* pChainController) noexcept;
	CSftmTask() = delete;
	CSftmTask(const CSftmTask&) = delete;
	void operator=(const CSftmTask&) = delete;
	CSftmTask(CSftmTask&&) = delete;
	CSftmTask& operator=(CSftmTask&&) = delete;
	virtual ~CSftmTask();

public:
	CSftmChainController* GetChainController() const noexcept;

public:
	virtual void Execute(CSftmWorker& worker) noexcept = 0;

	virtual unsigned GetUniqueIndex() const noexcept = 0;

protected:
	CSftmChainController* m_pChainController;
};
