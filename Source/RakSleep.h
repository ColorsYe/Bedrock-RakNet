/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 跨平台的线程睡眠函数 */
#pragma once
#include "Export.h"

/* 使当前线程休眠指定的毫秒数 */
void RAK_DLL_EXPORT RakSleep(unsigned int ms);
