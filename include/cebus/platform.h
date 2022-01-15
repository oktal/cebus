#pragma once

#if defined(_MSC_VER)
    #define CEBUS_COMPILER_MSVC 1

    #if defined(_M_X64)
        #define CEBUS_CPU_X64 1
    #endif

#elif defined(__GNUC__)
    #define CEBUS_COMPILER_GCC 1

    #if defined(__llvm__)
        #define CEBUS_COMPILER_LLVM 1
    #endif

    #if defined(__x86_64__)
        #define CEBUS_CPU_X64 1
    #endif

    #if defined(__aarch64__)
        #define CEBUS_CPU_ARM 1
    #endif
#else
    #error "Unsupported compiler!"
#endif


#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   #define CEBUS_PLATFORM_WINDOWS 1
#elif __APPLE__
    #define CEBUS_PLATFORM_APPLE 1
#elif __linux__
    #define CEBUS_PLATFORM_LINUX 1
#else
#   error "Unsupported platform!"
#endif
