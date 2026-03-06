/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* RakNet 时间类型定义 */
#pragma once
#include "NativeTypes.h"
#include "RakNetDefines.h"

namespace RakNet {

/* 如果要对 GetTime 使用大类型请定义 __GET_TIME_64BIT（传输时间时会占用更多带宽！） */
/* 当系统运行时间可能超过毫秒计数器的溢出时间（超过一个月）时，您可能需要这样做 */
#if __GET_TIME_64BIT==1
using Time = uint64_t;
using TimeMS = uint32_t;
using TimeUS = uint64_t;
#else
using Time = uint32_t;
using TimeMS = uint32_t;
using TimeUS = uint64_t;
#endif

} /* RakNet 命名空间 */
