#pragma once

namespace sftm
{
	class CAsyncTask
	{
	public:
		CAsyncTask() noexcept {}
		virtual ~CAsyncTask() {}

	public:
		virtual void Execute() noexcept = 0;
	};
}
