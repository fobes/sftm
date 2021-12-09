#pragma once
#include <mutex>
#include "CSftmCriticalSectionLock.h"
#include "CSftmSpinLock.h"

using CSftmSyncPrimitive = std::mutex;
//using CSftmSyncPrimitive = CSftmCriticalSectionLock;
//using CSftmSyncPrimitive = CSftmSpinLock;