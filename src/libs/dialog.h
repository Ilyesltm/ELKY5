#ifndef EMULATOR_INCLUDE_EMULATOR_DIALOG_H_
#define EMULATOR_INCLUDE_EMULATOR_DIALOG_H_

#include "common/abi.h"
#include "common/common.h"

namespace Libs::Dialog {

namespace CommonDialog {

int ELKY5_SYSV_ABI  CommonDialogInitialize();
bool ELKY5_SYSV_ABI CommonDialogIsUsed();

} // namespace CommonDialog

namespace LoginDialog {

int ELKY5_SYSV_ABI  LoginDialogInitialize();
int ELKY5_SYSV_ABI  LoginDialogTerminate();
int ELKY5_SYSV_ABI  LoginDialogOpen(const void* param);
int ELKY5_SYSV_ABI  LoginDialogClose();
int ELKY5_SYSV_ABI  LoginDialogUpdateStatus();
int ELKY5_SYSV_ABI  LoginDialogGetStatus();
int ELKY5_SYSV_ABI  LoginDialogGetResult(void* result);
void ELKY5_SYSV_ABI LoginDialogParamInitialize(void* param);

} // namespace LoginDialog

namespace SigninDialog {

int ELKY5_SYSV_ABI SigninDialogInitialize();
int ELKY5_SYSV_ABI SigninDialogTerminate();
int ELKY5_SYSV_ABI SigninDialogOpen(const void* param);
int ELKY5_SYSV_ABI SigninDialogClose();
int ELKY5_SYSV_ABI SigninDialogUpdateStatus();
int ELKY5_SYSV_ABI SigninDialogGetStatus();
int ELKY5_SYSV_ABI SigninDialogGetResult(void* result);

} // namespace SigninDialog

namespace SaveDataDialog {

int ELKY5_SYSV_ABI SaveDataDialogInitialize();
int ELKY5_SYSV_ABI SaveDataDialogGetStatus();
int ELKY5_SYSV_ABI SaveDataDialogUpdateStatus();
int ELKY5_SYSV_ABI SaveDataDialogGetResult(void* result);
int ELKY5_SYSV_ABI SaveDataDialogOpen(const void* param);
int ELKY5_SYSV_ABI SaveDataDialogClose(const void* close_param);
int ELKY5_SYSV_ABI SaveDataDialogIsReadyToDisplay();
int ELKY5_SYSV_ABI SaveDataDialogTerminate();
int ELKY5_SYSV_ABI SaveDataDialogProgressBarInc(int target, uint32_t delta);
int ELKY5_SYSV_ABI SaveDataDialogProgressBarSetValue(int target, uint32_t rate);

} // namespace SaveDataDialog

namespace MsgDialog {

int ELKY5_SYSV_ABI MsgDialogInitialize();
int ELKY5_SYSV_ABI MsgDialogOpen(const void* param);
int ELKY5_SYSV_ABI MsgDialogUpdateStatus();
int ELKY5_SYSV_ABI MsgDialogGetStatus();
int ELKY5_SYSV_ABI MsgDialogGetResult(void* result);
int ELKY5_SYSV_ABI MsgDialogTerminate();
int ELKY5_SYSV_ABI MsgDialogClose();
int ELKY5_SYSV_ABI MsgDialogProgressBarInc(int target, uint32_t delta);
int ELKY5_SYSV_ABI MsgDialogProgressBarSetValue(int target, uint32_t rate);
int ELKY5_SYSV_ABI MsgDialogProgressBarSetMsg(int target, const char* msg);

} // namespace MsgDialog

namespace SystemDialog {

enum class Kind { Error, Signin };

struct HostSnapshot {
	Kind     kind;
	uint64_t generation;
	int32_t  error_code;
};

struct VisualState {
	bool     active;
	bool     background;
	uint64_t revision;
};

bool        GetHostSnapshot(HostSnapshot* snapshot);
VisualState GetVisualState() noexcept;
void        SetVisibilityCallback(void (*callback)());
bool        HostClose(uint64_t generation);

} // namespace SystemDialog

namespace ErrorDialog {

int ELKY5_SYSV_ABI ErrorDialogInitialize();
int ELKY5_SYSV_ABI ErrorDialogOpen(const void* param);
int ELKY5_SYSV_ABI ErrorDialogClose();
int ELKY5_SYSV_ABI ErrorDialogTerminate();
int ELKY5_SYSV_ABI ErrorDialogUpdateStatus();
int ELKY5_SYSV_ABI ErrorDialogGetStatus();

} // namespace ErrorDialog

} // namespace Libs::Dialog

#endif /* EMULATOR_INCLUDE_EMULATOR_DIALOG_H_ */
