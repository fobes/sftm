#pragma once
#include "Export.h"

class TM_API CRawMemoryManager
{
public:
	class TM_API CRawMemory
	{
	public:
		CRawMemory() noexcept;
		~CRawMemory();

	public:
		bool Allocate(size_t nSize) noexcept;

	public:
		void* m_pData = { nullptr };
		size_t m_Size = { 0 };
	};

public:
	CRawMemoryManager() noexcept;
	~CRawMemoryManager();

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

