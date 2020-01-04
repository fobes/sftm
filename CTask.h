#pragma once

#include "CTaskAllocator.h"
#include "CTaskCounter.h"

class CWorker;

class TM_API CTask : public CTaskAllocator
{
public:
	CTask(CTaskCounter &taskCounter);
	virtual ~CTask();

public:
	virtual bool Execute(CWorker &worker) = 0;

public:
	CTaskCounter &m_taskCounter;
};
