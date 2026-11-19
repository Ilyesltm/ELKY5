cmake_minimum_required(VERSION 3.19...4.3)

if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
	set(CMAKE_POLICY_VERSION_MINIMUM 3.10 CACHE STRING "Minimum policy version for bundled dependencies")
endif()

get_property(ELKY5_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT ELKY5_GENERATOR_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build" FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

project(Elky5)

find_program(SCCACHE_PROGRAM sccache)
find_program(CCACHE_PROGRAM ccache)

foreach(lang IN ITEMS C CXX)
    # An explicitly empty launcher disables caching for this language.
    if(NOT DEFINED CMAKE_${lang}_COMPILER_LAUNCHER)
        if(SCCACHE_PROGRAM)
            set(CMAKE_${lang}_COMPILER_LAUNCHER "${SCCACHE_PROGRAM}")
            message(STATUS "sccache enabled for ${lang}: ${SCCACHE_PROGRAM}")
        elseif(CCACHE_PROGRAM)
            set(CMAKE_${lang}_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
            message(STATUS "ccache enabled for ${lang}: ${CCACHE_PROGRAM}")
        endif()
    endif()
endforeach()

set(ELKY5_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ELKY5_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(ELKY5_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty")

if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
	set(LINUX TRUE)
endif()

if (NOT (WIN32 OR LINUX OR APPLE))
    message(FATAL_ERROR "only Windows, Linux, and macOS builds are supported")
endif()

if(WIN32)
	# Prevent Windows headers from defining min/max macros in all targets.
	add_compile_definitions(NOMINMAX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include("${ELKY5_SOURCE_DIR}/utils.cmake")
include(CTest)

option(ELKY5_ENABLE_CLANG_TIDY "Run clang-tidy checks during builds" OFF)

set(ELKY5_BUILD_ORIGIN "Source" CACHE STRING "Build provenance shown in diagnostics")
set_property(CACHE ELKY5_BUILD_ORIGIN PROPERTY STRINGS Source Official Fork)
if(NOT ELKY5_BUILD_ORIGIN STREQUAL "Source" AND NOT ELKY5_BUILD_ORIGIN STREQUAL "Official" AND
   NOT ELKY5_BUILD_ORIGIN STREQUAL "Fork")
	message(FATAL_ERROR "ELKY5_BUILD_ORIGIN must be Source, Official, or Fork")
endif()

set(ELKY5_RELEASE_TAG "" CACHE STRING "Release tag embedded in official builds")
set(ELKY5_BUILD_REPOSITORY "" CACHE STRING "GitHub repository embedded in fork builds")
set(ELKY5_OFFICIAL_BUILD OFF)
set(ELKY5_FORK_BUILD OFF)
if(ELKY5_BUILD_ORIGIN STREQUAL "Official")
	if(ELKY5_RELEASE_TAG STREQUAL "")
		message(FATAL_ERROR "Official builds require ELKY5_RELEASE_TAG")
	endif()
	set(ELKY5_OFFICIAL_BUILD ON)
elseif(ELKY5_BUILD_ORIGIN STREQUAL "Fork")
	if(ELKY5_BUILD_REPOSITORY STREQUAL "")
		message(FATAL_ERROR "Fork builds require ELKY5_BUILD_REPOSITORY")
	endif()
	set(ELKY5_FORK_BUILD ON)
endif()

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "only 64-bit builds are supported")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
	set(ELKY5_BUILD ELKY5_BUILD_DEBUG)
else()
	set(ELKY5_BUILD ELKY5_BUILD_RELEASE)
endif()

if(LINUX OR APPLE)
	# macOS rides the POSIX/Linux code paths until it gets a dedicated platform
	set(ELKY5_PLATFORM ELKY5_PLATFORM_LINUX)
else()
	set(ELKY5_PLATFORM ELKY5_PLATFORM_WINDOWS)
endif()

set(CLANG FALSE)
set(GCC FALSE)
set(ELKY5_CLANG_CL FALSE)
get_filename_component(ELKY5_CXX_COMPILER_NAME "${CMAKE_CXX_COMPILER}" NAME_WE)
string(TOLOWER "${ELKY5_CXX_COMPILER_NAME}" ELKY5_CXX_COMPILER_NAME)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
	set(CLANG TRUE)
	set(ELKY5_COMPILER CLANG)
	if(WIN32 AND ELKY5_CXX_COMPILER_NAME STREQUAL "clang-cl")
		set(ELKY5_CLANG_CL TRUE)
		set(ELKY5_LINKER LLD_LINK)
	elseif(APPLE)
		set(ELKY5_LINKER LD64) # Apple's default linker; lld flags don't apply
	else()
		set(ELKY5_LINKER LLD)
	endif()
	if ((CMAKE_AR MATCHES "llvm") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0.0) AND (CMAKE_CXX_COMPILER MATCHES "winlibs"))
		string(REPLACE "llvm-ar.exe" "ar.exe" CMAKE_AR ${CMAKE_AR}) # llvm-ar.exe 11.0.0 from winlibs is broken
	endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(GCC TRUE)
	set(ELKY5_COMPILER GCC)
	set(ELKY5_LINKER LD)
else()
	message(FATAL_ERROR "only Clang and GCC are supported; found ${CMAKE_CXX_COMPILER_ID}")
endif()

set(ELKY5_PROJECT_NAME "Emulator" CACHE STRING "Project name suffix")

string(TOLOWER ${ELKY5_COMPILER} ELKY5_COMPILER_ID)
string(TOLOWER ${ELKY5_LINKER} ELKY5_LINKER_ID)

if (CLANG AND (ELKY5_LINKER STREQUAL LLD))
	set(ELKY5_LD_OPTIONS "-fuse-ld=lld")
endif()

if (ELKY5_LINKER STREQUAL LD AND NOT LINUX)
	set(ELKY5_LD_OPTIONS "-Wl,--image-base=0x100000000000")
endif()

project(Elky5${ELKY5_PROJECT_NAME}${CMAKE_BUILD_TYPE}${ELKY5_COMPILER} VERSION 0.3.0)

include_directories(
	${ELKY5_THIRD_PARTY_DIR}/gtest/include
	${ELKY5_THIRD_PARTY_DIR}/gtest
	${ELKY5_THIRD_PARTY_DIR}/fmt/include
	${ELKY5_THIRD_PARTY_DIR}/magic_enum/include/magic_enum
	${PROJECT_BINARY_DIR}
	${ELKY5_SOURCE_DIR}
)

set(ELKY5_VERSION "${PROJECT_VERSION}")

configure_file(
  ${ELKY5_SOURCE_DIR}/cmake_config.h.in
  ${PROJECT_BINARY_DIR}/cmake_config.h
  )

find_package(Git)

add_custom_target(Elky5GitVersion
	COMMAND ${CMAKE_COMMAND}
		-D INPUT_FILE=${ELKY5_SOURCE_DIR}/elky5GitVersion.h.in
		-D OUTPUT_FILE=${PROJECT_BINARY_DIR}/elky5GitVersion.h
		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
		-D GIT_WORKING_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}
		-P ${ELKY5_SOURCE_DIR}/generate_version.cmake
	BYPRODUCTS ${PROJECT_BINARY_DIR}/elky5GitVersion.h
	COMMENT "Generate elky5GitVersion.h"
)

add_library(elky5_git_version INTERFACE)
add_dependencies(elky5_git_version Elky5GitVersion)

if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12.0.0)
	list(APPEND ELKY5_IWYU
		elky5_emulator
		common
		#launcher
	)
	list(APPEND ELKY5_CLANG_TIDY
		elky5_emulator
		#common
		launcher
	)
endif()

option(ELKY5_BUILD_LAUNCHER "Build Qt launcher" ON)

config_compiler_and_linker()

add_subdirectory("${ELKY5_THIRD_PARTY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/3rdparty")
add_subdirectory("${ELKY5_SOURCE_DIR}/common" "${CMAKE_CURRENT_BINARY_DIR}/common")

file(GLOB elky5_emulator_src CONFIGURE_DEPENDS
	"${ELKY5_SOURCE_DIR}/libs/*.cpp"
	"${ELKY5_SOURCE_DIR}/libs/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/command_processor/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/command_processor/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/cache/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/cache/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/image/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/image/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/pipeline/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/pipeline/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/upscaler/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/upscaler/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/cfg/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/cfg/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/decode/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/decode/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/translate/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/frontend/translate/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/backend/spirv/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/backend/spirv/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/opcodes/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/opcodes/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/opcodes/*.inc"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/presentation/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/presentation/*.h"
	"${ELKY5_SOURCE_DIR}/graphics/presentation/window/*.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/presentation/window/*.h"
	"${ELKY5_SOURCE_DIR}/kernel/*.cpp"
	"${ELKY5_SOURCE_DIR}/kernel/*.h"
	"${ELKY5_SOURCE_DIR}/loader/*.cpp"
	"${ELKY5_SOURCE_DIR}/loader/*.h"
)

find_program(ELKY5_GLSLANG_VALIDATOR NAMES glslangValidator glslang REQUIRED)
set(gpu_tiler_shader_dir "${ELKY5_SOURCE_DIR}/graphics/host_gpu/shaders")
set(gpu_tiler_generated_dir "${PROJECT_BINARY_DIR}/gpu_tiler_shaders")
set(gpu_tiler_shader_names
	standard256
	standard4
	standard4_3d
	standard64
	standard64_3d
	prt
	prt_3d
	render_target
	depth
	promote_d16
	demote_d16
	color_transform
	color_clear
)
file(GLOB gpu_tiler_shader_includes CONFIGURE_DEPENDS "${gpu_tiler_shader_dir}/gpu_tiler_*.inc")
foreach(shader_name IN LISTS gpu_tiler_shader_names)
	set(shader_source "${gpu_tiler_shader_dir}/gpu_tiler_${shader_name}.comp")
	set(shader_spv "${gpu_tiler_generated_dir}/gpu_tiler_${shader_name}.spv")
	set(shader_header "${gpu_tiler_generated_dir}/gpu_tiler_${shader_name}_spv.h")
	string(TOUPPER "GPU_TILER_${shader_name}_SPV" shader_symbol)
	add_custom_command(
		OUTPUT "${shader_header}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${gpu_tiler_generated_dir}"
		COMMAND "${ELKY5_GLSLANG_VALIDATOR}" -V --target-env vulkan1.3 -Os
		        "-I${gpu_tiler_shader_dir}" -o "${shader_spv}" "${shader_source}"
		COMMAND ${CMAKE_COMMAND} -DINPUT=${shader_spv} -DOUTPUT=${shader_header}
		        -DSYMBOL=${shader_symbol} -P "${ELKY5_SOURCE_DIR}/embed_spirv.cmake"
		DEPENDS "${shader_source}" ${gpu_tiler_shader_includes}
		VERBATIM
	)
	list(APPEND gpu_tiler_shader_headers "${shader_header}")
endforeach()

set(fault_buffer_shader_source "${gpu_tiler_shader_dir}/fault_buffer_process.comp")
set(fault_buffer_shader_spv "${gpu_tiler_generated_dir}/fault_buffer_process.spv")
set(fault_buffer_shader_header "${gpu_tiler_generated_dir}/fault_buffer_process_spv.h")
add_custom_command(
	OUTPUT "${fault_buffer_shader_header}"
	COMMAND ${CMAKE_COMMAND} -E make_directory "${gpu_tiler_generated_dir}"
	COMMAND "${ELKY5_GLSLANG_VALIDATOR}" -V --target-env vulkan1.3 -Os
	        -DCACHING_PAGEBITS=14 -DMAX_PAGE_FAULTS=1024
	        -o "${fault_buffer_shader_spv}" "${fault_buffer_shader_source}"
	COMMAND ${CMAKE_COMMAND} -DINPUT=${fault_buffer_shader_spv} -DOUTPUT=${fault_buffer_shader_header}
	        -DSYMBOL=FAULT_BUFFER_PROCESS_SPV -P "${ELKY5_SOURCE_DIR}/embed_spirv.cmake"
	DEPENDS "${fault_buffer_shader_source}"
	VERBATIM
)
list(APPEND elky5_emulator_src "${fault_buffer_shader_header}")
list(APPEND elky5_emulator_src ${gpu_tiler_shader_headers})

set(gpu_blit_generated_dir "${PROJECT_BINARY_DIR}/gpu_blit_shaders")
function(add_gpu_blit_shader shader_name shader_file)
	set(shader_source "${gpu_tiler_shader_dir}/${shader_file}")
	set(shader_spv "${gpu_blit_generated_dir}/${shader_name}.spv")
	set(shader_header "${gpu_blit_generated_dir}/${shader_name}_spv.h")
	string(TOUPPER "${shader_name}_SPV" shader_symbol)
	add_custom_command(
		OUTPUT "${shader_header}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${gpu_blit_generated_dir}"
		COMMAND "${ELKY5_GLSLANG_VALIDATOR}" -V --target-env vulkan1.3 -Os ${ARGN}
		        "-I${gpu_tiler_shader_dir}" -o "${shader_spv}" "${shader_source}"
		COMMAND ${CMAKE_COMMAND} -DINPUT=${shader_spv} -DOUTPUT=${shader_header}
		        -DSYMBOL=${shader_symbol} -P "${ELKY5_SOURCE_DIR}/embed_spirv.cmake"
		DEPENDS "${shader_source}" "${gpu_tiler_shader_dir}/gpu_video_out_hdr10_to_sdr.inc"
		VERBATIM
	)
	set(gpu_blit_shader_headers ${gpu_blit_shader_headers} "${shader_header}" PARENT_SCOPE)
endfunction()
add_gpu_blit_shader(gpu_blit_fs_triangle gpu_blit_fs_triangle.vert)
add_gpu_blit_shader(gpu_blit_color_to_ms_depth gpu_blit_color_to_ms_depth.frag)
add_gpu_blit_shader(gpu_video_out_overlay gpu_video_out_overlay.frag)
add_gpu_blit_shader(gpu_imgui_hdr gpu_video_out_overlay.frag -DIMGUI_HDR=1)
add_gpu_blit_shader(gpu_pssr_linear gpu_pssr_linear.comp)
add_gpu_blit_shader(gpu_bvh_mirror gpu_bvh_mirror.comp)
add_gpu_blit_shader(gpu_bvh_walk gpu_bvh_mirror.comp -DWALK=1)
# At SPIR-V 1.6 glslang declares a task shader's workgroup size with LocalSizeId, which needs maintenance4.
add_gpu_blit_shader(gpu_mesh_indirect gpu_mesh_indirect.task --target-env spirv1.5)
list(APPEND elky5_emulator_src ${gpu_blit_shader_headers})

list(APPEND elky5_emulator_src
	"${ELKY5_SOURCE_DIR}/emulator.h"
	"${ELKY5_SOURCE_DIR}/emulator.cpp"
)
list(REMOVE_DUPLICATES elky5_emulator_src)

if (ELKY5_CLANG_CL)
	set_source_files_properties(${elky5_emulator_src} PROPERTIES COMPILE_FLAGS "-Wno-pragma-pack -Wno-deprecated-declarations -D_TIMESPEC_DEFINED")
endif()

set(elky5_emulator_link_libraries common Vulkan::Headers spirv-tools-opt spirv-tools SDL3::SDL3-static imgui xxhash FFmpeg::ffmpeg fmt::fmt nlohmann_json::nlohmann_json LibAtrac9 elky5_git_version Zydis::Zydis xbyak::xbyak opus OpenXR::openxr_loader libzstd_static)

# Linux system libraries.
if(LINUX)
	find_package(Threads REQUIRED)
	list(APPEND elky5_emulator_link_libraries m ${CMAKE_DL_LIBS} Threads::Threads)
endif()

set(inc_headers
	${ELKY5_SOURCE_DIR}
	${ELKY5_THIRD_PARTY_DIR}/VulkanMemoryAllocator/include
	${ELKY5_THIRD_PARTY_DIR}/SPIRV-Tools/include
	${ELKY5_THIRD_PARTY_DIR}/SPIRV-Headers/include
	${ELKY5_THIRD_PARTY_DIR}/fmt/include
	${ELKY5_THIRD_PARTY_DIR}/xxHash
	${ELKY5_THIRD_PARTY_DIR}/cpuinfo/include
	${ELKY5_THIRD_PARTY_DIR}/renderdoc
	${ELKY5_THIRD_PARTY_DIR}/stb
	${ELKY5_THIRD_PARTY_DIR}/ffx-api
)

if (ELKY5_CLANG_CL)
	list(APPEND elky5_emulator_link_libraries winpthread)
	list(APPEND inc_headers
		${ELKY5_THIRD_PARTY_DIR}/winpthread/include
	)
endif()

list(APPEND check_headers
	${ELKY5_SOURCE_DIR}
)

function(add_elky5_full_emulator_test target source)
	add_executable(${target} EXCLUDE_FROM_ALL ${source} ${elky5_emulator_src})
	target_link_libraries(${target} ${elky5_emulator_link_libraries})
	target_include_directories(${target} PRIVATE ${inc_headers})
	if (WIN32)
		target_link_libraries(${target} iphlpapi dxgi)
	endif()
	if (ELKY5_CLANG_CL)
		target_link_libraries(${target} onecore)
		add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ELKY5_THIRD_PARTY_DIR}/winpthread/bin/libwinpthread-1.dll" $<TARGET_FILE_DIR:${target}>/libwinpthread-1.dll)
	endif()
	# The macOS x86_64 guest address space needs its .zerofill segments anchored
	# by linker flags, or the kernel kills the binary on load (posix_spawn EIO).
	configure_macos_guest_address_space(${target})
endfunction()

function(configure_macos_guest_address_space target)
	if(APPLE AND (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64" OR
			(NOT CMAKE_OSX_ARCHITECTURES AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")))
		target_sources(${target} PRIVATE "${ELKY5_SOURCE_DIR}/kernel/macosGuestAddressSpace.cpp")
		target_compile_definitions(${target} PRIVATE ELKY5_LINKED_GUEST_ADDRESS_SPACE=1)
		target_link_options(${target} PRIVATE
			-Wl,-ld_classic,-no_pie,-no_fixup_chains,-no_huge,-pagezero_size,0x40000,-segaddr,SYSTEM_MANAGED,0x40000,-segaddr,SYSTEM_RESERVED,0x7ffffc000,-segaddr,USER_AREA,0x7000000000,-image_base,0x700000000000)
	endif()
endfunction()

add_elky5_full_emulator_test(shader_cfg_tests "${ELKY5_TESTS_DIR}/shaderCfgTests.cpp")

add_executable(scalar_provenance_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ScalarProvenanceTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/gpu_format.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ReadLaneElimination.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ResourceMaterialization.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/SrtWalker.cpp"
)
target_link_libraries(scalar_provenance_tests fmt::fmt xbyak::xbyak)
target_include_directories(scalar_provenance_tests PRIVATE ${inc_headers})

add_executable(build_flags_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/BuildFlagsTests.cpp")

add_executable(page_manager_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/PageManagerTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/pageManager.cpp"
)
target_link_libraries(page_manager_tests common)
target_include_directories(page_manager_tests PRIVATE ${inc_headers})

add_executable(archive_file_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ArchiveFileTests.cpp"
)
target_link_libraries(archive_file_tests common ZArchive::zarchive)
target_include_directories(archive_file_tests PRIVATE ${inc_headers})

add_executable(trophy_system_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/TrophySystemTests.cpp"
)
target_link_libraries(trophy_system_tests common Vulkan::Headers)
target_include_directories(trophy_system_tests PRIVATE ${inc_headers})

add_executable(avplayer_file_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/AvPlayerFileTests.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(avplayer_file_tests common FFmpeg::ffmpeg ZArchive::zarchive)
target_include_directories(avplayer_file_tests PRIVATE ${inc_headers})

add_executable(vdecsw_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/VdecswTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/videoDec2Decoder.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(vdecsw_tests common FFmpeg::ffmpeg)
target_include_directories(vdecsw_tests PRIVATE ${inc_headers})

add_executable(bit_array_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/BitArrayTests.cpp"
)
target_include_directories(bit_array_tests PRIVATE ${inc_headers})

add_executable(lru_cache_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/LruCacheTests.cpp"
)
target_include_directories(lru_cache_tests PRIVATE ${inc_headers})

add_executable(slot_vector_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/SlotVectorTests.cpp"
)
target_include_directories(slot_vector_tests PRIVATE ${inc_headers})

add_executable(common_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/CommonTests.cpp")
target_link_libraries(common_tests common elky5_test_gtest)

add_executable(ime_dialog_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ImeDialogTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/imeCommon.cpp"
	"${ELKY5_SOURCE_DIR}/libs/imeDialog.cpp"
)
target_include_directories(ime_dialog_tests PRIVATE ${inc_headers})

add_executable(memory_tracker_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/MemoryTrackerTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/pageManager.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/memoryTracker.cpp"
)
target_link_libraries(memory_tracker_tests fmt::fmt common)
target_include_directories(memory_tracker_tests PRIVATE ${inc_headers})

add_executable(shader_vertex_metadata_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ShaderVertexMetadataTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/shaderVertexMetadata.cpp"
)
target_include_directories(shader_vertex_metadata_tests PRIVATE ${inc_headers})

add_executable(resource_materialization_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ResourceMaterializationTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/gpu_format.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ResourceMaterialization.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/SrtWalker.cpp"
)
target_link_libraries(resource_materialization_tests fmt::fmt xbyak::xbyak)
target_include_directories(resource_materialization_tests PRIVATE ${inc_headers})

add_executable(resource_tracking_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ResourceTrackingTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/guest_gpu/gpu_format.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/shaderPixelParameter.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/SrtWalker.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ResourceTracking.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ResourceMaterialization.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/shader/recompiler/ir/passes/ShaderInfoCollection.cpp"
)
target_link_libraries(resource_tracking_tests fmt::fmt xbyak::xbyak)
target_include_directories(resource_tracking_tests PRIVATE ${inc_headers})

add_executable(audio_out2_port_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/AudioOut2PortTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libAudio2.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(audio_out2_port_tests common fmt::fmt)
target_include_directories(audio_out2_port_tests PRIVATE ${inc_headers})

add_executable(audio_out_timing_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/AudioOutTimingTests.cpp")
target_link_libraries(audio_out_timing_tests common fmt::fmt SDL3::SDL3-static)
target_include_directories(audio_out_timing_tests PRIVATE ${inc_headers})

add_executable(ngs2_sampler_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/Ngs2SamplerTests.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(ngs2_sampler_tests common fmt::fmt LibAtrac9)
target_include_directories(ngs2_sampler_tests PRIVATE ${inc_headers})

add_executable(ajm_resample_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/AjmResampleTests.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(ajm_resample_tests common fmt::fmt FFmpeg::ffmpeg LibAtrac9 opus)
target_include_directories(ajm_resample_tests PRIVATE ${inc_headers})

add_executable(pad_haptics_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/PadHapticsTests.cpp"
)
target_link_libraries(pad_haptics_tests common fmt::fmt SDL3::SDL3-static opus)
target_include_directories(pad_haptics_tests PRIVATE ${inc_headers})

add_executable(controller_settings_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ControllerSettingsTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/padVr.cpp"
)
target_link_libraries(controller_settings_tests common fmt::fmt SDL3::SDL3-static)
target_include_directories(controller_settings_tests PRIVATE ${inc_headers})

add_executable(host_input_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/HostInputTests.cpp")
target_link_libraries(host_input_tests common fmt::fmt SDL3::SDL3-static)
target_include_directories(host_input_tests PRIVATE ${inc_headers})

add_executable(save_data_memory_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/SaveDataMemoryTests.cpp"
)
target_link_libraries(save_data_memory_tests common fmt::fmt)
target_include_directories(save_data_memory_tests PRIVATE ${inc_headers})

add_executable(png_enc_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/PngEncTests.cpp")
target_link_libraries(png_enc_tests common fmt::fmt)
target_include_directories(png_enc_tests PRIVATE ${inc_headers})

add_executable(jpeg_dec_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/JpegDecTests.cpp")
target_link_libraries(jpeg_dec_tests common fmt::fmt)
target_include_directories(jpeg_dec_tests PRIVATE ${inc_headers})

add_executable(jpeg_enc_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/JpegEncTests.cpp")
target_link_libraries(jpeg_enc_tests common fmt::fmt)
target_include_directories(jpeg_enc_tests PRIVATE ${inc_headers})

add_executable(occlusion_queries_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/OcclusionQueriesTests.cpp")
target_link_libraries(occlusion_queries_tests common fmt::fmt Vulkan::Headers)
target_include_directories(occlusion_queries_tests PRIVATE ${inc_headers})

add_executable(ces_tests EXCLUDE_FROM_ALL "${ELKY5_TESTS_DIR}/CesTests.cpp")
target_link_libraries(ces_tests common fmt::fmt SDL3::SDL3-static)
target_include_directories(ces_tests PRIVATE ${inc_headers})

add_executable(save_data_dialog_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/SaveDataDialogTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/dialog.cpp"
)
target_link_libraries(save_data_dialog_tests common fmt::fmt)
target_include_directories(save_data_dialog_tests PRIVATE ${inc_headers})

add_elky5_full_emulator_test(http_uri_parse_tests
	"${ELKY5_TESTS_DIR}/HttpUriParseTests.cpp")

add_executable(rtc_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/RtcTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libRtc.cpp"
	"${ELKY5_SOURCE_DIR}/loader/symbolDatabase.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp")
target_link_libraries(rtc_tests common elky5_test_gtest)
target_include_directories(rtc_tests PRIVATE ${inc_headers})

add_executable(playgo_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/PlayGoTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libPlayGo.cpp"
	"${ELKY5_SOURCE_DIR}/loader/symbolDatabase.cpp"
	"${ELKY5_SOURCE_DIR}/loader/systemContent.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp")
target_link_libraries(playgo_tests common nlohmann_json::nlohmann_json elky5_test_gtest)
target_include_directories(playgo_tests PRIVATE ${inc_headers})

add_executable(vr_tracker2_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/VrTracker2Tests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libVrTracker2.cpp"
	"${ELKY5_SOURCE_DIR}/loader/symbolDatabase.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp")
target_link_libraries(vr_tracker2_tests common elky5_test_gtest)
target_include_directories(vr_tracker2_tests PRIVATE ${inc_headers})

add_executable(font_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/FontTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libFont.cpp"
	"${ELKY5_SOURCE_DIR}/loader/symbolDatabase.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp")
target_link_libraries(font_tests common SDL3::SDL3-static FFmpeg::ffmpeg elky5_test_gtest)
target_include_directories(font_tests PRIVATE ${inc_headers})
target_compile_definitions(font_tests PRIVATE ELKY5_NOTO_SANS_DIR="${ELKY5_NOTO_SANS_DIR}")

add_executable(event_queue_lifetime_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/EventQueueLifetimeTests.cpp"
	"${ELKY5_SOURCE_DIR}/kernel/eventQueue.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp"
)
target_link_libraries(event_queue_lifetime_tests common fmt::fmt)
target_include_directories(event_queue_lifetime_tests PRIVATE ${inc_headers})

add_elky5_full_emulator_test(kernel_file_system_tests
	"${ELKY5_TESTS_DIR}/KernelFileSystemTests.cpp")
target_link_libraries(kernel_file_system_tests ZArchive::zarchive)

add_executable(sync_on_address_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/SyncOnAddressTests.cpp"
	"${ELKY5_SOURCE_DIR}/kernel/syncOnAddress.cpp"
)
target_link_libraries(sync_on_address_tests common)
target_include_directories(sync_on_address_tests PRIVATE ${inc_headers})

add_executable(image_page_table_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/ImagePageTableTests.cpp"
)
target_link_libraries(image_page_table_tests common fmt::fmt)
target_include_directories(image_page_table_tests PRIVATE ${inc_headers})

add_elky5_full_emulator_test(shader_recompiler_compute_tests "${ELKY5_TESTS_DIR}/ShaderRecompilerComputeTests.cpp")

set(gpu_test_generated_dir "${PROJECT_BINARY_DIR}/gpu_test_shaders")
foreach(test_shader gpu_test_ms_depth gpu_test_ray_query)
	set(test_shader_spv "${gpu_test_generated_dir}/${test_shader}.spv")
	set(test_shader_header "${gpu_test_generated_dir}/${test_shader}_spv.h")
	string(TOUPPER "${test_shader}_SPV" test_shader_symbol)
	add_custom_command(
		OUTPUT "${test_shader_header}"
		COMMAND ${CMAKE_COMMAND} -E make_directory "${gpu_test_generated_dir}"
		COMMAND "${ELKY5_GLSLANG_VALIDATOR}" -V --target-env vulkan1.3 -Os
		        -o "${test_shader_spv}" "${gpu_tiler_shader_dir}/${test_shader}.comp"
		COMMAND ${CMAKE_COMMAND} -DINPUT=${test_shader_spv}
		        -DOUTPUT=${test_shader_header} -DSYMBOL=${test_shader_symbol}
		        -P "${ELKY5_SOURCE_DIR}/embed_spirv.cmake"
		DEPENDS "${gpu_tiler_shader_dir}/${test_shader}.comp"
		VERBATIM
	)
	target_sources(shader_recompiler_compute_tests PRIVATE "${test_shader_header}")
endforeach()

add_elky5_full_emulator_test(virtual_memory_allocation_tests "${ELKY5_TESTS_DIR}/VirtualMemoryAllocationTests.cpp")
add_library(elky5_test_gtest STATIC EXCLUDE_FROM_ALL
	"${ELKY5_THIRD_PARTY_DIR}/fmt/test/gtest/gmock-gtest-all.cc")
target_include_directories(elky5_test_gtest PUBLIC "${ELKY5_THIRD_PARTY_DIR}/fmt/test/gtest")
target_sources(virtual_memory_allocation_tests PRIVATE "${ELKY5_TESTS_DIR}/TrinityTests.cpp")
target_link_libraries(virtual_memory_allocation_tests elky5_test_gtest)

add_elky5_full_emulator_test(net_resolver_tests "${ELKY5_TESTS_DIR}/NetResolverTests.cpp")
target_link_libraries(net_resolver_tests elky5_test_gtest)

add_elky5_full_emulator_test(notification_tests "${ELKY5_TESTS_DIR}/NotificationTests.cpp")
target_link_libraries(notification_tests elky5_test_gtest)

add_elky5_full_emulator_test(process_exit_tests "${ELKY5_TESTS_DIR}/ProcessExitTests.cpp")
target_link_libraries(process_exit_tests elky5_test_gtest)

add_executable(dlss_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/DlssTests.cpp"
	"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/upscaler/dlss.cpp")
target_link_libraries(dlss_tests common Vulkan::Headers SDL3::SDL3-static elky5_test_gtest)
target_include_directories(dlss_tests PRIVATE ${inc_headers})

if (WIN32)
	add_executable(fsr_tests EXCLUDE_FROM_ALL
		"${ELKY5_TESTS_DIR}/FsrTests.cpp"
		"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/upscaler/dlss.cpp"
		"${ELKY5_SOURCE_DIR}/graphics/host_gpu/renderer/upscaler/fsr.cpp")
	target_link_libraries(fsr_tests common Vulkan::Headers SDL3::SDL3-static elky5_test_gtest)
	target_include_directories(fsr_tests PRIVATE ${inc_headers})
endif()

add_executable(psml_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/PsmlTests.cpp"
	"${ELKY5_SOURCE_DIR}/libs/libPsml.cpp"
	"${ELKY5_SOURCE_DIR}/loader/symbolDatabase.cpp"
	"${ELKY5_SOURCE_DIR}/loader/timer.cpp")
target_link_libraries(psml_tests common elky5_test_gtest)
target_include_directories(psml_tests PRIVATE ${inc_headers})
target_compile_definitions(virtual_memory_allocation_tests PRIVATE
	ELKY5_VIRTUAL_MEMORY_ALLOCATION_TESTS=1)

add_executable(gpu_blit_shader_tests EXCLUDE_FROM_ALL
	"${ELKY5_TESTS_DIR}/GpuBlitShaderTests.cpp"
	"${gpu_blit_generated_dir}/gpu_mesh_indirect_spv.h")
target_link_libraries(gpu_blit_shader_tests elky5_test_gtest Vulkan::Headers)
target_include_directories(gpu_blit_shader_tests PRIVATE ${inc_headers})

# These tests use exceptions.
if(NOT ELKY5_CLANG_CL)
	foreach(elky5_exception_test scalar_provenance_tests resource_tracking_tests
			virtual_memory_allocation_tests)
		target_compile_options(${elky5_exception_test} PRIVATE -fexceptions)
	endforeach()
endif()

if(BUILD_TESTING)
	add_test(NAME emulator_user_name_cli
	         COMMAND $<TARGET_FILE:elky5_emulator> --help --user-name 1234567890123456)
	add_test(NAME emulator_user_name_too_long
	         COMMAND $<TARGET_FILE:elky5_emulator> --help --user-name 12345678901234567)
	set_tests_properties(emulator_user_name_too_long PROPERTIES WILL_FAIL TRUE)
	add_test(NAME emulator_present_mode_cli
	         COMMAND $<TARGET_FILE:elky5_emulator> --help --present-mode Immediate)
	add_test(NAME emulator_present_mode_invalid
	         COMMAND $<TARGET_FILE:elky5_emulator> --help --present-mode Invalid)
	set_tests_properties(emulator_present_mode_invalid PROPERTIES WILL_FAIL TRUE)
	add_test(NAME ime_dialog COMMAND $<TARGET_FILE:ime_dialog_tests>)
	add_test(NAME shader_cfg COMMAND $<TARGET_FILE:shader_cfg_tests>)
	add_test(NAME scalar_provenance COMMAND $<TARGET_FILE:scalar_provenance_tests>)
	add_test(NAME image_page_table COMMAND $<TARGET_FILE:image_page_table_tests>)
	add_test(NAME memory_tracker COMMAND $<TARGET_FILE:memory_tracker_tests>)
	add_test(NAME page_manager COMMAND $<TARGET_FILE:page_manager_tests>)
	add_test(NAME build_flags COMMAND $<TARGET_FILE:build_flags_tests>)
	add_test(NAME archive_file COMMAND $<TARGET_FILE:archive_file_tests>)
	add_test(NAME trophy_system COMMAND $<TARGET_FILE:trophy_system_tests>)
	add_test(NAME avplayer_file COMMAND $<TARGET_FILE:avplayer_file_tests>)
	add_test(NAME vdecsw COMMAND $<TARGET_FILE:vdecsw_tests>)
	add_test(NAME bit_array COMMAND $<TARGET_FILE:bit_array_tests>)
	add_test(NAME lru_cache COMMAND $<TARGET_FILE:lru_cache_tests>)
	add_test(NAME common COMMAND $<TARGET_FILE:common_tests>)
	add_test(NAME shader_vertex_metadata COMMAND $<TARGET_FILE:shader_vertex_metadata_tests>)
	add_test(NAME resource_materialization COMMAND $<TARGET_FILE:resource_materialization_tests>)
	add_test(NAME resource_tracking COMMAND $<TARGET_FILE:resource_tracking_tests>)
	add_test(NAME event_queue_lifetime COMMAND $<TARGET_FILE:event_queue_lifetime_tests>)
	add_test(NAME kernel_file_system COMMAND $<TARGET_FILE:kernel_file_system_tests>)
	add_test(NAME sync_on_address COMMAND $<TARGET_FILE:sync_on_address_tests>)
	add_test(NAME audio_out2_port COMMAND $<TARGET_FILE:audio_out2_port_tests>)
	add_test(NAME audio_out_timing COMMAND $<TARGET_FILE:audio_out_timing_tests>)
	add_test(NAME ngs2_sampler COMMAND $<TARGET_FILE:ngs2_sampler_tests>)
	add_test(NAME ajm_resample COMMAND $<TARGET_FILE:ajm_resample_tests>)
	add_test(NAME pad_haptics COMMAND $<TARGET_FILE:pad_haptics_tests>)
	add_test(NAME controller_settings COMMAND $<TARGET_FILE:controller_settings_tests>)
	add_test(NAME host_input COMMAND $<TARGET_FILE:host_input_tests>)
	add_test(NAME save_data_memory COMMAND $<TARGET_FILE:save_data_memory_tests>)
	add_test(NAME png_enc COMMAND $<TARGET_FILE:png_enc_tests>)
	add_test(NAME jpeg_dec COMMAND $<TARGET_FILE:jpeg_dec_tests>)
	add_test(NAME jpeg_enc COMMAND $<TARGET_FILE:jpeg_enc_tests>)
	add_test(NAME occlusion_queries COMMAND $<TARGET_FILE:occlusion_queries_tests>)
	add_test(NAME ces COMMAND $<TARGET_FILE:ces_tests>)
	add_test(NAME save_data_dialog COMMAND $<TARGET_FILE:save_data_dialog_tests>)
	add_test(NAME http_uri_parse COMMAND $<TARGET_FILE:http_uri_parse_tests>)
	add_test(NAME rtc COMMAND $<TARGET_FILE:rtc_tests>)
	add_test(NAME playgo COMMAND $<TARGET_FILE:playgo_tests>)
	add_test(NAME vr_tracker2 COMMAND $<TARGET_FILE:vr_tracker2_tests>)
	add_test(NAME shader_recompiler_compute COMMAND $<TARGET_FILE:shader_recompiler_compute_tests>)
	add_test(NAME shader_recompiler_tessellation
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --tessellation-only)
	add_test(NAME shader_recompiler_float_min_max
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --float-min-max-only)
	add_test(NAME shader_recompiler_relaxed_float
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --relaxed-float-only)
	add_test(NAME shader_recompiler_vop3_conversions
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --vop3-conversions-only)
	add_test(NAME shader_recompiler_quadmask
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --quadmask-only)
	add_test(NAME shader_recompiler_integer16_ternary
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --integer16-ternary-only)
	add_test(NAME shader_recompiler_mad_u16
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --mad-u16-only)
	add_test(NAME shader_recompiler_mul_hi24
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --mul-hi24-only)
	add_test(NAME shader_recompiler_alignbyte
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --alignbyte-only)
	add_test(NAME graphics_pipeline_rasterization
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --polygon-mode-only)
	add_test(NAME graphics_draw_offsets
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --draw-offset-only)
	add_test(NAME virtual_memory_allocation
		COMMAND $<TARGET_FILE:virtual_memory_allocation_tests>)
	add_test(NAME trinity_mode
		COMMAND $<TARGET_FILE:virtual_memory_allocation_tests> --trinity-only)
	add_test(NAME psml COMMAND $<TARGET_FILE:psml_tests>)
	add_test(NAME notification COMMAND $<TARGET_FILE:notification_tests>)
	add_test(NAME process_exit COMMAND $<TARGET_FILE:process_exit_tests>)
	add_test(NAME slot_vector COMMAND $<TARGET_FILE:slot_vector_tests>)
	add_test(NAME font COMMAND $<TARGET_FILE:font_tests>)
	add_test(NAME dlss COMMAND $<TARGET_FILE:dlss_tests>)
	add_test(NAME gpu_blit_shader COMMAND $<TARGET_FILE:gpu_blit_shader_tests>)
	if (WIN32)
		add_test(NAME fsr COMMAND $<TARGET_FILE:fsr_tests>)
	endif()
	add_test(NAME pssr COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --pssr-only)
	add_test(NAME guest_red_zone_patcher
		COMMAND $<TARGET_FILE:virtual_memory_allocation_tests> --red-zone-patcher-only)
	add_test(NAME command_scheduler_timeline
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --scheduler-only)
	add_test(NAME stream_buffer_ring
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --stream-buffer-only)
	add_test(NAME gpu_command_lane
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --gpu-command-lane-only)
	add_test(NAME pm4_flip
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --flip-only)
	add_test(NAME video_out_flip_rate
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --flip-rate-only)
	add_test(NAME video_out_hdr10_to_sdr
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --video-out-only)
	add_test(NAME depth_bits
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --depth-bits-only)
	add_test(NAME srgb_texture
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --srgb-texture-only)
	add_test(NAME device_creation
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --device-creation-only)
	add_test(NAME msaa_storage
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --msaa-storage-only)
	add_test(NAME pm4_context_state
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --context-state-only)
	add_test(NAME gpu_tiler
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --gpu-tiler-only)
	add_test(NAME image_transfer_layout
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --image-layout-only)

	add_test(NAME texture_cache_layered_image
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --layered-image-only)
	add_test(NAME host_image_allocation
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --host-image-allocation-only)
	add_test(NAME texture_cache_image_views
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --image-view-cache-only)
	add_test(NAME texture_cache_storage_sampled
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --storage-sampled-only)
	add_test(NAME texture_cache_depth_readback
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --depth-readback-only)
	add_test(NAME compute_meta_clear_classification
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --compute-meta-clear-only)
	add_test(NAME buffer_cache_dirty_gc
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --buffer-cache-gc-only)
	add_test(NAME color_render_target_1d
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --rt1d-only)

	add_test(NAME texture_cache_image_overlap
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --image-overlap-only)
	add_test(NAME texture_cache_htile_clear
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --htile-clear-only)
	add_test(NAME buffer_cache_ranges
		COMMAND $<TARGET_FILE:shader_recompiler_compute_tests> --buffer-cache-range-only)

	add_custom_target(elky5_tests DEPENDS
		ime_dialog_tests
		shader_cfg_tests
		scalar_provenance_tests
		image_page_table_tests
		memory_tracker_tests
		page_manager_tests
		build_flags_tests
		archive_file_tests
		trophy_system_tests
		avplayer_file_tests
		vdecsw_tests
		bit_array_tests
		lru_cache_tests
		common_tests
		shader_vertex_metadata_tests
		resource_materialization_tests
		resource_tracking_tests
		event_queue_lifetime_tests
		kernel_file_system_tests
		sync_on_address_tests
		shader_recompiler_compute_tests
		virtual_memory_allocation_tests
		audio_out2_port_tests
		audio_out_timing_tests
		ngs2_sampler_tests
		ajm_resample_tests
		pad_haptics_tests
		controller_settings_tests
		host_input_tests
		save_data_memory_tests
		png_enc_tests
		jpeg_dec_tests
		jpeg_enc_tests
		occlusion_queries_tests
		ces_tests
		save_data_dialog_tests
		http_uri_parse_tests
		rtc_tests
		playgo_tests
		vr_tracker2_tests
		notification_tests
		process_exit_tests
		psml_tests
		slot_vector_tests
		font_tests
		dlss_tests
		gpu_blit_shader_tests
	)
	if (WIN32)
		add_dependencies(elky5_tests fsr_tests)
	endif()
endif()

# Test Darwin signal-context restoration for SSE4a instructions.
if(APPLE AND (CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64" OR
		(NOT CMAKE_OSX_ARCHITECTURES AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")))
	add_executable(macos_sse4a_tests EXCLUDE_FROM_ALL
		"${ELKY5_TESTS_DIR}/macosSse4aTests.cpp"
		"${ELKY5_SOURCE_DIR}/loader/x64InstructionEmulator.cpp"
		"${ELKY5_SOURCE_DIR}/loader/guestInstructionPatcher.cpp"
	)
	target_link_libraries(macos_sse4a_tests common fmt::fmt Zydis::Zydis xbyak::xbyak)
	target_include_directories(macos_sse4a_tests PRIVATE ${inc_headers})
	if(BUILD_TESTING)
		add_test(NAME macos_sse4a COMMAND $<TARGET_FILE:macos_sse4a_tests>)
		set_tests_properties(macos_sse4a PROPERTIES TIMEOUT 15)
		add_dependencies(elky5_tests macos_sse4a_tests)
	endif()
endif()

add_executable(elky5_emulator "${ELKY5_SOURCE_DIR}/main.cpp" ${elky5_emulator_src})
configure_macos_guest_address_space(elky5_emulator)

set(ELKY5_TROPHY_UNLOCK_SOUND "${CMAKE_CURRENT_SOURCE_DIR}/assets/sounds/trophy-unlock.wav")
add_custom_target(elky5_trophy_sound
	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:elky5_emulator>/assets/sounds"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ELKY5_TROPHY_UNLOCK_SOUND}"
		"$<TARGET_FILE_DIR:elky5_emulator>/assets/sounds/trophy-unlock.wav"
	VERBATIM)
add_dependencies(elky5_emulator elky5_trophy_sound)
install(FILES "${ELKY5_TROPHY_UNLOCK_SOUND}" DESTINATION assets/sounds)

add_custom_target(elky5_touch_cursors
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets/cursors"
		"$<TARGET_FILE_DIR:elky5_emulator>/assets/cursors"
	VERBATIM)
add_dependencies(elky5_emulator elky5_touch_cursors)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/cursors" DESTINATION assets)

set(ELKY5_NOTO_SANS_FONTS
	"${ELKY5_NOTO_SANS_DIR}/NotoSans-Regular.ttf"
	"${ELKY5_NOTO_SANS_DIR}/NotoSans-Bold.ttf"
	"${ELKY5_NOTO_SANS_DIR}/NotoSans-Italic.ttf"
	"${ELKY5_NOTO_SANS_DIR}/NotoSans-BoldItalic.ttf")
add_custom_target(elky5_noto_sans_fonts
	COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:elky5_emulator>/assets/fonts"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ELKY5_NOTO_SANS_FONTS}
		"$<TARGET_FILE_DIR:elky5_emulator>/assets/fonts"
	VERBATIM)
add_dependencies(elky5_emulator elky5_noto_sans_fonts)
install(FILES ${ELKY5_NOTO_SANS_FONTS} DESTINATION assets/fonts)

target_link_libraries(elky5_emulator ${elky5_emulator_link_libraries})
if (WIN32)
	target_link_libraries(elky5_emulator opengl32)
	target_link_libraries(elky5_emulator iphlpapi)
	target_link_libraries(elky5_emulator setupapi)
	target_link_libraries(elky5_emulator ws2_32)
	target_link_libraries(elky5_emulator dxgi)
endif()
if (ELKY5_CLANG_CL)
	target_link_libraries(elky5_emulator onecore)
endif()
if (CLANG AND NOT ELKY5_CLANG_CL)
	target_link_libraries(elky5_emulator pthread)
endif()
# dlopen/dlsym/dladdr for RenderDoc.
target_link_libraries(elky5_emulator ${CMAKE_DL_LIBS})
target_include_directories(elky5_emulator PRIVATE ${inc_headers})

clang_tidy_check(elky5_emulator "" "${check_headers}" "${inc_headers}")

include_what_you_use(elky5_emulator "${inc_headers}")

set(ELKY5_EMULATOR_MAP_NAME "elky5_emulator_${ELKY5_COMPILER_ID}_${ELKY5_LINKER_ID}.map")

set(ELKY5_EMULATOR_MAP_LINK_PATH "${CMAKE_CURRENT_BINARY_DIR}/${ELKY5_EMULATOR_MAP_NAME}")
set(ELKY5_EMULATOR_PDB_LINK_PATH "${CMAKE_CURRENT_BINARY_DIR}/elky5_emulator.pdb")

if(ELKY5_CLANG_CL)
	target_link_options(elky5_emulator PRIVATE
		"/DYNAMICBASE:NO"
		"/DEBUG:FULL"
		"/PDB:${ELKY5_EMULATOR_PDB_LINK_PATH}"
		"/lldmap:${ELKY5_EMULATOR_MAP_LINK_PATH}"
	)
	add_custom_command(TARGET elky5_emulator POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ELKY5_THIRD_PARTY_DIR}/winpthread/bin/libwinpthread-1.dll" $<TARGET_FILE_DIR:elky5_emulator>/libwinpthread-1.dll)
elseif(WIN32 OR LINUX)
	set_target_properties(elky5_emulator PROPERTIES LINK_FLAGS "${ELKY5_LD_OPTIONS}")
	# LINK_FLAGS is a space-delimited string: a -Map path containing spaces would
	# be split into bogus arguments, so pass it as a discrete link option instead.
	target_link_options(elky5_emulator PRIVATE "-Wl,-Map=${ELKY5_EMULATOR_MAP_LINK_PATH}")
endif()

if(LINUX)
    target_link_options(elky5_emulator PRIVATE
        $<$<CONFIG:Release>:-Wl,--strip-all>
    )
endif()

if(APPLE)
	# The emulator writes x86-64 trampolines/PLT stubs into memory and executes them, and
	# runs guest code under Rosetta. That requires the JIT / unsigned-executable-memory
	# entitlements, so re-sign the binary after every link (an unsigned/relinked binary
	# reverts to the hardened defaults and aborts when it first executes written code).
	add_custom_command(TARGET elky5_emulator POST_BUILD
		COMMAND codesign -s - --force --options runtime
			--entitlements "${ELKY5_SOURCE_DIR}/macos_jit.entitlements"
			$<TARGET_FILE:elky5_emulator>
		COMMENT "Codesign elky5_emulator with JIT entitlements (macOS)")
endif()

install(TARGETS elky5_emulator DESTINATION .)
if(ELKY5_BUILD_LAUNCHER)
	add_subdirectory("${ELKY5_SOURCE_DIR}/launcher" "${CMAKE_CURRENT_BINARY_DIR}/launcher")

	if(LINUX)
		target_link_options(launcher PRIVATE $<$<CONFIG:Release>:-Wl,--strip-all>)
		target_link_options(elky5_updater PRIVATE $<$<CONFIG:Release>:-Wl,--strip-all>)
	endif()
endif()
if(ELKY5_CLANG_CL)
	install(FILES "${ELKY5_THIRD_PARTY_DIR}/winpthread/bin/libwinpthread-1.dll" DESTINATION .)
endif()
