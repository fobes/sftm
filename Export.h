#pragma once

#ifdef EXPORT_API
#define TM_API __declspec(dllexport)
#else
#define TM_API __declspec(dllimport)
#endif
