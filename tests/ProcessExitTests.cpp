#include "common/abi.h"
#include "common/emulatorConfig.h"
#include "common/logging/log.h"
#include "common/subsystems.h"
#include "common/threads.h"
#include "loader/symbolDatabase.h"

#include <atomic>
#include <cstdlib>
#include <gtest/gtest.h>
#include <thread>

namespace Libs {
void InitLibKernel_1(Loader::SymbolDatabase*);
namespace LibC {
void InitLibC_1(Loader::SymbolDatabase*);
} // namespace LibC
} // namespace Libs

namespace {

using GuestExit = void(ELKY5_SYSV_ABI*)(int);

constexpr int EXIT_CODE                = 7;
constexpr int STATIC_DESTRUCTOR_STATUS = 99;

GuestExit Resolve(void (*init)(Loader::SymbolDatabase*), const char* nid) {
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
	init(&symbols);
	const auto* record = symbols.FindByNid(nid, Loader::SymbolType::Func);
	return record != nullptr ? reinterpret_cast<GuestExit>(record->vaddr) : nullptr;
}

// Stands for the host state (guest memory, GPU and window objects) that live threads still use
// when the guest calls exit(): running its destructor means static destruction ran.
struct StaticDestructorProbe {
	~StaticDestructorProbe() { std::_Exit(STATIC_DESTRUCTOR_STATUS); }
};

[[noreturn]] void ExitWithLiveThread(GuestExit guest_exit) {
	static StaticDestructorProbe probe;
	static std::atomic<bool>     running = false;
	std::thread([] {
		running.store(true);
		for (;;) {
			std::this_thread::yield();
		}
	}).detach();
	while (!running.load()) {
		std::this_thread::yield();
	}
	guest_exit(EXIT_CODE);
	std::_Exit(1);
}

TEST(ProcessExit, LibcExitEndsTheProcessWithoutStaticDestruction) {
	const auto guest_exit = Resolve(Libs::LibC::InitLibC_1, "uMei1W9uyNo");
	ASSERT_NE(guest_exit, nullptr);
	EXPECT_EXIT(ExitWithLiveThread(guest_exit), testing::ExitedWithCode(EXIT_CODE), "");
}

TEST(ProcessExit, KernelExitEndsTheProcessWithoutStaticDestruction) {
	const auto guest_exit = Resolve(Libs::InitLibKernel_1, "6Z83sYWFlA8");
	ASSERT_NE(guest_exit, nullptr);
	EXPECT_EXIT(ExitWithLiveThread(guest_exit), testing::ExitedWithCode(EXIT_CODE), "");
}

} // namespace

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
