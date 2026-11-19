#ifndef EMULATOR_INCLUDE_EMULATOR_AUDIO_H_
#define EMULATOR_INCLUDE_EMULATOR_AUDIO_H_

#include "common/abi.h"
#include "common/common.h"
namespace Libs::Audio {

void Initialize();
void Shutdown();

struct Lifecycle {
	static constexpr const char* name       = "Audio";
	static constexpr auto        initialize = Libs::Audio::Initialize;
	static constexpr auto        shutdown   = Libs::Audio::Shutdown;
};

namespace AudioOut {

struct AudioOutOutputParam;
struct AudioOutPortState;
struct AudioOutSystemState;

int ELKY5_SYSV_ABI AudioOutInit();
int ELKY5_SYSV_ABI AudioOutOpen(int user_id, int type, int index, uint32_t len, uint32_t freq,
                               uint32_t param);
int ELKY5_SYSV_ABI AudioOutSetVolume(int handle, uint32_t flag, int* vol);
int ELKY5_SYSV_ABI AudioOutOutputs(AudioOutOutputParam* param, uint32_t num);
int ELKY5_SYSV_ABI AudioOutOutput(int handle, const void* ptr);
int ELKY5_SYSV_ABI AudioOutClose(int handle);
int ELKY5_SYSV_ABI AudioOutGetPortState(int handle, AudioOutPortState* state);
int ELKY5_SYSV_ABI AudioOutGetSystemState(AudioOutSystemState* state);

} // namespace AudioOut

namespace AudioOut2 {

struct AudioOut2ContextParam;
struct AudioOut2PortParam;
struct AudioOut2Attribute;
struct AudioOut2PortState;
struct AudioOut2SystemState;
struct AudioOut2Position;
struct AudioOut2SpeakerInfo;
struct AudioOut2SystemDebugStateParam;
struct AudioOut2MasteringParamsHeader;
struct AudioOut2MasteringStatesHeader;

using AudioOut2ContextHandle      = uint64_t;
using AudioOut2PortHandle         = uint64_t;
using AudioOut2UserHandle         = uintptr_t;
using AudioOut2SpeakerArrayHandle = void*;

int ELKY5_SYSV_ABI AudioOut2Initialize();
int ELKY5_SYSV_ABI AudioOut2ContextQueryMemory(const AudioOut2ContextParam* params,
                                              size_t*                      memory_size);
int ELKY5_SYSV_ABI AudioOut2ContextResetParam(AudioOut2ContextParam* params);
int ELKY5_SYSV_ABI AudioOut2ContextCreate(const AudioOut2ContextParam* params, void* buffer,
                                         size_t buffer_size, AudioOut2ContextHandle* ctx);
int ELKY5_SYSV_ABI AudioOut2ContextDestroy(AudioOut2ContextHandle ctx);
int ELKY5_SYSV_ABI AudioOut2ContextSetAttributes(AudioOut2ContextHandle    ctx,
                                                const AudioOut2Attribute* attributes, uint32_t num);
int ELKY5_SYSV_ABI AudioOut2ContextAdvance(AudioOut2ContextHandle ctx);
int ELKY5_SYSV_ABI AudioOut2ContextPush(AudioOut2ContextHandle ctx, uint32_t blocking);
int ELKY5_SYSV_ABI AudioOut2ContextGetQueueLevel(AudioOut2ContextHandle ctx, uint32_t* queue_level,
                                                uint32_t* available_queues);
int ELKY5_SYSV_ABI AudioOut2PortCreate(AudioOut2ContextHandle ctx, const AudioOut2PortParam* params,
                                      AudioOut2PortHandle* port);
int ELKY5_SYSV_ABI AudioOut2PortDestroy(AudioOut2PortHandle port);
int ELKY5_SYSV_ABI AudioOut2PortSetAttributes(AudioOut2PortHandle       port,
                                             const AudioOut2Attribute* attributes, uint32_t num);
int ELKY5_SYSV_ABI AudioOut2PortGetState(AudioOut2PortHandle port, AudioOut2PortState* state);
int ELKY5_SYSV_ABI AudioOut2GetSystemState(AudioOut2SystemState* state);
int ELKY5_SYSV_ABI AudioOut2UserCreate(uint32_t user_id, AudioOut2UserHandle* handle);
int ELKY5_SYSV_ABI AudioOut2UserDestroy(AudioOut2UserHandle handle);
int ELKY5_SYSV_ABI AudioOut2UserGetSupportedAttributes(AudioOut2UserHandle handle,
                                                       uint32_t* context_attributes,
                                                       uint32_t* port_attributes);
size_t ELKY5_SYSV_ABI AudioOut2GetSpeakerArrayMemorySize(uint32_t num_speakers, uint8_t is_3d,
                                                        uint8_t is_ambisonics);
int ELKY5_SYSV_ABI    AudioOut2SpeakerArrayCreate(AudioOut2SpeakerArrayHandle* handle,
                                                 const void* vbap_params, const void* ambi_params);
int ELKY5_SYSV_ABI    AudioOut2SpeakerArrayDestroy(AudioOut2SpeakerArrayHandle handle);
int ELKY5_SYSV_ABI    AudioOut2GetSpeakerArrayCoefficients(
    AudioOut2SpeakerArrayHandle handle, AudioOut2Position pos, float spread, float* coefficients,
    uint32_t num_coefficients, uint8_t height_aware, float downmix_spread_radius);
int ELKY5_SYSV_ABI AudioOut2GetSpeakerArrayAmbisonicsCoefficients(AudioOut2SpeakerArrayHandle handle,
                                                                 uint32_t ambisonics_channel,
                                                                 float*   coefficients,
                                                                 uint32_t num_coefficients);
int ELKY5_SYSV_ABI AudioOut2GetSpeakerInfo(AudioOut2SpeakerInfo* info, uint32_t flags);
int ELKY5_SYSV_ABI AudioOut2SetSystemDebugState(const AudioOut2SystemDebugStateParam* param);
int ELKY5_SYSV_ABI AudioOut2Set3DLatency(uint32_t user_id, uint32_t output, uint32_t latency_us);
int ELKY5_SYSV_ABI AudioOut2MasteringInit(uint32_t flags);
int ELKY5_SYSV_ABI AudioOut2MasteringSetParam(const AudioOut2MasteringParamsHeader* param,
                                             uint32_t output, uint32_t flags);
int ELKY5_SYSV_ABI AudioOut2MasteringGetState(AudioOut2MasteringStatesHeader* state, uint32_t output,
                                             AudioOut2UserHandle user);
int ELKY5_SYSV_ABI AudioOut2MasteringTerm();

} // namespace AudioOut2

namespace AudioIn {

int ELKY5_SYSV_ABI AudioInOpen(int user_id, int type, int index, uint32_t len,
                              uint32_t freq, uint32_t param);
int ELKY5_SYSV_ABI AudioInHqOpen(int user_id, int type, int index, uint32_t len,
                                uint32_t freq, uint32_t param);
int ELKY5_SYSV_ABI AudioInClose(int handle);
int ELKY5_SYSV_ABI AudioInInput(int handle, void* dest);
int ELKY5_SYSV_ABI AudioInGetSilentState(int handle);

} // namespace AudioIn

namespace VoiceQoS {

int ELKY5_SYSV_ABI VoiceQoSInit(void* mem_block, uint32_t mem_size, int32_t app_type);

} // namespace VoiceQoS

namespace Ajm {

struct AjmBatchInfo;
struct AjmBatchError;
struct AjmBuffer;

struct AjmDecAt9ConfigDataInfo {
	uint32_t channels;
	uint32_t sample_rate;
	uint32_t frame_samples_per_channel;
	uint32_t superframe_samples_per_channel;
	uint32_t superframe_size;
};

struct AjmDecMp3ParseFrameInfo {
	size_t   frame_size;
	uint32_t num_channels;
	uint32_t samples_per_channel;
	uint32_t bitrate;
	uint32_t sample_rate;
	uint32_t encoder_delay;
	uint32_t num_frames;
	uint32_t total_samples;
	uint32_t ofl_type;
};

int ELKY5_SYSV_ABI AjmInitialize(int64_t reserved, uint32_t* context);
int ELKY5_SYSV_ABI AjmModuleRegister(uint32_t context, uint32_t codec, int64_t reserved);
int ELKY5_SYSV_ABI AjmModuleUnregister(uint32_t context, uint32_t codec);
int ELKY5_SYSV_ABI AjmFinalize(uint32_t context);
int ELKY5_SYSV_ABI AjmInstanceCreate(uint32_t context, uint32_t codec, uint64_t flags,
                                    uint32_t* instance);
int ELKY5_SYSV_ABI AjmInstanceDestroy(uint32_t context, uint32_t instance);
int ELKY5_SYSV_ABI AjmMemoryRegister(uint32_t context, void* ptr, size_t pages);
int ELKY5_SYSV_ABI AjmMemoryUnregister(uint32_t context, void* ptr);
int ELKY5_SYSV_ABI AjmBatchInitialize(void* buffer, size_t size, AjmBatchInfo* info);
int ELKY5_SYSV_ABI AjmBatchStart(uint32_t context, const AjmBatchInfo* info, int priority,
                                AjmBatchError* error, uint32_t* batch);
int ELKY5_SYSV_ABI AjmBatchWait(uint32_t context, uint32_t batch, uint32_t timeout,
                               AjmBatchError* error);
int ELKY5_SYSV_ABI AjmBatchCancel(uint32_t context, uint32_t batch);
int ELKY5_SYSV_ABI AjmBatchErrorDump(const AjmBatchInfo* info, AjmBatchError* error);
int ELKY5_SYSV_ABI AjmBatchJobInitialize(AjmBatchInfo* info, uint32_t instance,
                                        const void* codec_parameters, size_t codec_parameters_size,
                                        void* result);
int ELKY5_SYSV_ABI AjmBatchJobClearContext(AjmBatchInfo* info, uint32_t instance, void* result);
int ELKY5_SYSV_ABI AjmBatchJobDecode(AjmBatchInfo* info, uint32_t instance,
                                    const void* bitstream_input, size_t bitstream_input_size,
                                    void* pcm_output, size_t pcm_output_size, void* result);
int ELKY5_SYSV_ABI AjmBatchJobDecodeSingle(AjmBatchInfo* info, uint32_t instance,
                                          const void* bitstream_input, size_t bitstream_input_size,
                                          void* pcm_output, size_t pcm_output_size, void* result);
int ELKY5_SYSV_ABI AjmBatchJobDecodeSplit(AjmBatchInfo* info, uint32_t instance,
                                         const AjmBuffer* input_buffers, size_t input_buffers_num,
                                         const AjmBuffer* output_buffers, size_t output_buffers_num,
                                         void* result);
int ELKY5_SYSV_ABI AjmBatchJobEncode(AjmBatchInfo* info, uint32_t instance, const void* pcm_input,
                                    size_t pcm_input_size, void* bitstream_output,
                                    size_t bitstream_output_size, void* result);
int ELKY5_SYSV_ABI AjmBatchJobGetInfo(AjmBatchInfo* info, uint32_t instance, void* result);
int ELKY5_SYSV_ABI AjmBatchJobGetCodecInfo(AjmBatchInfo* info, uint32_t instance, void* result,
                                          size_t result_size);
int ELKY5_SYSV_ABI AjmBatchJobSetGaplessDecode(AjmBatchInfo* info, uint32_t instance,
                                              const void* gapless_decode, int reset, void* result);
int ELKY5_SYSV_ABI AjmBatchJobGetGaplessDecode(AjmBatchInfo* info, uint32_t instance, void* result);
int ELKY5_SYSV_ABI AjmBatchJobSetResampleParameters(AjmBatchInfo* info, uint32_t instance,
                                                   float ratio, uint32_t flags, void* result);
int ELKY5_SYSV_ABI AjmBatchJobSetResampleParametersEx(AjmBatchInfo* info, uint32_t instance,
                                                     float ratio_start,
                                                     float ratio_change_per_sample, uint32_t flags,
                                                     void* result);
int ELKY5_SYSV_ABI AjmBatchJobGetResampleInfo(AjmBatchInfo* info, uint32_t instance, void* result);
int ELKY5_SYSV_ABI AjmBatchJobGetStatistics(AjmBatchInfo* info, float interval, void* result);
int ELKY5_SYSV_ABI AjmBatchJobControl(AjmBatchInfo* info, uint32_t instance, uint64_t flags,
                                     const void* sideband_input, size_t sideband_input_size,
                                     void* sideband_output, size_t sideband_output_size);
int ELKY5_SYSV_ABI AjmBatchJobRun(AjmBatchInfo* info, uint32_t instance, uint64_t flags,
                                 const void* data_input, size_t data_input_size, void* data_output,
                                 size_t data_output_size, void* sideband_output,
                                 size_t sideband_output_size);
int ELKY5_SYSV_ABI AjmBatchJobRunSplit(AjmBatchInfo* info, uint32_t instance, uint64_t flags,
                                      const AjmBuffer* input_buffers, size_t input_buffers_num,
                                      const AjmBuffer* output_buffers, size_t output_buffers_num,
                                      void* sideband_output, size_t sideband_output_size);
int ELKY5_SYSV_ABI AjmDecAt9ParseConfigData(const void*              config_data,
                                           AjmDecAt9ConfigDataInfo* config_info);
int ELKY5_SYSV_ABI AjmDecMp3ParseFrame(const void* bitstream, size_t size, int parse_ofl,
                                      AjmDecMp3ParseFrameInfo* frame_info);
const char* ELKY5_SYSV_ABI AjmStrError(int error);

} // namespace Ajm

namespace AvPlayer {

struct AvPlayerInitData;
struct AvPlayerFrameInfoEx;
struct AvPlayerFrameInfo;
struct AvPlayerInternal;

using Bool = uint8_t;

AvPlayerInternal* ELKY5_SYSV_ABI AvPlayerInit(AvPlayerInitData* init);
int ELKY5_SYSV_ABI               AvPlayerInitEx(const void* init_ex, AvPlayerInternal** handle);
int ELKY5_SYSV_ABI               AvPlayerPostInit(AvPlayerInternal* h, const void* post_init);
int ELKY5_SYSV_ABI               AvPlayerAddSource(AvPlayerInternal* h, const char* filename);
int ELKY5_SYSV_ABI               AvPlayerAddSourceEx(AvPlayerInternal* h, uint32_t uri_type,
                                                    const void* source_details);
int ELKY5_SYSV_ABI               AvPlayerStreamCount(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerGetStreamInfo(AvPlayerInternal* h, uint32_t stream_id, void* info);
int ELKY5_SYSV_ABI      AvPlayerGetStreamInfoEx(AvPlayerInternal* h, uint32_t stream_id, void* info);
int ELKY5_SYSV_ABI      AvPlayerEnableStream(AvPlayerInternal* h, uint32_t stream_id);
int ELKY5_SYSV_ABI      AvPlayerDisableStream(AvPlayerInternal* h, uint32_t stream_id);
int ELKY5_SYSV_ABI      AvPlayerChangeStream(AvPlayerInternal* h, uint32_t old_stream_id,
                                            uint32_t new_stream_id);
int ELKY5_SYSV_ABI      AvPlayerStart(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerStartEx(AvPlayerInternal* h, const void* start_info_ex);
int ELKY5_SYSV_ABI      AvPlayerStop(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerPause(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerResume(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerSetLooping(AvPlayerInternal* h, Bool loop);
int ELKY5_SYSV_ABI      AvPlayerSetTrickSpeed(AvPlayerInternal* h, int32_t trick_speed);
int ELKY5_SYSV_ABI      AvPlayerSetAvSyncMode(AvPlayerInternal* h, uint32_t sync_mode);
int ELKY5_SYSV_ABI      AvPlayerSetAvailableBandwidth(AvPlayerInternal* h, uint32_t start_bandwidth,
                                                     uint32_t minimum_bandwidth,
                                                     uint32_t maximum_bandwidth);
Bool ELKY5_SYSV_ABI     AvPlayerGetVideoData(AvPlayerInternal* h, AvPlayerFrameInfo* video_info);
Bool ELKY5_SYSV_ABI     AvPlayerGetVideoDataEx(AvPlayerInternal* h, AvPlayerFrameInfoEx* video_info);
Bool ELKY5_SYSV_ABI     AvPlayerGetAudioData(AvPlayerInternal* h, AvPlayerFrameInfo* audio_info);
Bool ELKY5_SYSV_ABI     AvPlayerIsActive(AvPlayerInternal* h);
uint64_t ELKY5_SYSV_ABI AvPlayerCurrentTime(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerJumpToTime(AvPlayerInternal* h, uint64_t time_ms);
int ELKY5_SYSV_ABI      AvPlayerClose(AvPlayerInternal* h);
int ELKY5_SYSV_ABI      AvPlayerSetLogCallback(void* callback, void* user_data);

} // namespace AvPlayer

} // namespace Libs::Audio

#endif /* EMULATOR_INCLUDE_EMULATOR_AUDIO_H_ */
