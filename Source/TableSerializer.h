/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  本源代码使用 BSD 风格许可证授权，
 *  许可证文件位于源码树根目录的 LICENSE 文件中。
 *  同目录下的 PATENTS 文件中还包含额外的专利授权。
 *
 */

/* 数据表序列化/反序列化工具类 */
#pragma once
#include "RakMemoryOverride.h"
#include "DS_Table.h"
#include "Export.h"

namespace RakNet
{
	class BitStream;
}

namespace RakNet
{

class RAK_DLL_EXPORT TableSerializer
{
public:
	/* 将整张表序列化到比特流 */
	static void SerializeTable(DataStructures::Table *in, RakNet::BitStream *out);
	/* 从原始字节数据反序列化表 */
	static bool DeserializeTable(unsigned char *serializedTable, unsigned int dataLength, DataStructures::Table *out);
	/* 从比特流反序列化表 */
	static bool DeserializeTable(RakNet::BitStream *in, DataStructures::Table *out);
	/* 序列化列定义 */
	static void SerializeColumns(DataStructures::Table *in, RakNet::BitStream *out);
	/* 序列化列定义（跳过指定列索引） */
	static void SerializeColumns(DataStructures::Table *in, RakNet::BitStream *out, DataStructures::List<int> &skipColumnIndices);
	/* 反序列化列定义 */
	static bool DeserializeColumns(RakNet::BitStream *in, DataStructures::Table *out);	
	/* 序列化单行数据 */
	static void SerializeRow(DataStructures::Table::Row *in, unsigned keyIn, const DataStructures::List<DataStructures::Table::ColumnDescriptor> &columns, RakNet::BitStream *out);
	/* 序列化单行数据（跳过指定列索引） */
	static void SerializeRow(DataStructures::Table::Row *in, unsigned keyIn, const DataStructures::List<DataStructures::Table::ColumnDescriptor> &columns, RakNet::BitStream *out, DataStructures::List<int> &skipColumnIndices);
	/* 反序列化单行数据 */
	static bool DeserializeRow(RakNet::BitStream *in, DataStructures::Table *out);
	/* 序列化单个单元格 */
	static void SerializeCell(RakNet::BitStream *out, DataStructures::Table::Cell *cell, DataStructures::Table::ColumnType columnType);
	/* 反序列化单个单元格 */
	static bool DeserializeCell(RakNet::BitStream *in, DataStructures::Table::Cell *cell, DataStructures::Table::ColumnType columnType);
	/* 序列化过滤查询条件 */
	static void SerializeFilterQuery(RakNet::BitStream *in, DataStructures::Table::FilterQuery *query);
	/* 注意：此函数会分配 query->cell->c 的内存！ */
	static bool DeserializeFilterQuery(RakNet::BitStream *out, DataStructures::Table::FilterQuery *query);
	/* 序列化过滤查询条件列表 */
	static void SerializeFilterQueryList(RakNet::BitStream *in, DataStructures::Table::FilterQuery *query, unsigned int numQueries, unsigned int maxQueries);
	/* 注意：此函数会分配 queries、cells 和 query->cell->c 的内存！使用 DeallocateQueryList 释放。 */
	static bool DeserializeFilterQueryList(RakNet::BitStream *out, DataStructures::Table::FilterQuery **query, unsigned int *numQueries, unsigned int maxQueries, int allocateExtraQueries=0);
	/* 释放查询列表的内存 */
	static void DeallocateQueryList(DataStructures::Table::FilterQuery *query, unsigned int numQueries);
};

} /* RakNet 命名空间 */
