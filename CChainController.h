#pragma once
#include "Export.h"
#include "atomic"

class TM_API CChainController
{
public:
	CChainController() noexcept;
	CChainController(const CChainController&) = delete;
	void operator=(const CChainController&) = delete;
	CChainController(CChainController&&) = delete;
	CChainController& operator=(CChainController&&) = delete;
	~CChainController();

public:
	bool IsFinished() const noexcept;

	void Increase() noexcept;
	void Reduce() noexcept;

private:
	std::atomic<size_t> m_nCount = { 0 };
};

