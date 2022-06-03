#pragma once
#include "CChainController.hpp"

namespace sftm
{
	class CWorker;

	class CTask
	{
	public:
		CTask(CChainController& cc) noexcept;
		virtual ~CTask();

	public:
		CTask()							= delete;
		CTask(const CTask&)				= delete;
		void operator=(const CTask&)	= delete;
		CTask(CTask&&)					= delete;
		CTask& operator=(CTask&&)		= delete;

	public:
		virtual void Execute(CWorker& worker) noexcept = 0;

	public:
		CChainController& m_chainController;
	};

	inline CTask::CTask(CChainController& cc) noexcept
		:m_chainController(cc)
	{

	}

	inline CTask::~CTask() noexcept
	{

	}
}