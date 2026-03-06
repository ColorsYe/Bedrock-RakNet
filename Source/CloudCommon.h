/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_CloudClient==1 || _RAKNET_SUPPORT_CloudServer==1

#pragma once
#include "RakNetTypes.h"
#include "RakString.h"

namespace RakNet
{

class BitStream;
struct CloudQueryRow;

/*
 * Allocates CloudQueryRow and the row data. Override to use derived classes or different allocators
 * \ingroup CLOUD_GROUP
 */
class RAK_DLL_EXPORT CloudAllocator
{
public:
	CloudAllocator() {}
	virtual ~CloudAllocator() {}

	/* 分配 row */
	virtual CloudQueryRow* AllocateCloudQueryRow();
	/* 释放 row */
	virtual void DeallocateCloudQueryRow(CloudQueryRow *row);
	/* 分配 CloudQueryRow::data */
	virtual unsigned char *AllocateRowData(uint32_t bytesNeededForData);
	/* 释放 CloudQueryRow::data */
	virtual void DeallocateRowData(void *data);
};

/*
 * Serves as a key to identify data uploaded to or queried from the server.
 * \ingroup CLOUD_GROUP
 */
struct RAK_DLL_EXPORT CloudKey
{
	CloudKey() {}
	CloudKey(RakNet::RakString _primaryKey, uint32_t _secondaryKey) : primaryKey(_primaryKey), secondaryKey(_secondaryKey) {}
	~CloudKey() {}

	/*
	 * 标识主键.这是一个主要类别,例如应用程序的名称
	 * 必须非空
	 */
	RakNet::RakString primaryKey;

	/* Identifies the secondary key. This is intended to be a subcategory enumeration, such as PLAYER_LIST or RUNNING_SCORES */
	uint32_t secondaryKey;

	/* 内部使用 */
	void Serialize(bool writeToBitstream, BitStream *bitStream);
};

/* 内部使用 */
int CloudKeyComp(const CloudKey &key, const CloudKey &data);

/*
 * 用于查询云的数据成员
 * \ingroup CLOUD_GROUP
 */
struct RAK_DLL_EXPORT CloudQuery
{
	CloudQuery() {startingRowIndex=0; maxRowsToReturn=0; subscribeToResults=false;}

	/*
	 * List of keys to query. Must be at least of length 1.
	 * This query is run on uploads from all clients, and those that match the combination of primaryKey and secondaryKey are potentially returned
	 * If you pass more than one key at a time, the results are concatenated so if you need to differentiate between queries then send two different queries
	 */
	DataStructures::List<CloudKey> keys;

	/* If limiting the number of rows to return, this is the starting offset into the list. Has no effect unless maxRowsToReturn is > 0 */
	uint32_t startingRowIndex;

	/* 要返回的最大行数实际数字可能仍小于此值传递0表示没有限制 */
	uint32_t maxRowsToReturn;

	/* 如果为真当返回给您的结果发生变化时会自动获取更新取消订阅 CloudMemoryClient::Unsubscribe() */
	bool subscribeToResults;

	/* 内部使用 */
	void Serialize(bool writeToBitstream, BitStream *bitStream);
};

/* \ingroup CLOUD_GROUP */
struct RAK_DLL_EXPORT CloudQueryRow
{
	/* Key used to identify this data */
	CloudKey key;

	/* Data uploaded */
	unsigned char *data;

	/* Length of data uploaded */
	uint32_t length;

	/* 持有此数据的服务器的系统地址（客户端已连接）*/
	SystemAddress serverSystemAddress;

	/* 上传此数据的客户端的系统地址 */
	SystemAddress clientSystemAddress;

	/* 持有此数据的服务器的 RakNetGUID（客户端已连接）*/
	RakNetGUID serverGUID;

	/* 上传此数据的客户端的 RakNetGUID */
	RakNetGUID clientGUID;

	/* 内部使用 */
	void Serialize(bool writeToBitstream, BitStream *bitStream, CloudAllocator *allocator);
};

/* \ingroup CLOUD_GROUP */
struct RAK_DLL_EXPORT CloudQueryResult
{
	/* Query originally passed to Download() */
	CloudQuery cloudQuery;

	/* Results returned from query. If there were multiple keys in CloudQuery::keys then see resultKeyIndices */
	DataStructures::List<CloudQueryRow*> rowsReturned;

	/*
	 * If there were multiple keys in CloudQuery::keys, then each key is processed in order and the result concatenated to rowsReturned
	 * 每个查询的起始索引被写入 resultKeyIndices
	 * 例如，如果 CloudQuery::keys 有 4 个键，返回 3 行、0 行、5 行和 12 行，则
	 * resultKeyIndices would be 0, 3, 3, 8
	 */
	DataStructures::List<uint32_t> resultKeyIndices;

	/* Whatever was passed to CloudClient::Get() as CloudQuery::subscribeToResults */
	bool subscribeToResults;

	/* 内部使用 */
	void Serialize(bool writeToBitstream, BitStream *bitStream, CloudAllocator *allocator);
	/* 内部使用 */
	void SerializeHeader(bool writeToBitstream, BitStream *bitStream);
	/* 内部使用 */
	void SerializeNumRows(bool writeToBitstream, uint32_t &numRows, BitStream *bitStream);
	/* 内部使用 */
	void SerializeCloudQueryRows(bool writeToBitstream, uint32_t &numRows, BitStream *bitStream, CloudAllocator *allocator);
};

} /* Namespace RakNet */

#endif /* __CLOUD_COMMON_H */
