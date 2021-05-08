#pragma once
#include "Export.h"
#include "atomic"

class TM_API CSftmChainController
{
public:
	CSftmChainController() noexcept;
	CSftmChainController(const CSftmChainController&) = delete;
	void operator=(const CSftmChainController&) = delete;
	CSftmChainController(CSftmChainController&&) = delete;
	CSftmChainController& operator=(CSftmChainController&&) = delete;
	~CSftmChainController();

public:
	bool IsFinished() const noexcept;

	void Increase() noexcept;
	void Reduce() noexcept;

private:
	std::atomic<size_t> m_nCount = { 0 };
};

