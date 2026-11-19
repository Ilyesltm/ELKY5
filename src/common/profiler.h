#ifndef ELKY5_COMMON_PROFILER_H_
#define ELKY5_COMMON_PROFILER_H_

#include "common/common.h"

#include <cstdint>
#include <tracy/Tracy.hpp> // IWYU pragma: export

namespace profiler::colors {

inline constexpr uint32_t RedA100        = 0xff8a80;
inline constexpr uint32_t DeepOrangeA200 = 0xff6e40;

} // namespace profiler::colors

namespace Profiler {

void SetThreadName(const char* name);

void Initialize();
void Shutdown();

struct Lifecycle {
	static constexpr const char* name               = "Profiler";
	static constexpr auto        initialize         = Profiler::Initialize;
	static constexpr auto        shutdown           = Profiler::Shutdown;
	static constexpr auto        emergency_shutdown = Profiler::Shutdown;
};

} // namespace Profiler

#define ELKY5_PROFILER_CONCAT_IMPL(a, b) a##b
#define ELKY5_PROFILER_CONCAT(a, b)      ELKY5_PROFILER_CONCAT_IMPL(a, b)
#define ELKY5_PROFILER_COLOR_OR_DEFAULT(default_color, ...)                                         \
	ELKY5_PROFILER_COLOR_OR_DEFAULT_IMPL(default_color __VA_OPT__(, ) __VA_ARGS__, default_color)
#define ELKY5_PROFILER_COLOR_OR_DEFAULT_IMPL(default_color, color, ...) color

#define ELKY5_PROFILER_BLOCK(name, ...)                                                             \
	ELKY5_PROFILER_BLOCK_IMPL(__LINE__, name __VA_OPT__(, ) __VA_ARGS__)
#define ELKY5_PROFILER_BLOCK_IMPL(line, name, ...)                                                  \
	static constexpr tracy::SourceLocationData ELKY5_PROFILER_CONCAT(                               \
	    elky5_profiler_source_location_, line) {name, TracyFunction, TracyFile,                     \
	                                           static_cast<uint32_t>(line),                        \
	                                           ELKY5_PROFILER_COLOR_OR_DEFAULT(0, __VA_ARGS__)};    \
	tracy::ScopedZone ELKY5_PROFILER_CONCAT(elky5_profiler_block_, line)(                           \
	    &ELKY5_PROFILER_CONCAT(elky5_profiler_source_location_, line),                              \
	    TRACY_CALLSTACK, tracy::ProfilerAvailable())

#define ELKY5_PROFILER_FUNCTION(...) ELKY5_PROFILER_BLOCK(nullptr __VA_OPT__(, ) __VA_ARGS__)

#define ELKY5_PROFILER_THREAD(name) Profiler::SetThreadName(name)

#endif /* ELKY5_COMMON_PROFILER_H_ */
