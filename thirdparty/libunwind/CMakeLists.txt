cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# The unw_* api only, see README.md. Not for android, which has no use for it, and not for windows, which walks
# with dbghelp.
if(OE_BUILD_PLATFORM STREQUAL "android" OR OE_BUILD_PLATFORM STREQUAL "windows")
    return()
endif()

# The assembler has to target what the c compiler targets. The 32-bit x86 build gets there with -m32 in the c
# flags alone, and the register save/restore assembly is x86-64 code when that doesn't reach it.
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${CMAKE_C_FLAGS}")
enable_language(ASM)

add_library(libunwind STATIC
        src/libunwind.cpp
        src/UnwindRegistersSave.S
        src/UnwindRegistersRestore.S)
target_include_directories(libunwind PUBLIC include)
target_include_directories(libunwind PRIVATE src)
target_compile_definitions(libunwind PRIVATE _LIBUNWIND_IS_NATIVE_ONLY)
target_compile_options(libunwind PRIVATE -fno-exceptions -fno-rtti -funwind-tables -Wno-suggest-override) # Upstream code, not ours to annotate.
target_link_libraries(libunwind PUBLIC ${CMAKE_DL_LIBS})
set_target_properties(libunwind PROPERTIES OUTPUT_NAME unwind_api) # Not "unwind", that's the system one.
add_library(libunwind::libunwind ALIAS libunwind)
