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
#include <cstdio>
#include "NativeTypes.h"

/* 来自 http://www.azillionmonkeys.com/qed/hash.html */
/* 主要代码作者为 Paul Hsieh */
/* 此处仅添加了一些便捷函数 */
/* 另见 http://burtleburtle.net/bob/hash/doobs.html，该文表明此算法比该页面上的算法快 20%，但碰撞率更高 */

/* 对数据块计算超快哈希值 */
uint32_t SuperFastHash (const char * data, int length);
/* 增量式超快哈希计算 */
uint32_t SuperFastHashIncremental (const char * data, int len, unsigned int lastHash );
/* 对文件内容计算超快哈希值 */
uint32_t SuperFastHashFile (const char * filename);
/* 对文件指针计算超快哈希值 */
uint32_t SuperFastHashFilePtr (FILE *fp);
