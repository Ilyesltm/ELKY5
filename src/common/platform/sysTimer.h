#ifndef ELKY5_COMMON_PLATFORM_SYSTIMER_H_
#define ELKY5_COMMON_PLATFORM_SYSTIMER_H_

#include "common/common.h"

#include <algorithm>
#include <cstdint>
#include <ctime>

#if ELKY5_PLATFORM == ELKY5_PLATFORM_WINDOWS
#include <windows.h>
#endif

struct SysFileTimeStruct {
#if ELKY5_PLATFORM == ELKY5_PLATFORM_WINDOWS
	FILETIME time;
#elif ELKY5_PLATFORM == ELKY5_PLATFORM_LINUX
	// Nanoseconds preserve sub-second file timestamps.
	time_t time;
	long   nanos;
#endif
	bool is_invalid;
};

#if ELKY5_PLATFORM == ELKY5_PLATFORM_WINDOWS

// FILETIME counts 100 ns units from 1601-01-01.
inline int64_t SysFileTimeToNs(const SysFileTimeStruct& f) {
	const auto units =
	    static_cast<int64_t>((uint64_t {f.time.dwHighDateTime} << 32u) | f.time.dwLowDateTime);
	// Clamp times outside 1677-2262 instead of overflowing.
	return std::clamp<int64_t>(units - 116444736000000000, INT64_MIN / 100, INT64_MAX / 100) * 100;
}

#elif ELKY5_PLATFORM == ELKY5_PLATFORM_LINUX

inline int64_t SysFileTimeToNs(const SysFileTimeStruct& f) {
	return int64_t {f.time} * 1000000000 + f.nanos;
}

#endif

#endif /* ELKY5_COMMON_PLATFORM_SYSTIMER_H_ */
