#pragma once

#define STACK_MEMORY(TYPE, nCount)		\
	char data[sizeof(TYPE) * nCount];	\
	auto pAddress = (TYPE*)data;