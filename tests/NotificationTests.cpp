#include "common/abi.h"
#include "common/emulatorConfig.h"
#include "common/logging/log.h"
#include "common/subsystems.h"
#include "common/threads.h"
#include "libs/errno.h"
#include "loader/symbolDatabase.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

namespace Libs {
void InitLibKernel_1(Loader::SymbolDatabase*);
} // namespace Libs

namespace {

using SendNotificationRequest = int(ELKY5_SYSV_ABI*)(int, const char*, uint64_t, int);

// The request's text starts at byte 45 of a 3120-byte block.
constexpr size_t REQUEST_SIZE   = 3120;
constexpr size_t MESSAGE_OFFSET = 45;

SendNotificationRequest Resolve() {
	static Common::Subsystems subsystems;
	static bool               initialized = false;
	if (!initialized) {
		Common::InitializeThreads();
		subsystems.Initialize<Config::Lifecycle>();
		Config::Load({.printf_direction = Config::LogDirection::Silent});
		subsystems.Initialize<Log::Lifecycle>();
		initialized = true;
	}
	static Loader::SymbolDatabase symbols;
	static const auto*            record = [] {
		Libs::InitLibKernel_1(&symbols);
		return symbols.FindByNid("zl7hupSO0C0", Loader::SymbolType::Func);
	}();
	return record != nullptr ? reinterpret_cast<SendNotificationRequest>(record->vaddr) : nullptr;
}

std::array<char, REQUEST_SIZE> Request(const std::string& text) {
	std::array<char, REQUEST_SIZE> request {};
	std::memcpy(request.data() + MESSAGE_OFFSET, text.data(), text.size());
	return request;
}

TEST(Notification, PrintsTheRequestText) {
	const auto send = Resolve();
	ASSERT_NE(send, nullptr);

	const auto request = Request("hello from a title");
	testing::internal::CaptureStdout();
	EXPECT_EQ(send(0, request.data(), request.size(), 0), OK);
	EXPECT_EQ(testing::internal::GetCapturedStdout(), "[Notification] hello from a title\n");
}

TEST(Notification, StopsAtTheEndOfTheRequest) {
	const auto send = Resolve();
	ASSERT_NE(send, nullptr);

	// The text is not NUL-terminated inside the request size.
	auto request = Request("abcdef");
	testing::internal::CaptureStdout();
	EXPECT_EQ(send(0, request.data(), MESSAGE_OFFSET + 3, 0), OK);
	EXPECT_EQ(testing::internal::GetCapturedStdout(), "[Notification] abc\n");
}

TEST(Notification, RejectsMissingOrShortRequests) {
	const auto send = Resolve();
	ASSERT_NE(send, nullptr);

	const auto request = Request("ignored");
	testing::internal::CaptureStdout();
	EXPECT_EQ(send(0, nullptr, REQUEST_SIZE, 0), Libs::LibKernel::KERNEL_ERROR_EINVAL);
	EXPECT_EQ(send(0, request.data(), MESSAGE_OFFSET, 0), Libs::LibKernel::KERNEL_ERROR_EINVAL);
	EXPECT_TRUE(testing::internal::GetCapturedStdout().empty());
}

} // namespace

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
