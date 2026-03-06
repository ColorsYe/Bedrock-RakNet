/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/* 将整数转换为指定进制的字符串表示 */
char* Itoa( int value, char* result, int base );

#ifdef __cplusplus
}
#endif
