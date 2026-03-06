/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
 *  CloudServer.h
 * 存储客户端数据，并允许跨服务器通信以检索此数据
 * 待办
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_CloudServer==1

#pragma once
#include "PluginInterface2.h"
#include "RakMemoryOverride.h"
#include "NativeTypes.h"
#include "RakString.h"
#include "DS_Hash.h"
#include "CloudCommon.h"
#include "DS_OrderedList.h"

/* If the data is smaller than this value, an allocation is avoid. However, this value exists for every row */
static constexpr unsigned int CLOUD_SERVER_DATA_STACK_SIZE = 32;

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;

/*
 * Zero or more instances of CloudServerQueryFilter can be attached to CloudServer to restrict client queries
 * All attached instances of CloudServerQueryFilter on each corresponding operation, from all directly connected clients
 * If any attached instance returns false for a given operation, that operation is silently rejected
 * \ingroup CLOUD_GROUP
 */
class RAK_DLL_EXPORT CloudServerQueryFilter
{
public:
	CloudServerQueryFilter() {}
	virtual ~CloudServerQueryFilter() {}

	/*
	 * 当a local client wants to post data时调用
	 * 返回值: true to allow, false to reject
	 */
	virtual bool OnPostRequest(RakNetGUID clientGuid, SystemAddress clientAddress, CloudKey key, uint32_t dataLength, const char *data)=0;

	/*
	 * 当a local client wants to release data that it has previously uploaded时调用
	 * 返回值: true to allow, false to reject
	 */
	virtual bool OnReleaseRequest(RakNetGUID clientGuid, SystemAddress clientAddress, DataStructures::List<CloudKey> &cloudKeys)=0;

	/*
	 * 当a local client wants to query data时调用
	 * 如果返回false客户端将完全没有响应
	 * 返回值: 允许为true拒绝为false
	 */
	virtual bool OnGetRequest(RakNetGUID clientGuid, SystemAddress clientAddress, CloudQuery &query, DataStructures::List<RakNetGUID> &specificSystems)=0;

	/*
	 * 当a local client wants to stop getting updates for data时调用
	 * If you return false, the client will keep getting updates for that data
	 * 返回值: true to allow, false to reject
	 */
	virtual bool OnUnsubscribeRequest(RakNetGUID clientGuid, SystemAddress clientAddress, DataStructures::List<CloudKey> &cloudKeys, DataStructures::List<RakNetGUID> &specificSystems)=0;
};

/*
 * 存储客户端数据，并允许跨服务器通信以检索此数据
 * \ingroup CLOUD_GROUP
 */
class RAK_DLL_EXPORT CloudServer : public PluginInterface2, CloudAllocator
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(CloudServer)

	CloudServer();
	virtual ~CloudServer();

	/*
	 * Max bytes a client can upload
	 * Data in excess of this value is silently ignored
	 * defaults to 0 (unlimited)
	 * 参数[输入] bytes Max bytes a client can upload. 0 means unlimited.
	 */
	void SetMaxUploadBytesPerClient(uint64_t bytes);

	/*
	 * Max bytes returned by a download. If the number of bytes would exceed this amount, the returned list is truncated
	 * However, if this would result in no rows downloaded, then one row will be returned.
	 * 参数[输入] bytes Max bytes a client can download from a single Get(). 0 means unlimited.
	 */
	void SetMaxBytesPerDownload(uint64_t bytes);

	/*
	 * Add a server, which is assumed to be connected in a fully connected mesh to all other servers and also running the CloudServer plugin
	 * The other system must also call AddServer before getting the subscription data, or it will be rejected.
	 * Sending a message telling the other system to call AddServer(), followed by calling AddServer() locally, would be sufficient for this to work.
	 * 注意: This sends subscription data to the other system, using RELIABLE_ORDERED on channel 0
	 * 参数[输入] systemIdentifier Identifier of the remote system
	 */
	void AddServer(RakNetGUID systemIdentifier);

	/*
	 * Removes a server added through AddServer()
	 * 参数[输入] systemIdentifier Identifier of the remote system
	 */
	void RemoveServer(RakNetGUID systemIdentifier);

	/*
	 * 返回通过 AddServer() 添加的服务器列表
	 * 参数[输出] remoteServers List of servers added
	 */
	void GetRemoteServers(DataStructures::List<RakNetGUID> &remoteServersOut);

	/* Frees all memory. Does not remove query filters */
	void Clear();

	/*
	 * Report the specified SystemAddress to client queries, rather than what RakPeer reads.
	 * 这是useful if you already know your public IP
	 * This only applies to future updates, so call it before updating to apply to all queries
	 * 参数[输入] forcedAddress The systmeAddress to return in queries. Use UNASSIGNED_SYSTEM_ADDRESS (default) to use what RakPeer returns
	 */
	void ForceExternalSystemAddress(SystemAddress forcedAddress);

	/*
	 * Adds a callback called on each query. If all filters returns true for an operation, the operation is allowed.
	 * If the filter was already added, the function silently fails
	 * 参数[输入] filter An externally allocated instance of CloudServerQueryFilter. The instance must remain valid until it is removed with RemoveQueryFilter() or RemoveAllQueryFilters()
	 */
	void AddQueryFilter(CloudServerQueryFilter* filter);

	/*
	 * Removes a callback added with AddQueryFilter()
	 * The instance is not deleted, only unreferenced. It is up to the user to delete the instance, if necessary
	 * 参数[输入] filter An externally allocated instance of CloudServerQueryFilter. The instance must remain valid until it is removed with RemoveQueryFilter() or RemoveAllQueryFilters()
	 */
	void RemoveQueryFilter(CloudServerQueryFilter* filter);

	/*
	 * Removes all instances of CloudServerQueryFilter added with AddQueryFilter().
	 * The instances are not deleted, only unreferenced. It is up to the user to delete the instances, if necessary
	 */
	void RemoveAllQueryFilters();

protected:
	void Update() override;
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	void OnRakPeerShutdown() override;


	virtual void OnPostRequest(Packet *packet);
	virtual void OnReleaseRequest(Packet *packet);
	virtual void OnGetRequest(Packet *packet);
	virtual void OnUnsubscribeRequest(Packet *packet);
	virtual void OnServerToServerGetRequest(Packet *packet);
	virtual void OnServerToServerGetResponse(Packet *packet);

	uint64_t maxUploadBytesPerClient, maxBytesPerDowload;

	/* ---------------------------------------------------------------------------- */
	/* 对于给定的数据键，快速查找已上传的一个或所有系统 */
	/* ---------------------------------------------------------------------------- */
	struct CloudData
	{
		CloudData() {}
		~CloudData() {if (allocatedData) rakFree_Ex(allocatedData, _FILE_AND_LINE_);}
		bool IsUnused(void) const {return isUploaded==false && specificSubscribers.Size()==0;}
		void Clear() {if (dataPtr==allocatedData) rakFree_Ex(allocatedData, _FILE_AND_LINE_); allocatedData=0; dataPtr=0; dataLengthBytes=0; isUploaded=false;}

		unsigned char stackData[CLOUD_SERVER_DATA_STACK_SIZE];
		unsigned char *allocatedData; /* Uses allocatedData instead of stackData if length of data exceeds CLOUD_SERVER_DATA_STACK_SIZE */
		unsigned char *dataPtr; /* Points to either stackData or allocatedData */
		uint32_t dataLengthBytes;
		bool isUploaded;

		/* 持有此数据的服务器的系统地址（客户端已连接）*/
		SystemAddress serverSystemAddress;

		/* 上传此数据的客户端的系统地址 */
		SystemAddress clientSystemAddress;

		/* 持有此数据的服务器的 RakNetGUID（客户端已连接）*/
		RakNetGUID serverGUID;

		/* 上传此数据的客户端的 RakNetGUID */
		RakNetGUID clientGUID;

		/*
		 * 当此特定系统的键数据发生变化时，通知这些订阅者
		 * This list mutually exclusive with CloudDataList::nonSpecificSubscribers
		 */
		DataStructures::OrderedList<RakNetGUID, RakNetGUID> specificSubscribers;
	};
	void WriteCloudQueryRowFromResultList(unsigned int i, DataStructures::List<CloudData*> &cloudDataResultList, DataStructures::List<CloudKey> &cloudKeyResultList, BitStream *bsOut);
	void WriteCloudQueryRowFromResultList(DataStructures::List<CloudData*> &cloudDataResultList, DataStructures::List<CloudKey> &cloudKeyResultList, BitStream *bsOut);

	static int KeyDataPtrComp( const RakNetGUID &key, CloudData* const &data );
	struct CloudDataList
	{
		bool IsUnused(void) const {return keyData.Size()==0 && nonSpecificSubscribers.Size()==0;}
		bool IsNotUploaded(void) const {return uploaderCount==0;}
		bool RemoveSubscriber(RakNetGUID g) {
			bool objectExists;
			unsigned int index;
			index = nonSpecificSubscribers.GetIndexFromKey(g, &objectExists);
			if (objectExists)
			{
				subscriberCount--;
				nonSpecificSubscribers.RemoveAtIndex(index);
				return true;
			}
			return false;
		}

		unsigned int uploaderCount, subscriberCount;
		CloudKey key;

		/* Data uploaded from or subscribed to for various systems */
		DataStructures::OrderedList<RakNetGUID, CloudData*, CloudServer::KeyDataPtrComp> keyData;

		/*
		 * 当任何系统的键数据发生变化时，通知这些订阅者
		 * This list mutually exclusive with CloudData::specificSubscribers
		 */
		DataStructures::OrderedList<RakNetGUID, RakNetGUID> nonSpecificSubscribers;
	};

	static int KeyDataListComp( const CloudKey &key, CloudDataList * const &data );
	DataStructures::OrderedList<CloudKey, CloudDataList*, CloudServer::KeyDataListComp> dataRepository;

	struct KeySubscriberID
	{
		CloudKey key;
		DataStructures::OrderedList<RakNetGUID, RakNetGUID> specificSystemsSubscribedTo;
	};
	static int KeySubscriberIDComp(const CloudKey &key, KeySubscriberID * const &data );

	/* Remote systems */
	struct RemoteCloudClient
	{
		bool IsUnused(void) const {return uploadedKeys.Size()==0 && subscribedKeys.Size()==0;}

		DataStructures::OrderedList<CloudKey,CloudKey,CloudKeyComp> uploadedKeys;
		DataStructures::OrderedList<CloudKey,KeySubscriberID*,CloudServer::KeySubscriberIDComp> subscribedKeys;
		uint64_t uploadedBytes;
	};
	DataStructures::Hash<RakNetGUID, RemoteCloudClient*, 2048, RakNetGUID::ToUint32> remoteSystems;

	/* 对于给定的用户，释放所有已订阅和已上传的键 */
	void ReleaseSystem(RakNetGUID clientAddress );

	/* For a given user, release a set of keys */
	void ReleaseKeys(RakNetGUID clientAddress, DataStructures::List<CloudKey> &keys );

	void NotifyClientSubscribersOfDataChange( CloudData *cloudData, CloudKey &key, DataStructures::OrderedList<RakNetGUID, RakNetGUID> &subscribers, bool wasUpdated );
	void NotifyClientSubscribersOfDataChange( CloudQueryRow *row, DataStructures::OrderedList<RakNetGUID, RakNetGUID> &subscribers, bool wasUpdated );
	void NotifyServerSubscribersOfDataChange( CloudData *cloudData, CloudKey &key, bool wasUpdated );

	struct RemoteServer
	{
		RakNetGUID serverAddress;
		/* This server needs to know about these keys when they are updated or deleted */
		DataStructures::OrderedList<CloudKey,CloudKey,CloudKeyComp> subscribedKeys;
		/* This server has uploaded these keys, and needs to know about Get() requests */
		DataStructures::OrderedList<CloudKey,CloudKey,CloudKeyComp> uploadedKeys;

		/* Just for processing */
		bool workingFlag;

		/* If false, we don't know what keys they have yet, so send everything */
		bool gotSubscribedAndUploadedKeys;
	};

	static int RemoteServerComp(const RakNetGUID &key, RemoteServer* const &data );
	DataStructures::OrderedList<RakNetGUID, RemoteServer*, CloudServer::RemoteServerComp> remoteServers;

	struct BufferedGetResponseFromServer
	{
		void Clear(CloudAllocator *allocator);

		RakNetGUID serverAddress;
		CloudQueryResult queryResult;
		bool gotResult;
	};

	struct CloudQueryWithAddresses
	{
		/* Inputs */
		CloudQuery cloudQuery;
		DataStructures::List<RakNetGUID> specificSystems;

		void Serialize(bool writeToBitstream, BitStream *bitStream);
	};

	static int BufferedGetResponseFromServerComp(const RakNetGUID &key, BufferedGetResponseFromServer* const &data );
	struct GetRequest
	{
		void Clear(CloudAllocator *allocator);
		bool AllRemoteServersHaveResponded(void) const;
		CloudQueryWithAddresses cloudQueryWithAddresses;

		/* 请求开始的时间。如果等待另一个系统的响应时间过长，可以中止剩余系统 */
		RakNet::Time requestStartTime;

		/* Assigned by server that gets the request to identify response. See nextGetRequestId */
		uint32_t requestId;

		RakNetGUID requestingClient;

		DataStructures::OrderedList<RakNetGUID, BufferedGetResponseFromServer*, CloudServer::BufferedGetResponseFromServerComp> remoteServerResponses;
	};
	static int GetRequestComp(const uint32_t &key, GetRequest* const &data );
	DataStructures::OrderedList<uint32_t, GetRequest*, CloudServer::GetRequestComp> getRequests;
	RakNet::Time nextGetRequestsCheck;

	uint32_t nextGetRequestId;

	void ProcessAndTransmitGetRequest(GetRequest *getRequest);

	void ProcessCloudQueryWithAddresses(
		CloudServer::CloudQueryWithAddresses &cloudQueryWithAddresses,
		DataStructures::List<CloudData*> &cloudDataResultList,
		DataStructures::List<CloudKey> &cloudKeyResultList
		);

	void SendUploadedAndSubscribedKeysToServer( RakNetGUID systemAddress );
	void SendUploadedKeyToServers( CloudKey &cloudKey );
	void SendSubscribedKeyToServers( CloudKey &cloudKey );
	void RemoveUploadedKeyFromServers( CloudKey &cloudKey );
	void RemoveSubscribedKeyFromServers( CloudKey &cloudKey );

	void OnSendUploadedAndSubscribedKeysToServer( Packet *packet );
	void OnSendUploadedKeyToServers( Packet *packet );
	void OnSendSubscribedKeyToServers( Packet *packet );
	void OnRemoveUploadedKeyFromServers( Packet *packet );
	void OnRemoveSubscribedKeyFromServers( Packet *packet );
	void OnServerDataChanged( Packet *packet );

	void GetServersWithUploadedKeys(
		DataStructures::List<CloudKey> &keys,
		DataStructures::List<RemoteServer*> &remoteServersWithData
		);

	CloudServer::CloudDataList *GetOrAllocateCloudDataList(CloudKey key, bool *dataRepositoryExists, unsigned int &dataRepositoryIndex);

	void UnsubscribeFromKey(RemoteCloudClient *remoteCloudClient, RakNetGUID remoteCloudClientGuid, unsigned int keySubscriberIndex, CloudKey &cloudKey, DataStructures::List<RakNetGUID> &specificSystems);
	void RemoveSpecificSubscriber(RakNetGUID specificSubscriber, CloudDataList *cloudDataList, RakNetGUID remoteCloudClientGuid);

	DataStructures::List<CloudServerQueryFilter*> queryFilters;

	SystemAddress forceAddress;
};


} /* RakNet 命名空间 */

#endif


/* Key subscription */

/* 一个给定的系统可以订阅一个或多个键。*/
/* The subscription can be further be defined as only subscribing to keys uploaded by or changed by a given system. */
/* 可以订阅尚未上传的键，或上传到其他系统的键 */

/* 操作：*/

/* 1. SubscribeToKey() - 带订阅的 Get() 操作 */
/* A. Add to key subscription list for the client, which contains a keyId / specificUploaderList pair */
/* B. Send to remote servers that for this key, they should send us updates */
/* C. (Done, get operation returns current values) */

/* 2. UpdateData() - Post() 操作 */
/* A. 查找此数据的所有订阅者（用于上传系统）。*/
/* B. Send them the uploaded data */
/* C. 查找所有订阅此数据的服务器 */
/* D. Send them the uploaded data */

/* 3. DeleteData() - Release() 操作 */
/* A. 查找此数据的所有订阅者（用于删除系统）。*/
/* B. Inform them of the deletion */
/* C. 查找所有订阅此数据的服务器 */
/* D. Inform them of the deletion */

/* 4. Unsubscribe()（取消订阅）*/
/* A. 查找此订阅者，并移除其订阅 */
/* B. If no one else is subscribing to this key for any system, notify remote servers we no longer need subscription updates */

/* 内部使用 operations: */

/* 1. 查找是否有已连接的客户端订阅了给定的键 */
/* A. 用于在远程服务器上添加和移除我们对此键的订阅 */

/* 2. 对于给定的键和更新地址，找到所有关心的已连接客户端 */
/* A. First find connected clients that have subscribed to this key, regardless of address */
/* B. Then find connected clients that have subscribed to this key for this particular address */

/* 3. 查找所有已订阅给定键的远程服务器 */
/* A. 这样当键被更新或删除时，我们就知道该发送给谁 */

/* 4. For a given client (such as on disconnect), remove all records of their subscriptions */
