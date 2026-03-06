/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 可变参数 SQL 解析器 */
#pragma once
#include "DS_List.h"

#include <cstdarg>

namespace VariadicSQLParser
{
	/* 索引和类型映射结构 */
	struct IndexAndType
	{
		unsigned int strIndex;         /* 字符串索引 */
		unsigned int typeMappingIndex; /* 类型映射索引 */
	};
	/* 获取指定索引处的类型映射 */
	const char* GetTypeMappingAtIndex(int i);
	/* 获取格式字符串中的类型映射索引列表 */
	void GetTypeMappingIndices( const char *format, DataStructures::List<IndexAndType> &indices );
	/* 给定带有可变参数的 SQL 字符串，分配 argumentBinary 和 argumentLengths，并以二进制格式保存参数 */
	/* 最后 2 个参数为输出参数 */
	void ExtractArguments( va_list argptr, const DataStructures::List<IndexAndType> &indices, char ***argumentBinary, int **argumentLengths );
	/* 释放由 ExtractArguments 分配的内存 */
	void FreeArguments(const DataStructures::List<IndexAndType> &indices, char **argumentBinary, int *argumentLengths);
}
