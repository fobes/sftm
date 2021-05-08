#pragma once
#include "Export.h"

class TM_API CSftmRawMemoryManager
{
public:
	class TM_API CSftmRawMemory
	{
	public:
		CSftmRawMemory() noexcept;
		~CSftmRawMemory();

	public:
		bool Allocate(size_t nSize) noexcept;

	public:
		void* m_pData = { nullptr };
		size_t m_Size = { 0 };
	};

public:
	CSftmRawMemoryManager() noexcept;
	~CSftmRawMemoryManager();

public:
	bool Create() noexcept;
	void Release() noexcept;

public:
	void* Allocate(size_t nSize) noexcept;
	void Free(size_t nSize) noexcept;

private:
	char* m_pMemory = { nullptr };
	size_t m_nUsedCount = { 0 };
};
