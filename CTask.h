#pragma once
#include "CTaskAllocator.h"
#include "CChainController.h"

class CWorker;

class TM_API CTask : public CTaskAllocator
{
public:
	enum ETaskType { ESync = 0, EAsync = 1 };

public:
	CTask(CChainController* pChainController) noexcept;
	CTask() = delete;
	CTask(const CTask&) = delete;
	void operator=(const CTask&) = delete;
	CTask(CTask&&) = delete;
	CTask& operator=(CTask&&) = delete;
	virtual ~CTask();

public:
	ETaskType GetType() const noexcept;
	CChainController* GetChainController() const noexcept;

public:
	virtual bool Execute(CWorker& worker) noexcept = 0;

protected:
	CChainController* m_pChainController;
	ETaskType m_eType;
};
