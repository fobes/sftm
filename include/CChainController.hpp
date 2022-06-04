#pragma once
#include <atomic>

namespace sftm
{
	class CChainController
	{
	public:
		CChainController() noexcept;
		~CChainController();

	public:
		CChainController(const CChainController&)		= delete;
		void operator=(const CChainController&)			= delete;
		CChainController(CChainController&&)			= delete;
		CChainController& operator=(CChainController&&) = delete;

	public:
		bool IsFinished() const noexcept;

		void Increase() noexcept;
		void Reduce() noexcept;

	private:
		std::atomic<std::uint32_t> m_nCount = { 0 };
	};

	inline CChainController::CChainController() noexcept
	{

	}

	inline CChainController::~CChainController()
	{

	}

	inline bool CChainController::IsFinished() const noexcept
	{
		return m_nCount == 0;
	}

	inline void CChainController::Increase() noexcept
	{
		++m_nCount;
	}

	inline void CChainController::Reduce() noexcept
	{
		--m_nCount;
	}
}