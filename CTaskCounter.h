#pragma once

#include "Export.h"
#include "atomic"

class TM_API CTaskCounter
{
public:
	CTaskCounter();
	~CTaskCounter();

public:
	//Закончена ли цепочка
	bool IsEmpty() const;

	//Увеличить счетчик задач
	void Increase();
	//Уменьшить счетчик задач
	void Reduce();

private:
	std::atomic<size_t> m_nCount;
};
