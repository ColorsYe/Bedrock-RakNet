/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 格式化字符串工具 */


#pragma once
#include "Export.h"

extern "C" {
char * FormatString(const char *format, ...);
}
/* 线程安全版本 */
extern "C" {
char * FormatStringTS(char *output, const char *format, ...);
}
