#pragma once

#include "CTaskAllocator.h"
#include "CTaskCounter.h"

class CWorker;

class TM_API CTask : public CTaskAllocator
{
public:
	CTask(CTaskCounter &taskCounter) noexcept;
	CTask() = delete;
	CTask(const CTask&) = delete;
	void operator=(const CTask&) = delete;
	CTask(CTask&&) = delete;
	CTask& operator=(CTask&&) = delete;
	virtual ~CTask();

public:
	virtual bool Execute(CWorker &worker) noexcept = 0;

public:
	CTaskCounter &m_taskCounter;
};
