/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

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
	static void SerializeTable(DataStructures::Table *in, RakNet::BitStream *out);
	static bool DeserializeTable(unsigned char *serializedTable, unsigned int dataLength, DataStructures::Table *out);
	static bool DeserializeTable(RakNet::BitStream *in, DataStructures::Table *out);
	static void SerializeColumns(DataStructures::Table *in, RakNet::BitStream *out);
	static void SerializeColumns(DataStructures::Table *in, RakNet::BitStream *out, DataStructures::List<int> &skipColumnIndices);
	static bool DeserializeColumns(RakNet::BitStream *in, DataStructures::Table *out);	
	static void SerializeRow(DataStructures::Table::Row *in, unsigned keyIn, const DataStructures::List<DataStructures::Table::ColumnDescriptor> &columns, RakNet::BitStream *out);
	static void SerializeRow(DataStructures::Table::Row *in, unsigned keyIn, const DataStructures::List<DataStructures::Table::ColumnDescriptor> &columns, RakNet::BitStream *out, DataStructures::List<int> &skipColumnIndices);
	static bool DeserializeRow(RakNet::BitStream *in, DataStructures::Table *out);
	static void SerializeCell(RakNet::BitStream *out, DataStructures::Table::Cell *cell, DataStructures::Table::ColumnType columnType);
	static bool DeserializeCell(RakNet::BitStream *in, DataStructures::Table::Cell *cell, DataStructures::Table::ColumnType columnType);
	static void SerializeFilterQuery(RakNet::BitStream *in, DataStructures::Table::FilterQuery *query);
	// Note that this allocates query->cell->c!
	static bool DeserializeFilterQuery(RakNet::BitStream *out, DataStructures::Table::FilterQuery *query);
	static void SerializeFilterQueryList(RakNet::BitStream *in, DataStructures::Table::FilterQuery *query, unsigned int numQueries, unsigned int maxQueries);
	// Note that this allocates queries, cells, and query->cell->c!. Use DeallocateQueryList to free.
	static bool DeserializeFilterQueryList(RakNet::BitStream *out, DataStructures::Table::FilterQuery **query, unsigned int *numQueries, unsigned int maxQueries, int allocateExtraQueries=0);
	static void DeallocateQueryList(DataStructures::Table::FilterQuery *query, unsigned int numQueries);
};

} // namespace RakNet
