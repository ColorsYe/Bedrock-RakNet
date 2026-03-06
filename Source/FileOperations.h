/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 文件操作工具函数 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FileOperations==1

#pragma once
#include "Export.h"

/* 递归创建目录并写入文件 */
bool RAK_DLL_EXPORT WriteFileWithDirectories( const char *path, char *data, unsigned dataLength );
/* 判断字符是否为路径分隔符 */
bool RAK_DLL_EXPORT IsSlash(unsigned char c);
/* 在路径末尾添加分隔符 */
void RAK_DLL_EXPORT AddSlash( char *input );
/* 如果路径包含空格则添加引号 */
void RAK_DLL_EXPORT QuoteIfSpaces(char *str);
/* 检查目录是否存在 */
bool RAK_DLL_EXPORT DirectoryExists(const char *directory);
/* 获取文件长度 */
unsigned int RAK_DLL_EXPORT GetFileLength(const char *path);

#endif
