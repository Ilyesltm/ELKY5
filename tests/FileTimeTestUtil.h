#ifndef ELKY5_TESTS_FILE_TIME_TEST_UTIL_H_
#define ELKY5_TESTS_FILE_TIME_TEST_UTIL_H_

#include <chrono>
#include <cstdint>
#include <filesystem>

// Sets a host file's write time to `unix_ns` nanoseconds since the Unix epoch.
inline void SetUnixWriteTime(const std::filesystem::path& path, int64_t unix_ns) {
	// Round to the file clock resolution first: the 1601 -> 1970 epoch shift overflows int64
	// nanoseconds inside the MSVC clock_cast.
	const auto time = std::chrono::floor<std::filesystem::file_time_type::duration>(
	    std::chrono::sys_time<std::chrono::nanoseconds> {std::chrono::nanoseconds(unix_ns)});
#ifdef _MSVC_STL_VERSION // no file_clock::from_sys
	const auto file_time = std::chrono::clock_cast<std::chrono::file_clock>(time);
#else
	const auto file_time = std::chrono::file_clock::from_sys(time);
#endif
	std::filesystem::last_write_time(path, file_time);
}

#endif // ELKY5_TESTS_FILE_TIME_TEST_UTIL_H_
