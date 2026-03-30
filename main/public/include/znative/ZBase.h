//
// Created by LiangKeJin on 2024/11/15.
//

#pragma once

#include <iostream>
#include <memory>

#ifdef _WIN32
   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
         // iOS Simulator
        #ifndef __IOS_SIMULATOR__
        #define __IOS_SIMULATOR__
        #endif
    #elif TARGET_OS_IPHONE
        // iOS device
        #ifndef __IOS__
        #define __IOS__
        #endif
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
        #ifndef __MACOS__
        #define __MACOS__
        #endif
    #else
        #error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // android
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#endif

// 为了防止缩进，IDE 无法修改namespace的缩进
#define NAMESPACE_BEG(ns) namespace ns {
#define NAMESPACE_END }

// 可以使用 -DDEFAULT_NAMESPACE=xxx 来修改默认的命名空间
#ifndef DEFAULT_NAMESPACE
#define DEFAULT_NAMESPACE znative
#endif

#ifndef NAMESPACE_DEFAULT
#define NAMESPACE_DEFAULT NAMESPACE_BEG(DEFAULT_NAMESPACE)
#endif

