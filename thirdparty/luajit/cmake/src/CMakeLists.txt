cmake_minimum_required(VERSION 3.0)

set(LJ_CROSSCOMPILING OFF)
if (NOT ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "${CMAKE_SYSTEM_NAME}"))
  set(LJ_CROSSCOMPILING ON)
endif()

set(LUAJIT_DISABLE_GC64 OFF CACHE BOOL "Disable GC64 mode for x64")
set(LUA_MULTILIB "lib" CACHE PATH "The name of lib directory." )
set(CMAKE_CROSSCOMPILING ${LJ_CROSSCOMPILING} CACHE BOOL "Cross-compiling")
set(LUAJIT_DISABLE_FFI OFF CACHE BOOL "Permanently disable the FFI extension")
set(LUAJIT_DISABLE_JIT OFF CACHE BOOL "Disable the JIT compiler")
set(LUAJIT_NO_UNWIND OFF CACHE BOOL "Disable the UNWIND")
set(LUAJIT_NUMMODE 0 CACHE STRING
"Specify the number mode to use. Possible values:
  0 - Default mode
  1 - Single number mode
  2 - Dual number mode
")
set(LUAJIT_USE_GDBJIT OFF CACHE BOOL "Enable the GDB JIT API")
set(LUAJIT_FORCE_UTF8_FOPEN OFF CACHE BOOL "Enforce UTF8 paths charset in fopen()/freopen() calls, even on Windows")

set(MINILUA_EXE minilua)
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
  set(MINILUA_EXE minilua.exe)
endif()
set(MINILUA_PATH ${CMAKE_CURRENT_BINARY_DIR}/minilua/${MINILUA_EXE})

# Build the minilua for host platform
if (NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(host/cmake/minilua)
  set(MINILUA_PATH $<TARGET_FILE:minilua>)
else()
  make_directory(${CMAKE_CURRENT_BINARY_DIR}/minilua)

  add_custom_command(OUTPUT ${MINILUA_PATH}
    COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/host/cmake/minilua
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/minilua
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/minilua)

  add_custom_target(minilua ALL
    DEPENDS ${MINILUA_PATH}
  )
endif()

include(TestBigEndian)
test_big_endian(LJ_BIG_ENDIAN)

include(CheckTypeSize)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-fno-stack-protector NO_STACK_PROTECTOR_FLAG)

include(DetectArchitecture)
detect_architecture(LJ_DETECTED_ARCH)

include(DetectFPUApi)
detect_fpu_mode(LJ_DETECTED_FPU_MODE)
detect_fpu_abi(LJ_DETECTED_FPU_ABI)

find_library(LIBM_LIBRARIES NAMES m)
find_library(LIBDL_LIBRARIES NAMES dl)

set(TARGET_ARCH "")
set(DASM_FLAGS "")

set(LJ_TARGET_ARCH "")
if("${LJ_DETECTED_ARCH}" STREQUAL "x86")
  set(LJ_TARGET_ARCH "x86")
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "x86_64")
  set(LJ_TARGET_ARCH "x64")
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "AArch64")
  set(LJ_TARGET_ARCH "arm64")
  if (LJ_BIG_ENDIAN)
    set(TARGET_ARCH -D__AARCH64EB__=1)
  endif()
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "ARM")
  set(LJ_TARGET_ARCH "arm")
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "Mips64")
  set(LJ_TARGET_ARCH "mips64")
  if (NOT LJ_BIG_ENDIAN)
    set(TARGET_ARCH -D__MIPSEL__=1)
  endif()
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "Mips")
  set(LJ_TARGET_ARCH "mips")
  if (NOT LJ_BIG_ENDIAN)
    set(TARGET_ARCH -D__MIPSEL__=1)
  endif()
elseif ("${LJ_DETECTED_ARCH}" STREQUAL "PowerPC")
  if (LJ_64)
    set(LJ_TARGET_ARCH "ppc64")
  else()
    set(LJ_TARGET_ARCH "ppc")
  endif()
  if (LJ_BIG_ENDIAN)
    set(TARGET_ARCH -DLJ_ARCH_ENDIAN=LUAJIT_BE)
  else()
    set(TARGET_ARCH -DLJ_ARCH_ENDIAN=LUAJIT_LE)
  endif()
else()
  message(FATAL_ERROR "Unsupported target architecture: '${LJ_DETECTED_ARCH}'")
endif()

if("${LJ_DETECTED_FPU_MODE}" STREQUAL "Hard")
  set(LJ_HAS_FPU 1)
  set(DASM_FLAGS ${DASM_FLAGS} -D FPU)
  set(TARGET_ARCH ${TARGET_ARCH} -DDLJ_ARCH_HASFPU=1)
else()
  set(LJ_HAS_FPU 0)
  set(TARGET_ARCH ${TARGET_ARCH} -DDLJ_ARCH_HASFPU=0)
endif()

if("${LJ_DETECTED_FPU_ABI}" STREQUAL "Hard")
  set(LJ_ABI_SOFTFP 0)
  set(DASM_FLAGS ${DASM_FLAGS} -D HFABI)
  set(TARGET_ARCH ${TARGET_ARCH} -DLJ_ABI_SOFTFP=0)
else()
  set(LJ_ABI_SOFTFP 1)
  set(TARGET_ARCH ${TARGET_ARCH} -DLJ_ABI_SOFTFP=1)
endif()

set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_TARGET=LUAJIT_ARCH_${LJ_TARGET_ARCH})

if (WIN32 OR MINGW)
  set(DASM_FLAGS ${DASM_FLAGS} -D WIN)
elseif (IOS)
  set(DASM_FLAGS ${DASM_FLAGS} -D IOS)
  set(DASM_FLAGS ${DASM_FLAGS} -D NO_UNWIND)
  set(TARGET_ARCH ${TARGET_ARCH} -DLJ_NO_UNWIND=1)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_NO_UNWIND)
endif()

set(LJ_DEFINITIONS "")

set(LJ_ENABLE_LARGEFILE 1)
if (ANDROID AND (${CMAKE_SYSTEM_VERSION} LESS 21))
  set(LJ_ENABLE_LARGEFILE 0)
elseif (WIN32 OR MINGW)
  set(LJ_ENABLE_LARGEFILE 0)
endif()

if (LJ_ENABLE_LARGEFILE)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS}
      -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)
endif()

set(LJ_FFI 1)
if (LUAJIT_DISABLE_FFI)
  set(LJ_FFI 0)
endif()

set(LJ_JIT 1)
if (IOS)
  set(LJ_JIT 0)
elseif (LUAJIT_DISABLE_JIT)
  set(LJ_JIT 0)
endif()

if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(LJ_64 1)
endif()

set(LJ_GC64 ${LJ_64})

if (LJ_64 AND LUAJIT_DISABLE_GC64 AND ("${LJ_TARGET_ARCH}" STREQUAL "x64"))
  set(LJ_GC64 0)
endif()

set(LJ_FR2 ${LJ_GC64})

set(LJ_NUMMODE_SINGLE 0) # Single-number mode only.
set(LJ_NUMMODE_SINGLE_DUAL 1) # Default to single-number mode.
set(LJ_NUMMODE_DUAL 2) # Dual-number mode only.
set(LJ_NUMMODE_DUAL_SINGLE 3) # Default to dual-number mode.

set(LJ_ARCH_NUMMODE ${LJ_NUMMODE_DUAL})
if (LJ_HAS_FPU)
  set(LJ_ARCH_NUMMODE ${LJ_NUMMODE_DUAL_SINGLE})
endif()

if (("${LJ_TARGET_ARCH}" STREQUAL "x86") OR
    ("${LJ_TARGET_ARCH}" STREQUAL "x64"))
  set(LJ_ARCH_NUMMODE ${LJ_NUMMODE_SINGLE_DUAL})
endif()

if (("${LJ_TARGET_ARCH}" STREQUAL "arm") OR
    ("${LJ_TARGET_ARCH}" STREQUAL "arm64") OR
    ("${LJ_TARGET_ARCH}" STREQUAL "mips") OR
    ("${LJ_TARGET_ARCH}" STREQUAL "mips64"))
  set(LJ_ARCH_NUMMODE ${LJ_NUMMODE_DUAL})
endif()

# Enable or disable the dual-number mode for the VM.
if (((LJ_ARCH_NUMMODE EQUAL LJ_NUMMODE_SINGLE) AND (LUAJIT_NUMMODE EQUAL 2)) OR
    ((LJ_ARCH_NUMMODE EQUAL LJ_NUMMODE_DUAL) AND (LUAJIT_NUMMODE EQUAL 1)))
  message (FATAL_ERROR "No support for this number mode on this architecture")
endif()
if ((LJ_ARCH_NUMMODE EQUAL LJ_NUMMODE_DUAL) OR
    ((LJ_ARCH_NUMMODE EQUAL LJ_NUMMODE_DUAL_SINGLE) AND NOT (LUAJIT_NUMMODE EQUAL 1)) OR
    ((LJ_ARCH_NUMMODE EQUAL LJ_NUMMODE_SINGLE_DUAL) AND (LUAJIT_NUMMODE EQUAL 2)))
  set(LJ_DUALNUM 1)
else()
  set(LJ_DUALNUM 0)
endif()

set(BUILDVM_ARCH_H ${CMAKE_CURRENT_BINARY_DIR}/buildvm_arch.h)
set(DASM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../dynasm/dynasm.lua)

if (NOT LJ_BIG_ENDIAN)
  set(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_LE)
else()
  set(DASM_FLAGS ${DASM_FLAGS} -D ENDIAN_BE)
endif()

if (LJ_64)
  set(DASM_FLAGS ${DASM_FLAGS} -D P64)
endif()

if (LJ_FFI)
  set(DASM_FLAGS ${DASM_FLAGS} -D FFI)
endif()

if (LJ_JIT)
  set(DASM_FLAGS ${DASM_FLAGS} -D JIT)
endif()

if (LJ_DUALNUM)
  set(DASM_FLAGS ${DASM_FLAGS} -D DUALNUM)
endif()

set(DASM_ARCH ${LJ_TARGET_ARCH})

if ("${LJ_TARGET_ARCH}" STREQUAL "x64")
  if (NOT LJ_FR2)
    set(DASM_ARCH "x86")
  endif()
endif()

set(DASM_FLAGS ${DASM_FLAGS} -D VER=)

set(TARGET_OS_FLAGS "")
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_LINUX)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_WINDOWS)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_OSX -DTARGET_OS_IPHONE=1)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_OSX -DTARGET_OS_IPHONE=0)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_LINUX)
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Haiku")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_POSIX)
elseif ("${CMAKE_SYSTEM_NAME}" MATCHES "(Open|Free|Net)BSD")
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_BSD)
else()
  set(TARGET_OS_FLAGS ${TARGET_OS_FLAGS} -DLUAJIT_OS=LUAJIT_OS_OTHER)
endif()

if (LUAJIT_DISABLE_GC64)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_DISABLE_GC64)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_DISABLE_GC64)
endif()

set(TARGET_ARCH ${TARGET_ARCH} ${TARGET_OS_FLAGS})
set(LJ_DEFINITIONS ${LJ_DEFINITIONS} ${TARGET_OS_FLAGS})

if (LUAJIT_DISABLE_FFI)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_DISABLE_FFI)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_DISABLE_FFI)
endif()
if (LUAJIT_DISABLE_JIT)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_DISABLE_JIT)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_DISABLE_JIT)
endif()
if(LUAJIT_USE_GDBJIT)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_USE_GDBJIT)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_USE_GDBJIT)
endif()
if(LUAJIT_FORCE_UTF8_FOPEN)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_FORCE_UTF8_FOPEN)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_FORCE_UTF8_FOPEN)
endif()

if(NOT LUAJIT_NO_UNWIND AND NOT IOS)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_UNWIND_EXTERNAL)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_UNWIND_EXTERNAL)
endif()

if (("${LUAJIT_NUMMODE}" STREQUAL "1") OR
    ("${LUAJIT_NUMMODE}" STREQUAL "2"))
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_NUMMODE=${LUAJIT_NUMMODE})
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_NUMMODE=${LUAJIT_NUMMODE})
endif()

if (LUAJIT_ENABLE_GDBJIT)
  set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUAJIT_ENABLE_GDBJIT)
  set(TARGET_ARCH ${TARGET_ARCH} -DLUAJIT_ENABLE_GDBJIT)
endif()

set(VM_DASC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/vm_${DASM_ARCH}.dasc)

add_custom_command(OUTPUT ${BUILDVM_ARCH_H}
  COMMAND ${MINILUA_PATH} ${DASM_PATH} ${DASM_FLAGS}
          -o ${BUILDVM_ARCH_H} ${VM_DASC_PATH}
  DEPENDS minilua)
add_custom_target(buildvm_arch_h ALL
  DEPENDS ${BUILDVM_ARCH_H}
)

# Build the buildvm for host platform
set(BUILDVM_COMPILER_FLAGS "${TARGET_ARCH}")

set(BUILDVM_COMPILER_FLAGS_PATH
  "${CMAKE_CURRENT_BINARY_DIR}/buildvm_flags.config")
file(WRITE ${BUILDVM_COMPILER_FLAGS_PATH} "${BUILDVM_COMPILER_FLAGS}")

set(BUILDVM_EXE buildvm)
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Windows")
  set(BUILDVM_EXE buildvm.exe)
endif()

if (NOT CMAKE_CROSSCOMPILING)
  add_subdirectory(host/cmake/buildvm)
  set(BUILDVM_PATH $<TARGET_FILE:buildvm>)
  add_dependencies(buildvm buildvm_arch_h)
else()
  set(BUILDVM_PATH ${CMAKE_CURRENT_BINARY_DIR}/buildvm/${BUILDVM_EXE})

  find_program(DEFAULT_HOST_C_COMPILER "gcc")
  find_program(DEFAULT_HOST_CXX_COMPILER "g++")
  find_program(DEFAULT_HOST_CMAKE_COMMAND "cmake")
  mark_as_advanced(DEFAULT_HOST_C_COMPILER DEFAULT_HOST_CXX_COMPILER DEFAULT_HOST_CMAKE_COMMAND)

  set(HOST_CMAKE_COMMAND ${DEFAULT_HOST_CMAKE_COMMAND} CACHE STRING "Bypass the host CMake command")
  set(HOST_C_COMPILER ${DEFAULT_HOST_C_COMPILER} CACHE STRING "Bypass the host C compiler command")
  set(HOST_CXX_COMPILER ${DEFAULT_HOST_CXX_COMPILER} CACHE STRING "Bypass the host C++ compiler command")

  make_directory(${CMAKE_CURRENT_BINARY_DIR}/buildvm)

  add_custom_command(OUTPUT ${BUILDVM_PATH}
    COMMAND ${HOST_CMAKE_COMMAND} -E env CC="${HOST_C_COMPILER}" CXX="${HOST_CXX_COMPILER}"
            ${HOST_CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/host/cmake/buildvm
                                  -DEXTRA_COMPILER_FLAGS_FILE=${BUILDVM_COMPILER_FLAGS_PATH}
    COMMAND ${HOST_CMAKE_COMMAND} -E env CC="${HOST_C_COMPILER}" CXX="${HOST_CXX_COMPILER}"
            ${HOST_CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/buildvm
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/host/cmake/buildvm/CMakeLists.txt
    DEPENDS buildvm_arch_h
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/buildvm)

  add_custom_target(buildvm ALL
    DEPENDS ${BUILDVM_PATH}
  )
endif()

set(LJVM_MODE elfasm)
if (APPLE)
  set(LJVM_MODE machasm)
elseif (WIN32 OR MINGW)
  set(LJVM_MODE peobj)
endif()

set(LJ_VM_NAME lj_vm.S)
if ("${LJVM_MODE}" STREQUAL "peobj")
  set(LJ_VM_NAME lj_vm.obj)
endif()

set(LJ_VM_S_PATH ${CMAKE_CURRENT_BINARY_DIR}/${LJ_VM_NAME})
add_custom_command(OUTPUT ${LJ_VM_S_PATH}
  COMMAND ${BUILDVM_PATH} -m ${LJVM_MODE} -o ${LJ_VM_S_PATH}
  DEPENDS buildvm
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_target(lj_gen_vm_s ALL
  DEPENDS ${LJ_VM_S_PATH}
)

if (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
  set_source_files_properties(${LJ_VM_NAME} PROPERTIES COMPILE_FLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

make_directory(${CMAKE_CURRENT_BINARY_DIR}/jit)
set(LJ_LIBDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/lj_libdef.h)
set(LJ_RECDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/lj_recdef.h)
set(LJ_FFDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/lj_ffdef.h)
set(LJ_BCDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/lj_bcdef.h)
set(LJ_VMDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/jit/vmdef.lua)

set(LJ_LIB_SOURCES
  lib_base.c lib_buffer.c lib_math.c lib_bit.c lib_string.c lib_table.c
  lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c)
add_custom_command(
  OUTPUT ${LJ_LIBDEF_PATH} ${LJ_VMDEF_PATH} ${LJ_RECDEF_PATH} ${LJ_FFDEF_PATH}
  OUTPUT ${LJ_BCDEF_PATH}
  COMMAND ${BUILDVM_PATH} -m libdef -o ${LJ_LIBDEF_PATH} ${LJ_LIB_SOURCES}
  COMMAND ${BUILDVM_PATH} -m recdef -o ${LJ_RECDEF_PATH} ${LJ_LIB_SOURCES}
  COMMAND ${BUILDVM_PATH} -m ffdef -o ${LJ_FFDEF_PATH} ${LJ_LIB_SOURCES}
  COMMAND ${BUILDVM_PATH} -m bcdef -o ${LJ_BCDEF_PATH} ${LJ_LIB_SOURCES}
  COMMAND ${BUILDVM_PATH} -m vmdef -o ${LJ_VMDEF_PATH} ${LJ_LIB_SOURCES}
  DEPENDS buildvm ${LJ_LIB_SOURCE}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_target(lj_gen_headers ALL
  DEPENDS ${LJ_LIBDEF_PATH} ${LJ_RECDEF_PATH} ${LJ_VMDEF_PATH}
  DEPENDS ${LJ_FFDEF_PATH} ${LJ_BCDEF_PATH}
)

set(LJ_FOLDDEF_PATH ${CMAKE_CURRENT_BINARY_DIR}/lj_folddef.h)

set(LJ_FOLDDEF_SOURCE lj_opt_fold.c)
add_custom_command(
  OUTPUT ${LJ_FOLDDEF_PATH}
  COMMAND ${BUILDVM_PATH} -m folddef -o ${LJ_FOLDDEF_PATH} ${LJ_FOLDDEF_SOURCE}
  DEPENDS ${BUILDVM_PATH}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_target(lj_gen_folddef ALL
  DEPENDS ${LJ_FOLDDEF_PATH}
)

set(luajit_sources
    lib_aux.c
    lib_base.c
    lib_bit.c
    lib_buffer.c
    lib_debug.c
    lib_ffi.c
    lib_init.c
    lib_io.c
    lib_jit.c
    lib_math.c
    lib_os.c
    lib_package.c
    lib_string.c
    lib_table.c
    lj_alloc.c
    lj_api.c
    lj_asm.c
    lj_bc.c
    lj_bcread.c
    lj_bcwrite.c
    lj_buf.c
    lj_carith.c
    lj_ccall.c
    lj_ccallback.c
    lj_cconv.c
    lj_cdata.c
    lj_char.c
    lj_clib.c
    lj_cparse.c
    lj_crecord.c
    lj_ctype.c
    lj_debug.c
    lj_dispatch.c
    lj_err.c
    lj_ffrecord.c
    lj_func.c
    lj_gc.c
    lj_ir.c
    lj_lex.c
    lj_lib.c
    lj_load.c
    lj_mcode.c
    lj_meta.c
    lj_obj.c
    lj_opt_dce.c
    lj_opt_fold.c
    lj_opt_loop.c
    lj_opt_mem.c
    lj_opt_narrow.c
    lj_opt_sink.c
    lj_opt_split.c
    lj_parse.c
    lj_prng.c
    lj_profile.c
    lj_record.c
    lj_serialize.c
    lj_snap.c
    lj_state.c
    lj_str.c
    lj_strfmt.c
    lj_strfmt_num.c
    lj_strscan.c
    lj_tab.c
    lj_trace.c
    lj_udata.c
    lj_vmevent.c
    lj_vmmath.c
    ${LJ_VM_NAME})

if(LUAJIT_USE_GDBJIT)
  list(APPEND luajit_sources lj_gdbjit.c)
endif()

if(WIN32 AND LUAJIT_FORCE_UTF8_FOPEN)
  list(APPEND luajit_sources lj_fopen.c)
endif()

# Build the luajit static library
add_library(libluajit ${luajit_sources})
set_target_properties(libluajit PROPERTIES OUTPUT_NAME luajit)
add_dependencies(libluajit
  buildvm_arch_h
  buildvm
  lj_gen_headers
  lj_gen_folddef)
target_include_directories(libluajit PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR})
if (BUILD_SHARED_LIBS)
  if(WIN32 OR MINGW)
    set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUA_BUILD_AS_DLL)
  endif()
  if (APPLE AND NOT LUAJIT_ENABLE_GC64)
    target_link_libraries(libluajit "-image_base 7fff04c4a000")
  endif()
endif()

if(LIBM_LIBRARIES)
  target_link_libraries(libluajit ${LIBM_LIBRARIES})
endif()

if (LIBDL_LIBRARIES)
  target_link_libraries(libluajit ${LIBDL_LIBRARIES})
endif()

set(LJ_DEFINITIONS ${LJ_DEFINITIONS} -DLUA_MULTILIB="${LUA_MULTILIB}")
target_compile_definitions(libluajit PRIVATE ${LJ_DEFINITIONS})

if ("${LJ_TARGET_ARCH}" STREQUAL "x86")
  if (CMAKE_COMPILER_IS_CLANGXX OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(libluajit PRIVATE
      -march=i686 -msse -msse2 -mfpmath=sse)
  endif()
endif()

set(LJ_COMPILE_OPTIONS -U_FORTIFY_SOURCE)
if (NO_STACK_PROTECTOR_FLAG)
  set(LJ_COMPILE_OPTIONS ${LJ_COMPILE_OPTIONS} -fno-stack-protector)
endif()
if (IOS AND ("${LJ_TARGET_ARCH}" STREQUAL "arm64"))
  set(LJ_COMPILE_OPTIONS ${LJ_COMPILE_OPTIONS} -fno-omit-frame-pointer)
endif()

target_compile_options(libluajit PRIVATE ${LJ_COMPILE_OPTIONS})

# Build the luajit binary
add_executable(luajit luajit.c)
target_link_libraries(luajit libluajit)
if (APPLE AND LJ_64 AND NOT LUAJIT_ENABLE_GC64)
  target_link_libraries(luajit "-pagezero_size 10000" "-image_base 100000000")
endif()
if ("${CMAKE_SYSTEM_NAME}" MATCHES "(Open|Free|Net)BSD")
  target_link_libraries(luajit c++abi pthread)
endif()
target_compile_definitions(luajit PRIVATE ${LJ_DEFINITIONS})

set(luajit_headers lauxlib.h lua.h luaconf.h luajit.h lualib.h)
install(FILES ${luajit_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/luajit)
install(TARGETS libluajit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(TARGETS luajit DESTINATION "${CMAKE_INSTALL_BINDIR}")
