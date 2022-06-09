#pragma once
#include <atomic>

namespace sftm
{
	class CChainController
	{
	public:
		CChainController() noexcept {}
		~CChainController() {}

	public:
		CChainController(const CChainController&)		= delete;
		void operator=(const CChainController&)			= delete;
		CChainController(CChainController&&)			= delete;
		CChainController& operator=(CChainController&&) = delete;

	public:
		bool IsFinished() const noexcept
		{
			return m_nCount == 0;
		}

		void Increase() noexcept
		{
			++m_nCount;
		}
		void Reduce() noexcept
		{
			--m_nCount;
		}

	private:
		std::atomic<std::uint32_t> m_nCount = { 0 };
	};
}