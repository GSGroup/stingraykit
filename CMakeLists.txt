cmake_minimum_required(VERSION 2.8.3)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib)

project(stingraykit)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q  <TARGET> <LINK_FLAGS> <OBJECTS>")
SET(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> q  <TARGET> <LINK_FLAGS> <OBJECTS>")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (NOT DEFINED PLATFORM_OSPLUS)
	include(FindThreads)
endif (NOT DEFINED PLATFORM_OSPLUS)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(TestBigEndian)
include(CheckTypeSize)

include(Findlibunwind)

find_package(PkgConfig)

if (DEFINED STINGRAY_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${STINGRAY_INSTALL_PREFIX})
elseif (DEFINED STINGRAY_DEFAULT_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX ${STINGRAY_DEFAULT_INSTALL_PREFIX})
endif (DEFINED STINGRAY_INSTALL_PREFIX)

message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)

message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(PLATFORM_DARWIN TRUE)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_DARWIN=1)
endif (CMAKE_SYSTEM_NAME MATCHES "Darwin")

if (PRODUCTION_BUILD)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPRODUCTION_BUILD=1)
endif (PRODUCTION_BUILD)

if (PLATFORM_EMBEDDED)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_EMBEDDED=1)
endif (PLATFORM_EMBEDDED)

if (USE_BSON_SERIALIZATION)
	list(APPEND STINGRAYKIT_DEFINITIONS -DUSE_BSON_SERIALIZATION=1)
endif (USE_BSON_SERIALIZATION)

find_package(ICU)
if (ICU_FOUND AND ICU_I18N_FOUND)
	message(STATUS "using ICU library")
	list(APPEND STINGRAYKIT_INCLUDE_DIRECTORIES ${ICU_INCLUDE_DIRS})
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_ICU_I18N=1 -DU_STATIC_IMPLEMENTATION=1)
	list(APPEND STINGRAYKIT_LIBS ${ICU_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_DATA_LIBRARY})
else()
	message(STATUS "not using ICU library")
endif ()

CHECK_TYPE_SIZE(time_t TIME_T_SIZE)
if (TIME_T_SIZE LESS 8)
	message(STATUS "time_t size is ${TIME_T_SIZE}, the resulting program might be vulnerable to year-2038 bug!")
	add_definitions(-DSTINGRAYKIT_32_BIT_TIME_T=1)
endif()

set(stingraykit_SRC
	stingraykit/ActionTransaction.cpp
	stingraykit/BigNum.cpp
	stingraykit/EnumToString.cpp
	stingraykit/Factory.cpp
	stingraykit/ProgressValue.cpp
	stingraykit/Random.cpp
	stingraykit/SystemException.cpp
	stingraykit/TypeInfo.cpp
	stingraykit/UUID.cpp
	stingraykit/Version.cpp
	stingraykit/any.cpp
	stingraykit/self_counter.cpp
	stingraykit/shared_ptr.cpp
	stingraykit/toolkit.cpp

	stingraykit/diagnostics/AbortWrap.cpp
	stingraykit/diagnostics/AsyncProfiler.cpp
	stingraykit/diagnostics/CheckpointProfiler.cpp
	stingraykit/diagnostics/SystemProfiler.cpp

	stingraykit/function/function_name_getter.cpp

	stingraykit/io/BithreadCircularBuffer.cpp
	stingraykit/io/BufferedPipe.cpp
	stingraykit/io/DataBuffer.cpp
	stingraykit/io/DataSourceReader.cpp
	stingraykit/io/PipeDataSource.cpp
	stingraykit/io/PipeReader.cpp

	stingraykit/locale/LangCode.cpp
	stingraykit/locale/LocaleString.cpp
	stingraykit/locale/StringCodec.cpp
	stingraykit/locale/Translit.cpp

	stingraykit/log/Logger.cpp
	stingraykit/log/LoggerMessage.cpp
	stingraykit/log/LoggerStream.cpp
	stingraykit/log/SystemLogger.cpp

	stingraykit/serialization/Serialization.cpp

	stingraykit/signal/signal_policies.cpp

	stingraykit/string/TranslatedString.cpp
	stingraykit/string/Unicode.cpp
	stingraykit/string/regex.cpp
	stingraykit/string/string_stream.cpp

	stingraykit/thread/CancellationToken.cpp
	stingraykit/thread/DummyCancellationToken.cpp
	stingraykit/thread/ITaskExecutor.cpp
	stingraykit/thread/Thread.cpp
	stingraykit/thread/ThreadOperation.cpp
	stingraykit/thread/ThreadPool.cpp
	stingraykit/thread/ThreadTaskExecutor.cpp

	stingraykit/time/BrokenDownTime.cpp
	stingraykit/time/ElapsedTime.cpp
	stingraykit/time/NtpTimestamp.cpp
	stingraykit/time/Time.cpp

	stingraykit/timer/EventWatcher.cpp
	stingraykit/timer/Timer.cpp
)


if (PLATFORM_STAPI)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_STAPI=1)

	if (PLATFORM_OSPLUS)
		list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_OSPLUS=1)

		list(APPEND stingraykit_SRC
			stingraykit/thread/osplus/OS21CallOnce.cpp
			stingraykit/thread/osplus/OS21TaskExecutor.cpp
			stingraykit/thread/osplus/OS21ThreadEngine.cpp
			stingraykit/time/osplus/TimeEngine.cpp
		)
	endif (PLATFORM_OSPLUS)
endif (PLATFORM_STAPI)


if (PLATFORM_LINUX)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_LINUX=1)
	set (PLATFORM_POSIX TRUE)
endif (PLATFORM_LINUX)

if (PLATFORM_POSIX)
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_POSIX=1)

	check_function_exists(posix_spawn HAVE_POSIX_SPAWN)
	if (HAVE_POSIX_SPAWN)
		list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_POSIX_SPAWN=1)
	endif (HAVE_POSIX_SPAWN)

	set(COMMON_FLAGS "${COMMON_FLAGS} -pthread")

	list(APPEND stingraykit_SRC
		stingraykit/thread/posix/BackgroundProcess.cpp
		stingraykit/thread/posix/PosixCallOnce.cpp
		stingraykit/thread/posix/PosixConditionVariable.cpp
		stingraykit/thread/posix/PosixSemaphore.cpp
		stingraykit/thread/posix/PosixThreadEngine.cpp
		stingraykit/time/posix/TimeEngine.cpp
	)
endif (PLATFORM_POSIX)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	list(APPEND STINGRAYKIT_DEFINITIONS -DDEBUG)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

message(STATUS "Compiler identification: ${CMAKE_C_COMPILER_ID}")

if ((CMAKE_C_COMPILER_ID MATCHES "GNU") OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))
	set(PLATFORM_GCC TRUE)
	set(COMMON_FLAGS "${COMMON_FLAGS} -Wall -Werror=return-type -Wsign-compare -Wno-unused-variable -Wdisabled-optimization -D_FORTIFY_SOURCE=2")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")

	check_c_compiler_flag(-Winit-self HAVE_SELF_INIT_WARNING)
	if (HAVE_SELF_INIT_WARNING)
		set(COMMON_FLAGS "${COMMON_FLAGS} -Winit-self")
	endif (HAVE_SELF_INIT_WARNING)

	separate_arguments(DUMP_GCC_VERSION_COMMAND UNIX_COMMAND "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1} -dumpversion")
	execute_process(COMMAND ${DUMP_GCC_VERSION_COMMAND} OUTPUT_VARIABLE GCC_VERSION)
	string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
	list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
	list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)

	if ((GCC_MAJOR MATCHES "4") AND ((GCC_MINOR MATCHES "5") OR (GCC_MINOR MATCHES "7")))
		set(COMMON_FLAGS "${COMMON_FLAGS} -fno-strict-aliasing") #gcc-4.5,4.7 has known issues with strict aliasing mode
		message(STATUS "strict aliasing was turned off [frequent crashes on mips/sh4, v8 crash(4.5)]")
	else ()
		set(COMMON_FLAGS "${COMMON_FLAGS} -fstrict-aliasing")
	endif ()

	if (CMAKE_C_COMPILER_ID MATCHES "Clang")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Qunused-arguments -Wno-overloaded-virtual -Wno-shift-count-overflow")
	endif (CMAKE_C_COMPILER_ID MATCHES "Clang")

	CHECK_CXX_COMPILER_FLAG("-fstack-protector-all" HAVE_STACK_PROTECTOR)

	CHECK_CXX_COMPILER_FLAG("-fvisibility=hidden" HAVE_FVISIBILITY)
	if (HAVE_FVISIBILITY)
		list(APPEND STINGRAYKIT_DEFINITIONS -DSTINGRAYKIT_USES_GCC_VISIBILITY=1)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
		set(COMMON_FLAGS "${COMMON_FLAGS} -fvisibility=hidden")
	endif (HAVE_FVISIBILITY)

	CHECK_CXX_COMPILER_FLAG("-ffast-math" HAVE_FAST_MATH)

	if (HAVE_FAST_MATH)
		set(COMMON_FLAGS "-ffast-math ${COMMON_FLAGS}")
	endif (HAVE_FAST_MATH)

	CHECK_CXX_COMPILER_FLAG("-flto" HAVE_FLTO)
	CHECK_CXX_COMPILER_FLAG("-fwhopr" HAVE_WHOPR)
	CHECK_CXX_COMPILER_FLAG("-fwhole-program" HAVE_WHOLE_PROGRAM)
#	CHECK_CXX_LINKER_FLAG("-Wl,--sort-common" HAVE_SORT_COMMON)

	SET(CMAKE_REQUIRED_FLAGS "-Wl,--sort-common")
	CHECK_C_COMPILER_FLAG("" HAVE_SORT_COMMON)
	SET(CMAKE_REQUIRED_FLAGS "")

	if (PLATFORM_LINUX_EMU OR PLATFORM_EMU_TS)
		if (HAVE_STACK_PROTECTOR)
			message(STATUS "using stack protector")
			set(COMMON_FLAGS "${COMMON_FLAGS} -fstack-protector-all")
		else (HAVE_STACK_PROTECTOR)
			message(STATUS "no -fstack-protector-all")
		endif (HAVE_STACK_PROTECTOR)
	endif (PLATFORM_LINUX_EMU OR PLATFORM_EMU_TS)

	if (CMAKE_BUILD_TYPE MATCHES Debug)
		set (COMMON_SOURCES_FLAGS "${COMMON_SOURCES_FLAGS} -ggdb")
		set (GENERATED_SOURCES_FLAGS "${GENERATED_SOURCES_FLAGS} -ggdb1")
	endif (CMAKE_BUILD_TYPE MATCHES Debug)

	if (CMAKE_BUILD_TYPE MATCHES Release)

		set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} -Wl,--as-needed -Wl,-O1 -Wl,--relax -Wl,--no-export-dynamic")

		message(STATUS "gcc version: ${GCC_VERSION}")
		if ((CMAKE_C_COMPILER_ID MATCHES "GNU") AND (GCC_MAJOR MATCHES "3"))
			message(STATUS "USING GCC 3.X LEGACY MODE")
		else ()
			set (COMMON_FLAGS "${COMMON_FLAGS} -fdata-sections -ffunction-sections")
			set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} ${GC_SECTIONS}")
		endif ()

		if (STINGRAY_GENERATE_DEBUG_INFO)
			set (COMMON_SOURCES_FLAGS "${COMMON_SOURCES_FLAGS} -ggdb")
			set (GENERATED_SOURCES_FLAGS "${GENERATED_SOURCES_FLAGS} -ggdb1")
		else (STINGRAY_GENERATE_DEBUG_INFO)
			set (COMMON_FLAGS "${COMMON_FLAGS} -s")
		endif (STINGRAY_GENERATE_DEBUG_INFO)

		if (HAVE_SORT_COMMON)
			set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} -Wl,--sort-common")
		endif (HAVE_SORT_COMMON)
		if (HAVE_WHOPR AND ALLOW_LTO)
			set(COMMON_FLAGS "-fwhopr ${COMMON_FLAGS}")
			set(COMMON_LINK_OPTS "-fwhopr ${COMMON_LINK_OPTS}")
		elseif (HAVE_FLTO AND ALLOW_LTO)
			set(COMMON_FLAGS "-flto ${COMMON_FLAGS}")
			set(COMMON_LINK_OPTS "-flto ${COMMON_LINK_OPTS}")
		endif (HAVE_WHOPR AND ALLOW_LTO)
		if (HAVE_WHOLE_PROGRAM)
			set(COMMON_LINK_OPTS "-fwhole-program ${COMMON_LINK_OPTS}")
		endif (HAVE_WHOLE_PROGRAM)
	endif (CMAKE_BUILD_TYPE MATCHES Release)

	if (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
		set (COMMON_SOURCES_FLAGS "${COMMON_SOURCES_FLAGS} -ggdb")
		set (GENERATED_SOURCES_FLAGS "${GENERATED_SOURCES_FLAGS} -ggdb1")

		set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} -Wl,--as-needed")
		if (HAVE_WHOLE_PROGRAM)
			set(COMMON_LINK_OPTS "-fwhole-program ${COMMON_LINK_OPTS}")
		endif (HAVE_WHOLE_PROGRAM)
	endif (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

	if (CMAKE_BUILD_TYPE MATCHES MinSizeRel)
		set (COMMON_FLAGS "${COMMON_FLAGS} -fdata-sections -ffunction-sections")
		if (STINGRAY_GENERATE_DEBUG_INFO)
			set (COMMON_SOURCES_FLAGS "${COMMON_SOURCES_FLAGS} -ggdb")
			set (GENERATED_SOURCES_FLAGS "${GENERATED_SOURCES_FLAGS} -ggdb1")
		else (STINGRAY_GENERATE_DEBUG_INFO)
			set (COMMON_FLAGS "${COMMON_FLAGS} -s")
		endif (STINGRAY_GENERATE_DEBUG_INFO)

		set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} -Wl,--as-needed -Wl,-O1 -Wl,--relax ${GC_SECTIONS} -Wl,--no-export-dynamic")
		if (HAVE_SORT_COMMON)
			set (COMMON_LINK_OPTS "${COMMON_LINK_OPTS} -Wl,--sort-common")
		endif (HAVE_SORT_COMMON)
		if (HAVE_WHOPR AND ALLOW_LTO)
			set(COMMON_FLAGS "-fwhopr ${COMMON_FLAGS}")
			set(COMMON_LINK_OPTS "-fwhopr ${COMMON_LINK_OPTS}")
		elseif (HAVE_FLTO AND ALLOW_LTO)
			set(COMMON_FLAGS "-flto ${COMMON_FLAGS}")
			set(COMMON_LINK_OPTS "-flto ${COMMON_LINK_OPTS}")
		endif (HAVE_WHOPR AND ALLOW_LTO)
		if (HAVE_WHOLE_PROGRAM)
			set(COMMON_LINK_OPTS "-fwhole-program ${COMMON_LINK_OPTS}")
		endif (HAVE_WHOLE_PROGRAM)
	endif (CMAKE_BUILD_TYPE MATCHES MinSizeRel)

	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
	# FIXME removed besause breaks ar
	#set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ${COMMON_LINK_OPTS}")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${COMMON_LINK_OPTS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_LINK_OPTS}")
endif ((CMAKE_C_COMPILER_ID MATCHES "GNU") OR (CMAKE_C_COMPILER_ID MATCHES "Clang"))

check_cxx_source_compiles("
static __thread int i = 0;
#ifdef __mips__
__attribute__((nomips16))
#endif
int main() { return i; }" HAVE_THREAD_KEYWORD)

if (HAVE_THREAD_KEYWORD)
	list(APPEND STINGRAYKIT_DEFINITIONS -DSTINGRAYKIT_HAS_THREAD_KEYWORD=1)
endif (HAVE_THREAD_KEYWORD)

CHECK_C_SOURCE_COMPILES("#include <time.h>
	int main(int argc, char *argv[]) { clockid_t x; return 0; }" HAVE_CLOCKID_T)

if (USE_BACKTRACE)
	find_package(libunwind)
	if (LIBUNWIND_FOUND)
		set(COMMON_FLAGS "${COMMON_FLAGS} -fno-omit-frame-pointer -funwind-tables")
		list(APPEND STINGRAYKIT_LIBS ${libunwind_LIBRARIES})
		list(APPEND STINGRAYKIT_DEFINITIONS -DUSE_LIBUNWIND_BACKTRACE=1)
		list(APPEND stingraykit_SRC stingraykit/diagnostics/LibunwindBacktrace.cpp)
	else()
		if (PLATFORM_GCC)
			CHECK_C_SOURCE_COMPILES("#include <unwind.h>
				_Unwind_Reason_Code UnwindFunc(struct _Unwind_Context *ctx, void *bt) { return _URC_NO_REASON; }
				int main() { _Unwind_Backtrace(&UnwindFunc, (void*)0); return 0; }" HAVE_GCC_UNWIND)
			if (HAVE_GCC_UNWIND)
				message(STATUS "Using GCC backtrace")
				list(APPEND STINGRAYKIT_DEFINITIONS -DUSE_GCC_BACKTRACE=1)
				list(APPEND stingraykit_SRC
					stingraykit/diagnostics/gcc/Backtrace.cpp
				)
			endif (HAVE_GCC_UNWIND)
		endif (PLATFORM_GCC)

		if (NOT DEFINED HAVE_GCC_UNWIND)
			check_function_exists("backtrace" HAVE_BACKTRACE)

			if (HAVE_BACKTRACE)
				list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_BACKTRACE=1)
				list(APPEND stingraykit_SRC
					stingraykit/thread/posix/Backtrace.cpp
				)
			endif (HAVE_BACKTRACE)
		endif (NOT DEFINED HAVE_GCC_UNWIND)

		if (USE_BFD_BACKTRACE)
			check_include_file(bfd.h HAVE_BFD_H)
			if (HAVE_BFD_H)
				list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_BFD_BACKTRACE=1)
			endif (HAVE_BFD_H)
			list(APPEND STINGRAYKIT_LIBS bfd)
		endif (USE_BFD_BACKTRACE)
	endif()
endif (USE_BACKTRACE)

CHECK_C_SOURCE_COMPILES("#include <pthread.h>
	int main() { pthread_setname_np((pthread_t)0, (const char*)0); return 0; }" HAVE_PTHREAD_SETNAME)

if (HAVE_PTHREAD_SETNAME)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_PTHREAD_SETNAME)
endif (HAVE_PTHREAD_SETNAME)

CHECK_C_SOURCE_COMPILES("#include <pthread.h>
	int main() { pthread_yield(); return 0; }" HAVE_PTHREAD_YIELD)

if (HAVE_PTHREAD_YIELD)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_PTHREAD_YIELD)
endif (HAVE_PTHREAD_YIELD)

CHECK_CXX_SOURCE_COMPILES("#include <cxxabi.h>
	int main() { try {} catch (abi::__forced_unwind&) { throw; } return 0; }" HAVE_ABI_FORCE_UNWIND)
if (HAVE_ABI_FORCE_UNWIND)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_ABI_FORCE_UNWIND=1)
endif (HAVE_ABI_FORCE_UNWIND)

CHECK_C_SOURCE_COMPILES("int main(int argc, char *argv[]) { int x; return __atomic_load_n(&x, __ATOMIC_SEQ_CST); return x; }" HAVE_ATOMIC_BUILTINS)
if (HAVE_ATOMIC_BUILTINS)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_ATOMIC_BUILTINS=1)
endif (HAVE_ATOMIC_BUILTINS)

CHECK_C_SOURCE_COMPILES("int main(int argc, char *argv[]) { int x; __sync_add_and_fetch(&x, 1); return x; }" HAVE_SYNC_AAF)
if (HAVE_SYNC_AAF)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_SYNC_AAF=1)
endif (HAVE_SYNC_AAF)

CHECK_CXX_SOURCE_COMPILES("#include <bits/atomicity.h>
	int main(int argc, char *argv[]) { _Atomic_word x; __gnu_cxx::__exchange_and_add(&x, 1); return x; }" HAVE_SYNC_EAA)
if (HAVE_SYNC_EAA)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_SYNC_EAA=1)
endif (HAVE_SYNC_EAA)

CHECK_CXX_SOURCE_COMPILES("#include <ext/atomicity.h>
	int main(int argc, char *argv[]) { _Atomic_word x; __gnu_cxx::__exchange_and_add(&x, 1); return x; }" HAVE_SYNC_EAA_EXT)
if (HAVE_SYNC_EAA_EXT)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_SYNC_EAA_EXT=1)
endif (HAVE_SYNC_EAA_EXT)

CHECK_INCLUDE_FILE(atomic.h HAVE_ATOMIC_H)
if (HAVE_ATOMIC_H)
	list(APPEND STINGRAYKIT_DEFINITIONS -DHAVE_ATOMIC_H=1)
endif (HAVE_ATOMIC_H)

CHECK_INCLUDE_FILE("valgrind/helgrind.h" HAVE_HELGRIND_H)
if (PLATFORM_LINUX_EMU OR PLATFORM_EMU_TS)
	if (HAVE_HELGRIND_H)
		list(APPEND STINGRAYKIT_DEFINITIONS -DSTINGRAY_USE_HELGRIND_ANNOTATIONS=1)
	endif ()
endif (PLATFORM_LINUX_EMU OR PLATFORM_EMU_TS)

list(APPEND STINGRAYKIT_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stingraykit-doxygen)

if (PLATFORM_POSIX)
	message(STATUS "Using posix platform")
	list(APPEND STINGRAYKIT_DEFINITIONS -DPLATFORM_POSIX=1 -DHAVE_POSIX_THREADS=1 -DPOSIX_HAVE_MONOTONIC_TIMER=1 -D_REENTRANT=1)
	list(APPEND STINGRAYKIT_LIBS pthread rt)
else (PLATFORM_POSIX)
	message("Not using posix stuff")
endif (PLATFORM_POSIX)

set_source_files_properties(${stingraykit_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_SOURCES_FLAGS} ${STINGRAY_NO_ASM_CODE_FLAGS} -DSTINGRAY_LIB_SOURCE")

add_library(stingraykit STATIC ${stingraykit_SRC})

add_custom_command(OUTPUT ${CMAKE_INSTALL_PREFIX}/system/ COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_INSTALL_PREFIX}/system/)

set(STINGRAYKIT_DEFINITIONS ${STINGRAYKIT_DEFINITIONS} PARENT_SCOPE)
set(STINGRAYKIT_INCLUDE_DIRECTORIES ${STINGRAYKIT_INCLUDE_DIRECTORIES} PARENT_SCOPE)
set(STINGRAYKIT_LIBS ${STINGRAYKIT_LIBS} PARENT_SCOPE)

add_definitions(${STINGRAYKIT_DEFINITIONS})
include_directories(${STINGRAYKIT_INCLUDE_DIRECTORIES})

add_custom_target(stingraykit-doxygen doxygen ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/doxy.cfg 2> /dev/null)

set(STINGRAYKIT_LIBS_STR "")
foreach(_LIB ${STINGRAYKIT_LIBS})
	set(STINGRAYKIT_LIBS_STR "${STINGRAYKIT_LIBS_STR} -l${_LIB}")
endforeach()

set(STINGRAYKIT_DEFINITIONS_STR "")
foreach(_DEF ${STINGRAYKIT_DEFINITIONS})
	set(STINGRAYKIT_DEFINITIONS_STR "${STINGRAYKIT_DEFINITIONS_STR} ${_DEF}")
endforeach()

set(STINGRAYKIT_INCLUDE_DIRECTORIES_STR "")
foreach(_DIR ${STINGRAYKIT_INCLUDE_DIRECTORIES})
	set(STINGRAYKIT_INCLUDE_DIRECTORIES_STR "${STINGRAYKIT_INCLUDE_DIRECTORIES_STR} -I${_DIR}")
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/stingraykit.pc.in" "${LIBRARY_OUTPUT_PATH}/stingraykit.pc")
