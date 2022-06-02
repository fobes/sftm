#pragma once
#include <mutex>
//#include "CCriticalSectionLock.hpp"
//#include "CSpinLock.hpp"

namespace sftm
{
	using CSyncPrimitive = std::mutex;
	//using CSftmSyncPrimitive = CSftmCriticalSectionLock;
	//using CSftmSyncPrimitive = CSftmSpinLock;
}