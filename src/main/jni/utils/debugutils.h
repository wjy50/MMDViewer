//
// Created by wjy50 on 18-3-17.
//

#ifndef MMDVIEWER_DEBUGUTILS_H
#define MMDVIEWER_DEBUGUTILS_H

#define MMDVIEWER_DEBUG_ENABLED

#ifdef MMDVIEWER_DEBUG_ENABLED

#include <android/log.h>

#define LOG_SYSTEM_OUT(format,...) __android_log_print(ANDROID_LOG_DEBUG,"System.out",format,__VA_ARGS__)

#else

#define LOG_SYSTEM_OUT(format,...)

#endif

#endif //MMDVIEWER_DEBUGUTILS_H
