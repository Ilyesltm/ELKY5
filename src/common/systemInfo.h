#ifndef ELKY5_COMMON_SYSTEM_INFO_H_
#define ELKY5_COMMON_SYSTEM_INFO_H_

#include <string>

namespace Common {

struct SystemInfo {
	std::string ProcessorName;
};

[[nodiscard]] SystemInfo GetSystemInfo();

} // namespace Common

#endif /* ELKY5_COMMON_SYSTEM_INFO_H_ */
