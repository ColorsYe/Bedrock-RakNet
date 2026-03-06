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
 * 
 * 包含 ReplicaManager 类的第三次迭代版本。
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ReplicaManager3==1

#pragma once
#include "RakNetTypes.h"
#include "RakNetTime.h"
#include "BitStream.h"
#include "PacketPriority.h"
#include "PluginInterface2.h"
#include "NetworkIDObject.h"
#include "DS_OrderedList.h"
#include "DS_Queue.h"

/*
 * \defgroup REPLICA_MANAGER_GROUP3 ReplicaManager3
 * Third implementation of object replication
 * * \ingroup PLUGINS_GROUP
 */

namespace RakNet
{
class Connection_RM3;
class Replica3;

/*
 * \ingroup REPLICA_MANAGER_GROUP3
 * 用于multiple worlds. World 0 is created automatically by default
 */
using WorldId = uint8_t;


/*
 * 内部使用
 * \ingroup REPLICA_MANAGER_GROUP3
 */
struct PRO
{
	/* Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultPacketPriority(). */
	PacketPriority priority;

	/* Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultPacketReliability(). */
	PacketReliability reliability;

	/* Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultOrderingChannel(). */
	char orderingChannel;

	/* Passed to RakPeerInterface::Send(). Defaults to 0. */
	uint32_t sendReceipt;

	bool operator==( const PRO& right ) const;
	bool operator!=( const PRO& right ) const;
};


/*
 * System to help automate game object construction, destruction, and serialization
 * ReplicaManager3 tracks your game objects and automates the networking for replicating them across the network<BR>
 * As objects are created, destroyed, or serialized differently, those changes are pushed out to other systems.<BR>
 * To use:<BR>
 * <OL>
 * <LI>Derive from Connection_RM3 and implement Connection_RM3::AllocReplica(). This is a factory function where given a user-supplied identifier for a class (such as name) return an instance of that class. Should be able to return any networked object in your game.
 * <LI>Derive from ReplicaManager3 and implement AllocConnection() and DeallocConnection() to return the class you created in step 1.
 * <LI>Derive your networked game objects from Replica3. All pure virtuals have to be implemented, however defaults are provided for Replica3::QueryConstruction(), Replica3::QueryRemoteConstruction(), and Replica3::QuerySerialization() depending on your network architecture.
 * <LI>When a new game object is created on the local system, pass it to ReplicaManager3::Reference().
 * <LI>When a game object is destroyed on the local system, and you want other systems to know about it, call Replica3::BroadcastDestruction()
 * </OL>
 * <BR>
 * At this point, all new connections will automatically download, get construction messages, get destruction messages, and update serialization automatically.
 * \ingroup REPLICA_MANAGER_GROUP3
 */
class RAK_DLL_EXPORT ReplicaManager3 : public PluginInterface2
{
public:
	ReplicaManager3();
	virtual ~ReplicaManager3();

	/*
	 * Implement to return a game specific derivation of Connection_RM3
	 * The connection object represents a remote system connected to you that is using the ReplicaManager3 system.<BR>
	 * It has functions to perform operations per-connection.<BR>
	 * AllocConnection() and DeallocConnection() are factory functions to create and destroy instances of the connection object.<BR>
	 * It is used if autoCreate is true via SetAutoManageConnections() (true by default). Otherwise, the function is not called, and you will have to call PushConnection() manually<BR>
	 * 注意: If you do not want a new network connection to immediately download game objects, SetAutoManageConnections() and PushConnection() are how you do this.
	 * 另见 SetAutoManageConnections()
	 * 参数[输入] systemAddress Address of the system you are adding
	 * 参数[输入] rakNetGUID GUID of the system you are adding. See Packet::rakNetGUID or RakPeerInterface::GetGUIDFromSystemAddress()
	 * 返回值: The new connection instance.
	 */
	virtual Connection_RM3* AllocConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID) const=0;

	/*
	 * Implement to destroy a class instanced returned by AllocConnection()
	 * Most likely just implement as {delete connection;}<BR>
	 * It is used if autoDestroy is true via SetAutoManageConnections() (true by default). Otherwise, the function is not called and you would then be responsible for deleting your own connection objects.
	 * 参数[输入] connection The pointer instance to delete
	 */
	virtual void DeallocConnection(Connection_RM3 *connection) const=0;

	/*
	 * Enable or disable automatically assigning connections to new instances of Connection_RM3
	 * ReplicaManager3 can automatically create and/or destroy Connection_RM3 as systems connect or disconnect from RakPeerInterface.<BR>
	 * By default this is on, to make the system easier to learn and setup.<BR>
	 * If you don't want all connections to take part in the game, or you want to delay when a connection downloads the game, set autoCreate to false.<BR>
	 * If you want to delay deleting a connection that has dropped, set autoDestroy to false. If you do this, then you must call PopConnection() to remove that connection from being internally tracked. You'll also have to delete the connection instance on your own.<BR>
	 * 参数[输入] autoCreate Automatically call ReplicaManager3::AllocConnection() for each new connection. Defaults to true. Also see AutoCreateConnectionList()
	 * 参数[输入] autoDestroy Automatically call ReplicaManager3::DeallocConnection() for each dropped connection. Defaults to true.
	 */
	void SetAutoManageConnections(bool autoCreate, bool autoDestroy);

	/* 返回值: What was passed to the autoCreate parameter of SetAutoManageConnections() */
	bool GetAutoCreateConnections(void) const;

	/* 返回值: What was passed to the autoDestroy parameter of SetAutoManageConnections() */
	bool GetAutoDestroyConnections(void) const;

	/*
	 * Call AllocConnection() and PushConnection() for each connection in participantList
	 * 参数[输入] participantListIn The list of connections to allocate
	 * 参数[输入] participantListOut The connections allocated, if any
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void AutoCreateConnectionList(
		DataStructures::List<RakNetGUID> &participantListIn,
		DataStructures::List<Connection_RM3*> &participantListOut,
		WorldId worldId=0);

	/*
	 * Track a new Connection_RM3 instance
	 * If autoCreate is false for SetAutoManageConnections(), then you need this function to add new instances of Connection_RM3 yourself.<BR>
	 * You don't need to track this pointer yourself, you can get it with GetConnectionAtIndex(), GetConnectionByGUID(), or GetConnectionBySystemAddress().<BR>
	 * 参数[输入] newConnection The new connection instance to track.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	bool PushConnection(RakNet::Connection_RM3 *newConnection, WorldId worldId=0);

	/*
	 * 停止 tracking a connection
	 * On call, for each replica returned by GetReplicasCreatedByGuid(), QueryActionOnPopConnection() will be called. Depending on the return value, this may delete the corresponding replica.<BR>
	 * If autoDestroy is true in the call to SetAutoManageConnections() (true by default) then this is called automatically when the connection is lost. In that case, the returned connection instance is deleted.<BR>
	 * 参数[输入] guid of the connection to get. Passed to ReplicaManager3::AllocConnection() originally.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	RakNet::Connection_RM3 * PopConnection(RakNetGUID guid, WorldId worldId=0);

	/*
	 * Adds a replicated object to the system.
	 * Anytime you create a new object that derives from Replica3, and you want ReplicaManager3 to use it, pass it to Reference().<BR>
	 * Remote systems already connected will potentially download this object the next time ReplicaManager3::Update() is called, which happens every time you call RakPeerInterface::Receive().<BR>
	 * You can also call ReplicaManager3::Update() manually to send referenced objects right away
	 * 参数[输入] replica3 The object to start tracking
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void Reference(RakNet::Replica3 *replica3, WorldId worldId=0);

	/*
	 * Removes a replicated object from the system.
	 * The object is not deallocated, it is up to the caller to do so.<BR>
	 * This is called automatically from the 析构函数 of Replica3, so you don't need to call it manually unless you want to stop tracking an object before it is destroyed.
	 * 参数[输入] replica3 The object to stop tracking
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void Dereference(RakNet::Replica3 *replica3, WorldId worldId=0);

	/*
	 * Removes multiple replicated objects from the system.
	 * 与 Dereference(), but for a list of objects.<BR> 相同
	 * Useful with the lists returned by GetReplicasCreatedByGuid(), GetReplicasCreatedByMe(), or GetReferencedReplicaList().<BR>
	 * 参数[输入] replicaListIn List of objects
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void DereferenceList(DataStructures::List<Replica3*> &replicaListIn, WorldId worldId=0);

	/*
	 * Returns all objects originally created by a particular system
	 * Originally created is defined as the value of Replica3::creatingSystemGUID, which is automatically assigned in ReplicaManager3::Reference().<BR>
	 * You do not have to be directly connected to that system to get the objects originally created by that system.<BR>
	 * 参数[输入] guid GUID of the system we are referring to. Originally passed as the guid parameter to ReplicaManager3::AllocConnection()
	 * 参数[输出] List of Replica3 instances to be returned
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void GetReplicasCreatedByGuid(RakNetGUID guid, DataStructures::List<Replica3*> &replicaListOut, WorldId worldId=0);

	/*
	 * Returns all objects originally created by your system
	 * Calls GetReplicasCreatedByGuid() for your own system guid.
	 * 参数[输出] List of Replica3 instances to be returned
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void GetReplicasCreatedByMe(DataStructures::List<Replica3*> &replicaListOut, WorldId worldId=0);

	/*
	 * 返回 entire list of Replicas that we know about
	 * This is all Replica3 instances passed to Reference, as well as instances we downloaded and created via Connection_RM3::AllocReference()
	 * 参数[输出] List of Replica3 instances to be returned
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void GetReferencedReplicaList(DataStructures::List<Replica3*> &replicaListOut, WorldId worldId=0);

	/*
	 * 返回 number of replicas known about
	 * 返回 size of the list that would be returned by GetReferencedReplicaList()
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: How many replica objects are in the list of replica objects
	 */
	unsigned GetReplicaCount(WorldId worldId=0) const;

	/*
	 * 返回 a replica by index
	 * Returns one of the items in the list that would be returned by GetReferencedReplicaList()
	 * 参数[输入] index An index, from 0 to GetReplicaCount()-1.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: A Replica3 instance
	 */
	Replica3 *GetReplicaAtIndex(unsigned index, WorldId worldId=0);

	/*
	 * 返回 number of connections
	 * 返回 number of connections added with ReplicaManager3::PushConnection(), minus the number removed with ReplicaManager3::PopConnection()
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: The number of registered connections
	 */
	unsigned int GetConnectionCount(WorldId worldId=0) const;

	/*
	 * Returns a connection pointer previously added with PushConnection()
	 * 参数[输入] index An index, from 0 to GetConnectionCount()-1.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: A Connection_RM3 pointer
	 */
	Connection_RM3* GetConnectionAtIndex(unsigned index, WorldId worldId=0) const;

	/*
	 * Returns a connection pointer previously added with PushConnection()
	 * 参数[输入] sa The system address of the connection to return
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: A Connection_RM3 pointer, or 0 if not found
	 */
	Connection_RM3* GetConnectionBySystemAddress(const SystemAddress &sa, WorldId worldId=0) const;

	/*
	 * Returns a connection pointer previously added with PushConnection.()
	 * 参数[输入] guid The guid of the connection to return
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: A Connection_RM3 pointer, or 0 if not found
	 */
	Connection_RM3* GetConnectionByGUID(RakNetGUID guid, WorldId worldId=0) const;

	/* 参数[输入] Default ordering channel to use for object creation, destruction, and serializations */
	void SetDefaultOrderingChannel(char def);

	/* 参数[输入] Default packet priority to use for object creation, destruction, and serializations */
	void SetDefaultPacketPriority(PacketPriority def);

	/* 参数[输入] Default packet reliability to use for object creation, destruction, and serializations */
	void SetDefaultPacketReliability(PacketReliability def);

	/*
	 * Every intervalMS milliseconds, Connection_RM3::OnAutoserializeInterval() will be called.<BR>
	 * Defaults to 30.<BR>
	 * Pass with <0 to disable. Pass 0 to Serialize() every time RakPeer::Recieve() is called<BR>
	 * If you want to control the update interval with more granularity, use the return values from Replica3::Serialize().<BR>
	 * 参数[输入] intervalMS How frequently to autoserialize all objects. This controls the maximum number of game object updates 每秒.
	 */
	void SetAutoSerializeInterval(RakNet::Time intervalMS);

	/*
	 * 返回 connections that we think have an instance of the specified Replica3 instance
	 * This can be wrong, for example if that system locally deleted the outside the scope of ReplicaManager3, if QueryRemoteConstruction() returned false, or if DeserializeConstruction() returned false.
	 * 参数[输入] replica The replica to check against.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 参数[输出] connectionsThatHaveConstructedThisReplica Populated with connection instances that we believe have replica allocated
	 */
	void GetConnectionsThatHaveReplicaConstructed(Replica3 *replica, DataStructures::List<Connection_RM3*> &connectionsThatHaveConstructedThisReplica, WorldId worldId=0);

	/*
	 * Returns if GetDownloadWasCompleted() returns true for all connections
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 * 返回值: True when all downloads have been completed
	 */
	bool GetAllConnectionDownloadsCompleted(WorldId worldId=0) const;

	/*
	 * ReplicaManager3 can support multiple worlds, where each world has a separate NetworkIDManager, list of connections, replicas, etc
	 * A world with id 0 is created automatically. If you want multiple worlds, use this function, and ReplicaManager3::SetNetworkIDManager() to have a different NetworkIDManager instance per world
	 * 参数[输入] worldId A unique identifier for this world. User-defined
	 */
	void AddWorld(WorldId worldId);

	/*
	 * Deallocate a world added with AddWorld, or the default world with id 0
	 * Deallocating a world will also stop tracking and updating all connections and replicas associated with that world.
	 * 参数[输入] worldId A worldId value previously added with AddWorld()
	 */
	void RemoveWorld(WorldId worldId);

	/*
	 * 获取 one of the WorldId values added with AddWorld()
	 * WorldId 0 is created by default. Worlds will not necessarily be in the order added with AddWorld(). Edit RemoveWorld() changing RemoveAtIndexFast() to RemoveAtIndex() to preserve order.
	 * 参数[输入] index A value between 0 and GetWorldCount()-1
	 * 返回值: One of the WorldId values added with AddWorld()
	 */
	WorldId GetWorldIdAtIndex(unsigned int index);

	/*
	 * 返回 number of world id specifiers in memory, added with AddWorld() and removed with RemoveWorld()
	 * 返回值: The number of worlds added
	 */
	unsigned int GetWorldCount(void) const;

	/*
	 * 设置 networkIDManager instance that this plugin relys upon.<BR>
	 * Uses whatever instance is attached to RakPeerInterface if unset.<BR>
	 * To support multiple worlds, you should set it to a different manager for each instance of the plugin
	 * 参数[输入] _networkIDManager The externally allocated NetworkIDManager instance for this plugin to use.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void SetNetworkIDManager(NetworkIDManager *_networkIDManager, WorldId worldId=0);

	/*
	 * Returns what was passed to SetNetworkIDManager(), or the instance on RakPeerInterface if unset.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	NetworkIDManager *GetNetworkIDManager(WorldId worldId=0) const;

	/*
	 * Send a network command to destroy one or more Replica3 instances
	 * Usually you won't need this, but use Replica3::BroadcastDestruction() instead.
	 * The objects are unaffected locally
	 * 参数[输入] replicaList List of Replica3 objects to tell other systems to destroy.
	 * 参数[输入] exclusionAddress Which system to not send to. UNASSIGNED_SYSTEM_ADDRESS to send to all.
	 * 参数[输入] worldId 用于multiple worlds. World 0 is created automatically by default. See AddWorld()
	 */
	void BroadcastDestructionList(DataStructures::List<Replica3*> &replicaListSource, const SystemAddress &exclusionAddress, WorldId worldId=0);

	/*
	 * 内部使用
	 * Tell other systems that have this replica to destroy this replica.<BR>
	 * You shouldn't need to call this, as it happens in the Replica3 析构函数
	 */
	void BroadcastDestruction(Replica3 *replica, const SystemAddress &exclusionAddress);

	/*
	 * 内部使用
	 * Frees 内部使用 lists.<BR>
	 * 参数[输入] deleteWorlds True to also delete the worlds added with AddWorld()
	 * Externally allocated pointers are not deallocated
	 */
	void Clear(bool deleteWorlds=false);

	/* 内部使用 */
	PRO GetDefaultSendParameters(void) const;

	/* 调用 interfaces, send data */
	void Update() override;

	/* 内部使用 */
	struct RM3World
	{
		RM3World();
		void Clear(ReplicaManager3 *replicaManager3);

		DataStructures::List<Connection_RM3*> connectionList;
		DataStructures::List<Replica3*> userReplicaList;
		WorldId worldId;
		NetworkIDManager *networkIDManager;
	};
protected:
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;
	void OnRakPeerShutdown() override;
	void OnDetach() override;

	PluginReceiveResult OnConstruction(Packet *packet, unsigned char *packetData, int packetDataLength, RakNetGUID senderGuid, unsigned char packetDataOffset, WorldId worldId);
	PluginReceiveResult OnSerialize(Packet *packet, unsigned char *packetData, int packetDataLength, RakNetGUID senderGuid, RakNet::Time timestamp, unsigned char packetDataOffset, WorldId worldId);
	PluginReceiveResult OnDownloadStarted(Packet *packet, unsigned char *packetData, int packetDataLength, RakNetGUID senderGuid, unsigned char packetDataOffset, WorldId worldId);
	PluginReceiveResult OnDownloadComplete(Packet *packet, unsigned char *packetData, int packetDataLength, RakNetGUID senderGuid, unsigned char packetDataOffset, WorldId worldId);

	void DeallocReplicaNoBroadcastDestruction(RakNet::Connection_RM3 *connection, RakNet::Replica3 *replica3);
	RakNet::Connection_RM3 * PopConnection(unsigned int index, WorldId worldId);
	Replica3* GetReplicaByNetworkID(NetworkID networkId, WorldId worldId);
	unsigned int ReferenceInternal(RakNet::Replica3 *replica3, WorldId worldId);

	PRO defaultSendParameters;
	RakNet::Time autoSerializeInterval;
	RakNet::Time lastAutoSerializeOccurance;
	bool autoCreateConnections, autoDestroyConnections;
	Replica3 *currentlyDeallocatingReplica;
	/* 将on the first call设置为ReferenceInternal(), and should never be changed after that */
	/* 用于lookup in Replica3LSRComp. I don't want to rely on GetNetworkID() in case it changes at runtime */
	uint32_t nextReferenceIndex;

	/* For O(1) lookup */
	RM3World *worldsArray[255];
	/* For fast traversal */
	DataStructures::List<RM3World *> worldsList;

	friend class Connection_RM3;
};

static constexpr int RM3_NUM_OUTPUT_BITSTREAM_CHANNELS=16;

/* \ingroup REPLICA_MANAGER_GROUP3 */
struct LastSerializationResultBS
{
	RakNet::BitStream bitStream[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];
	bool indicesToSend[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];
};

/*
 * Represents the serialized data for an object the last time it was sent. Used by Connection_RM3::OnAutoserializeInterval() and Connection_RM3::SendSerializeIfChanged()
 * \ingroup REPLICA_MANAGER_GROUP3
 */
struct LastSerializationResult
{
	LastSerializationResult();
	~LastSerializationResult() noexcept;
	
	/*
	 * The replica instance we serialized
	 * 注意: replica MUST be the first member of this struct because I cast from replica to LastSerializationResult in Update()
	 */
	RakNet::Replica3 *replica;
	/* bool neverSerialize; */
/* bool isConstructed; */
	RakNet::Time whenLastSerialized;

	void AllocBS();
	LastSerializationResultBS* lastSerializationResultBS;
};

/*
 * 传递给 Replica3::Serialize() 的参数
 * \ingroup REPLICA_MANAGER_GROUP3
 */
struct SerializeParameters
{
	/*
	 * Write your output for serialization here
	 * If nothing is written, the serialization will not occur
	 * Write to any or all of the NUM_OUTPUT_BITSTREAM_CHANNELS channels available. Channels can hold independent data
	 */
	RakNet::BitStream outputBitstream[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];

	/*
	 * Last bitstream we sent for this replica to this system.
	 * Read, but DO NOT MODIFY
	 */
	RakNet::BitStream* lastSentBitstream[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];

	/*
	 * 将to non-zero设置为transmit a timestamp with this message
	 * Defaults to 0
	 * Use RakNet::GetTime() for this
	 */
	RakNet::Time messageTimestamp;

	/*
	 * Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultPacketPriority().
	 * Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultPacketReliability().
	 * Passed to RakPeerInterface::Send(). Defaults to ReplicaManager3::SetDefaultOrderingChannel().
	 */
	PRO pro[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];

	/* Passed to RakPeerInterface::Send(). */
	RakNet::Connection_RM3 *destinationConnection;

	/*
	 * For prior serializations this tick, for the same connection, how many bits have we written so far?
	 * Use this to limit how many objects you send to update per-tick if desired
	 */
	BitSize_t bitsWrittenSoFar;

	/*
	 * When this object was last serialized to the connection
	 * 0 means never
	 */
	RakNet::Time whenLastSerialized;

	/*
	 * Current time, in milliseconds.
	 * curTime - whenLastSerialized is how long it has been since this object was last sent
	 */
	RakNet::Time curTime;
};

/* \ingroup REPLICA_MANAGER_GROUP3 */
struct DeserializeParameters
{
	RakNet::BitStream serializationBitstream[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];
	bool bitstreamWrittenTo[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS];
	RakNet::Time timeStamp;
	RakNet::Connection_RM3 *sourceConnection;
};

/* \ingroup REPLICA_MANAGER_GROUP3 */
enum SendSerializeIfChangedResult
{
	SSICR_SENT_DATA,
	SSICR_DID_NOT_SEND_DATA,
	SSICR_NEVER_SERIALIZE,
};

/*
 * Each remote system is represented by Connection_RM3. Used to allocate Replica3 and track which instances have been allocated
 * Important function: AllocReplica() - must be overridden to create an object given an identifier for that object, which you define for all objects in your game
 * \ingroup REPLICA_MANAGER_GROUP3
 */
class RAK_DLL_EXPORT Connection_RM3
{
public:

	Connection_RM3(const SystemAddress &_systemAddress, RakNetGUID _guid);
	virtual ~Connection_RM3();

	/*
	 * Class factory to create a Replica3 instance, given a user-defined identifier
	 * Identifier is returned by Replica3::WriteAllocationID() for what type of class to create.<BR>
	 * This is called when you download a replica from another system.<BR>
	 * See Replica3::Dealloc for the corresponding destruction message.<BR>
	 * Return 0 if unable to create the intended object. Note, in that case the other system will still think we have the object and will try to serialize object updates to us. Generally, you should not send objects the other system cannot create.<BR>
	 * 另见 Replica3::WriteAllocationID().
	 * Sample implementation:<BR>
	 * {RakNet::RakString typeName; allocationIdBitstream->Read(typeName); if (typeName=="Soldier") return new Soldier; return 0;}<BR>
	 * 参数[输入] allocationIdBitstream user-defined bitstream uniquely identifying a game object type
	 * 参数[输入] replicaManager3 Instance of ReplicaManager3 that controls this connection
	 * 返回值: The new replica instance
	 */
	virtual Replica3 *AllocReplica(RakNet::BitStream *allocationIdBitstream, ReplicaManager3 *replicaManager3)=0;

	/*
	 * 获取 list of all replicas that are constructed for this connection
	 * 参数[输出] objectsTheyDoHave Destination list. Returned in sorted ascending order, sorted on the value of the Replica3 pointer.
	 */
	virtual void GetConstructedReplicas(DataStructures::List<Replica3*> &objectsTheyDoHave);

	/*
	 * 如果we think this remote connection has this replica constructed则返回 true
	 * 参数[输入] replica3 Which replica we are querying
	 * 返回值: True if constructed, false othewise
	 */
	bool HasReplicaConstructed(RakNet::Replica3 *replica);

	/*
	 * When a new connection connects, before sending any objects, SerializeOnDownloadStarted() is called
	 * 参数[输出] bitStream Passed to DeserializeOnDownloadStarted()
	 */
	virtual void SerializeOnDownloadStarted(RakNet::BitStream *bitStream) {(void) bitStream;}

	/*
	 * Receives whatever was written in SerializeOnDownloadStarted()
	 * 参数[输入] bitStream Written in SerializeOnDownloadStarted()
	 */
	virtual void DeserializeOnDownloadStarted(RakNet::BitStream *bitStream) {(void) bitStream;}

	/*
	 * When a new connection connects, after constructing and serialization all objects, SerializeOnDownloadComplete() is called
	 * 参数[输出] bitStream Passed to DeserializeOnDownloadComplete()
	 */
	virtual void SerializeOnDownloadComplete(RakNet::BitStream *bitStream) {(void) bitStream;}

	/*
	 * Receives whatever was written in DeserializeOnDownloadComplete()
	 * 参数[输入] bitStream Written in SerializeOnDownloadComplete()
	 */
	virtual void DeserializeOnDownloadComplete(RakNet::BitStream *bitStream) {(void) bitStream;}

	/* 返回值: The system address passed to the 构造函数 of this object */
	SystemAddress GetSystemAddress(void) const {return systemAddress;}

	/* 返回值: Returns the RakNetGUID passed to the 构造函数 of this object */
	RakNetGUID GetRakNetGUID(void) const {return guid;}

	/* 返回值: True if ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE arrived for this connection */
	bool GetDownloadWasCompleted(void) const {return gotDownloadComplete;}

	/* List of enumerations for how to get the list of valid objects for other systems */
	enum ConstructionMode
	{
		/*
		 * For every object that does not exist on the remote system, call Replica3::QueryConstruction() every tick.
		 * Do not call Replica3::QueryDestruction()
		 * Do not call Connection_RM3::QueryReplicaList()
		 */
		QUERY_REPLICA_FOR_CONSTRUCTION,

		/*
		 * For every object that does not exist on the remote system, call Replica3::QueryConstruction() every tick. Based on the call, the object may be sent to the other system.
		 * For every object that does exist on the remote system, call Replica3::QueryDestruction() every tick. Based on the call, the object may be deleted on the other system.
		 * Do not call Connection_RM3::QueryReplicaList()
		 */
		QUERY_REPLICA_FOR_CONSTRUCTION_AND_DESTRUCTION,

		/*
		 * Do not call Replica3::QueryConstruction() or Replica3::QueryDestruction()
		 * Call Connection_RM3::QueryReplicaList() to determine which objects exist on remote systems
		 * This can be faster than QUERY_REPLICA_FOR_CONSTRUCTION and QUERY_REPLICA_FOR_CONSTRUCTION_AND_DESTRUCTION for large worlds
		 * See GridSectorizer.h under /Source for code that can help with this
		 */
		QUERY_CONNECTION_FOR_REPLICA_LIST
	};

	/*
	 * Return whether or not downloads to our system should all be processed the same tick (call to RakPeer::Receive() )
	 * Normally the system will send ID_REPLICA_MANAGER_DOWNLOAD_STARTED, ID_REPLICA_MANAGER_CONSTRUCTION for all downloaded objects,
	 * ID_REPLICA_MANAGER_SERIALIZE for each downloaded object, and lastly ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE.
	 * This enables the application to show a downloading splash screen on ID_REPLICA_MANAGER_DOWNLOAD_STARTED, a progress bar, and to close the splash screen and activate all objects on ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE
	 * However, if the application was not set up for this then it would result in incomplete objects spread out over time, and cause problems
	 * If you return true from QueryGroupDownloadMessages(), then these messages will be returned all in one tick, returned only when the download is complete
	 * 注意: ID_REPLICA_MANAGER_DOWNLOAD_STARTED calls the callback DeserializeOnDownloadStarted()
	 * 注意: ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE calls the callback DeserializeOnDownloadComplete()
	 */
	virtual bool QueryGroupDownloadMessages(void) const {return false;}

	/*
	 * Queries how to get the list of objects that exist on remote systems
	 * The default of calling QueryConstruction for every known object is easy to use, but not efficient, especially for large worlds where many objects are outside of the player's circle of influence.<BR>
	 * QueryDestruction is also not necessarily useful or efficient, as object destruction tends to happen in known cases, and can be accomplished by calling Replica3::BroadcastDestruction()
	 * QueryConstructionMode() allows you to specify more efficient algorithms than the default when overriden.
	 * 返回值: How to get the list of objects that exist on the remote system. You should always return the same value for a given connection
	 */
	virtual ConstructionMode QueryConstructionMode(void) const {return QUERY_REPLICA_FOR_CONSTRUCTION_AND_DESTRUCTION;}

	/*
	 * Callback used when QueryConstructionMode() returns QUERY_CONNECTION_FOR_REPLICA_LIST
	 * This advantage of this callback is if that there are many objects that a particular connection does not have, then we do not have to iterate through those
	 * objects calling QueryConstruction() for each of them.<BR>
	 * <BR>
	 * See GridSectorizer in the Source directory as a method to find all objects within a certain radius in a fast way.<BR>
	 * <BR>
	 * 参数[输出] newReplicasToCreate Anything in this list will be created on the remote system
	 * 参数[输出] existingReplicasToDestroy Anything in this list will be destroyed on the remote system
	 */
	virtual void QueryReplicaList(
		DataStructures::List<Replica3*> &newReplicasToCreate,
		DataStructures::List<Replica3*> &existingReplicasToDestroy) {(void) newReplicasToCreate; (void) existingReplicasToDestroy;}

	/*
	 * Override which replicas to serialize and in what order for a connection for a ReplicaManager3::Update() cycle
	 * By default, Connection_RM3 will iterate through queryToSerializeReplicaList and call QuerySerialization() on each Replica in that list
	 * queryToSerializeReplicaList is populated in the order in which ReplicaManager3::Reference() is called for those objects.
	 * If you write to to replicasToSerialize and return true, you can control in what order and for which replicas to call QuerySerialization()
	 * Example use case:
	 * We have more data to send then the bandwidth supports, so want to prioritize sends. For example enemies shooting are more important than animation effects
	 * When QuerySerializationList(), sort objects by priority, and write the list to replicasToSerialize, optionally skipping objects with a lower serialization frequency
	 * If you hit your bandwidth limit when checking SerializeParameters::bitsWrittenSoFar, you can return RM3SR_DO_NOT_SERIALIZE for all remaining items
	 * 注意: Only replicas written to replicasToSerialize are transmitted. Even if you returned RM3SR_SERIALIZED_ALWAYS a prior ReplicaManager3::Update() cycle, the replica will not be transmitted if it is not in replicasToSerialize
	 * 注意: If you do not know what objects are candidates for serialization, you can use queryToSerializeReplicaList as a source for your filtering or sorting operations
	 * 参数[输入] replicasToSerialize List of replicas to call QuerySerialization() on
	 * 返回值: Return true to use replicasToSerialize (replicasToSerialize may be empty if desired). Otherwise return false.
	 */
	virtual bool QuerySerializationList(DataStructures::List<Replica3*> &replicasToSerialize) {(void) replicasToSerialize; return false;}

	/*
	 * 内部使用 This is used internally - however, you can also call it manually to send a data update for a remote replica.<BR>
	 * Sends over a serialization update for replica.<BR>
	 * NetworkID::GetNetworkID() is written automatically, serializationData is the object data.<BR>
	 * 参数[输入] replica Which replica to serialize
	 * 参数[输入] serializationData Serialized object data
	 * 参数[输入] timestamp 0 means no timestamp. Otherwise message is prepended with ID_TIMESTAMP
	 * 参数[输入] sendParameters Parameters on how to send
	 * 参数[输入] rakPeer Instance of RakPeerInterface to send on
	 * 参数[输入] worldId Which world, see ReplicaManager3::AddWorld()
	 * 参数[输入] curTime The current time
	 */
	virtual SendSerializeIfChangedResult SendSerialize(RakNet::Replica3 *replica, bool indicesToSend[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS], RakNet::BitStream serializationData[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS], RakNet::Time timestamp, PRO sendParameters[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS], RakNet::RakPeerInterface *rakPeer, unsigned char worldId, RakNet::Time curTime);

	/*
	 * 内部使用
	 * Calls Connection_RM3::SendSerialize() if Replica3::Serialize() returns a different result than what is contained in lastSerializationResult.<BR>
	 * 被autoserialization in Connection_RM3::OnAutoserializeInterval()使用
	 * 参数[输入] lsr Item in the queryToSerializeReplicaList
	 * 参数[输入] sp Controlling parameters over the serialization
	 * 参数[输入] rakPeer Instance of RakPeerInterface to send on
	 * 参数[输入] worldId Which world, see ReplicaManager3::AddWorld()
	 * 参数[输入] curTime The current time
	 */
	virtual SendSerializeIfChangedResult SendSerializeIfChanged(LastSerializationResult *lsr, SerializeParameters *sp, RakNet::RakPeerInterface *rakPeer, unsigned char worldId, ReplicaManager3 *replicaManager, RakNet::Time curTime);

	/*
	 * 内部使用
	 * Given a list of objects that were created and destroyed, serialize and send them to another system.
	 * 参数[输入] newObjects Objects to serialize construction
	 * 参数[输入] deletedObjects Objects to serialize destruction
	 * 参数[输入] sendParameters Controlling parameters over the serialization
	 * 参数[输入] rakPeer Instance of RakPeerInterface to send on
	 * 参数[输入] worldId Which world, see ReplicaManager3::AddWorld()
	 * 参数[输入] replicaManager3 ReplicaManager3 instance
	 */
	virtual void SendConstruction(DataStructures::List<Replica3*> &newObjects, DataStructures::List<Replica3*> &deletedObjects, PRO sendParameters, RakNet::RakPeerInterface *rakPeer, unsigned char worldId, ReplicaManager3 *replicaManager3);

	/* 内部使用 */
	void SendValidation(RakNet::RakPeerInterface *rakPeer, WorldId worldId);

	/* 内部使用 */
	void AutoConstructByQuery(ReplicaManager3 *replicaManager3, WorldId worldId);


	/* 内部使用 - does the other system have this connection too? Validated means we can now use it */
	bool isValidated;
	/* 内部使用 - Used to see if we should send download started */
	bool isFirstConstruction;

	static int Replica3LSRComp( Replica3 * const &replica3, LastSerializationResult * const &data );

	/* 内部使用 */
	void ClearDownloadGroup(RakPeerInterface *rakPeerInterface);
protected:

	SystemAddress systemAddress;
	RakNetGUID guid;

	/*
		操作：

		Locally reference a new replica:
		Add to queryToConstructReplicaList for all objects

		Add all objects to queryToConstructReplicaList

		Download:
		Add to constructedReplicaList for connection that send the object to us
		Add to queryToSerializeReplicaList for connection that send the object to us
		Add to queryToConstructReplicaList for all other connections

		Never construct for this connection:
		移除 from queryToConstructReplicaList

		Construct to this connection
		移除 from queryToConstructReplicaList
		Add to constructedReplicaList for this connection
		Add to queryToSerializeReplicaList for this connection

		Serialize:
		Iterate through queryToSerializeReplicaList

		Never serialize for this connection
		移除 from queryToSerializeReplicaList

		Reference (this system has this object already)
		移除 from queryToConstructReplicaList
		Add to constructedReplicaList for this connection
		Add to queryToSerializeReplicaList for this connection

		Downloaded an existing object
		if replica is in queryToConstructReplicaList, OnConstructToThisConnection()
		else ignore

		发送 destruction from query
		移除 from queryToDestructReplicaList
		移除 from queryToSerializeReplicaList
		移除 from constructedReplicaList
		添加 to queryToConstructReplicaList

		Do not query destruction again
		移除 from queryToDestructReplicaList
	*/
	void OnLocalReference(Replica3* replica3, ReplicaManager3 *replicaManager);
	void OnDereference(Replica3* replica3, ReplicaManager3 *replicaManager);
	void OnDownloadFromThisSystem(Replica3* replica3, ReplicaManager3 *replicaManager);
	void OnDownloadFromOtherSystem(Replica3* replica3, ReplicaManager3 *replicaManager);
	void OnNeverConstruct(unsigned int queryToConstructIdx, ReplicaManager3 *replicaManager);
	void OnConstructToThisConnection(unsigned int queryToConstructIdx, ReplicaManager3 *replicaManager);
	void OnConstructToThisConnection(Replica3 *replica, ReplicaManager3 *replicaManager);
	void OnNeverSerialize(LastSerializationResult *lsr, ReplicaManager3 *replicaManager);
	void OnReplicaAlreadyExists(unsigned int queryToConstructIdx, ReplicaManager3 *replicaManager);
	void OnDownloadExisting(Replica3* replica3, ReplicaManager3 *replicaManager);
	void OnSendDestructionFromQuery(unsigned int queryToDestructIdx, ReplicaManager3 *replicaManager);
	void OnDoNotQueryDestruction(unsigned int queryToDestructIdx, ReplicaManager3 *replicaManager);
	void ValidateLists(ReplicaManager3 *replicaManager) const;
	void SendSerializeHeader(RakNet::Replica3 *replica, RakNet::Time timestamp, RakNet::BitStream *bs, WorldId worldId);
	
	/* The list of objects that our local system and this remote system both have */
	/* Either we sent this object to them, or they sent this object to us */
	/* A given Replica can be either in queryToConstructReplicaList or constructedReplicaList but not both at the same time */
	DataStructures::OrderedList<Replica3*, LastSerializationResult*, Connection_RM3::Replica3LSRComp> constructedReplicaList;

	/* Objects that we have, but this system does not, and we will query each tick to see if it should be sent to them */
	/* If we do send it to them, the replica is moved to constructedReplicaList */
	/* A given Replica can be either in queryToConstructReplicaList or constructedReplicaList but not both at the same time */
	DataStructures::List<LastSerializationResult*> queryToConstructReplicaList;

	/* Objects that this system has constructed are added at the same time to queryToSerializeReplicaList */
	/* This list is used to serialize all objects that this system has to this connection */
	DataStructures::List<LastSerializationResult*> queryToSerializeReplicaList;

	/* Objects that are constructed on this system are also queried if they should be destroyed to this system */
	DataStructures::List<LastSerializationResult*> queryToDestructReplicaList;

	/* Working lists */
	DataStructures::List<Replica3*> constructedReplicasCulled, destroyedReplicasCulled;

	/* This is used if QueryGroupDownloadMessages() returns true when ID_REPLICA_MANAGER_DOWNLOAD_STARTED arrives */
	/* Packets will be gathered and not returned until ID_REPLICA_MANAGER_DOWNLOAD_COMPLETE arrives */
	bool groupConstructionAndSerialize;
	DataStructures::Queue<Packet*> downloadGroup;

	/* Stores if we got download complete for this connection */
	bool gotDownloadComplete;

	friend class ReplicaManager3;
private:
	Connection_RM3() {};

	ConstructionMode constructionMode;
};

/*
 * Return codes for Connection_RM3::GetConstructionState() and Replica3::QueryConstruction()
 * Indicates what state the object should be in for the remote system
 * \ingroup REPLICA_MANAGER_GROUP3
 */
enum RM3ConstructionState
{
	/*
	 * This object should exist on the remote system. Send a construction message if necessary
	 * If the NetworkID is already in use, it will not do anything
	 * If it is not in use, it will create the object, and then call DeserializeConstruction
	 */
	RM3CS_SEND_CONSTRUCTION,

	/*
	 * This object should exist on the remote system.
	 * The other system already has the object, and the object will never be deleted.
	 * This is true of objects that are loaded with the level, for example.
	 * Treat it as if it existed, without sending a construction message.
	 * Will call Serialize() and SerializeConstructionExisting() to the object on the remote system
	 */
	RM3CS_ALREADY_EXISTS_REMOTELY,

	/* 与 RM3CS_ALREADY_EXISTS_REMOTELY but does not call SerializeConstructionExisting() 相同 */
	RM3CS_ALREADY_EXISTS_REMOTELY_DO_NOT_CONSTRUCT,

	/*
	 * This object will never be sent to the target system
	 * This object will never be serialized from this system to the target system
	 */
	RM3CS_NEVER_CONSTRUCT,
	
	/* Don't do anything this tick. Will query again next tick */
	RM3CS_NO_ACTION,

	/* 枚举最大值 */
	RM3CS_MAX,
};

/*
 * If this object already exists for this system, should it be removed?
 * \ingroup REPLICA_MANAGER_GROUP3
 */
enum RM3DestructionState
{
	/* This object should not exist on the remote system. Send a destruction message if necessary. */
	RM3DS_SEND_DESTRUCTION,

	/* This object will never be destroyed by a per-tick query. Don't call again */
	RM3DS_DO_NOT_QUERY_DESTRUCTION,

	/* Don't do anything this tick. Will query again next tick */
	RM3DS_NO_ACTION,

	/* 枚举最大值 */
	RM3DS_MAX,
};

/*
 * 构建对象时的返回码
 * \ingroup REPLICA_MANAGER_GROUP3
 */
enum RM3SerializationResult
{
	/*
	 * This object serializes identically no matter who we send to
	 * We also send it to every connection (broadcast).
	 * Efficient for memory, speed, and bandwidth but only if the object is always broadcast identically.
	 */
	RM3SR_BROADCAST_IDENTICALLY,

	/*
	 * 与 RM3SR_BROADCAST_IDENTICALLY, but assume the object needs to be serialized, do not check with a memcmp 相同
	 * Assume the object changed, and serialize it
	 * Use this if you know exactly when your object needs to change. Can be faster than RM3SR_BROADCAST_IDENTICALLY.
	 * An example of this is if every member variable has an accessor, changing a member sets a flag, and you check that flag in Replica3::QuerySerialization()
	 * The opposite of this is RM3SR_DO_NOT_SERIALIZE, in case the object did not change
	 */
	RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION,

	/*
	 * Either this object serializes differently depending on who we send to or we send it to some systems and not others.
	 * Inefficient for memory and speed, but efficient for bandwidth
	 * However, if you don't know what to return, return this
	 */
	RM3SR_SERIALIZED_UNIQUELY,

	/*
	 * Do not compare against last sent value. Just send even if the data is the same as the last tick
	 * If the data is always changing anyway, or you want to send unreliably, this is a good method of serialization
	 * Can send unique data per connection if desired. If same data is sent to all connections, use RM3SR_SERIALIZED_ALWAYS_IDENTICALLY for even better performance
	 * Efficient for memory and speed, but not necessarily bandwidth
	 */
	RM3SR_SERIALIZED_ALWAYS,

	/* 已废弃, use RM3SR_BROADCAST_IDENTICALLY_FORCE_SERIALIZATION */
	RM3SR_SERIALIZED_ALWAYS_IDENTICALLY,

	/* Do not serialize this object this tick, for this connection. Will query again next autoserialize timer */
	RM3SR_DO_NOT_SERIALIZE,

	/*
	 * Never serialize this object for this connection
	 * Useful for objects that are downloaded, and never change again
	 * Efficient
	 */
	RM3SR_NEVER_SERIALIZE_FOR_THIS_CONNECTION,

	/* 枚举最大值 */
	RM3SR_MAX,
};

/*
 * First pass at topology to see if an object should be serialized
 * \ingroup REPLICA_MANAGER_GROUP3
 */
enum RM3QuerySerializationResult
{
	/* Call Serialize() to see if this object should be serializable for this connection */
	RM3QSR_CALL_SERIALIZE,
	/* Do not call Serialize() this tick to see if this object should be serializable for this connection */
	RM3QSR_DO_NOT_CALL_SERIALIZE,
	/* Never call Serialize() for this object and connection. This system will not serialize this object for this topology */
	RM3QSR_NEVER_CALL_SERIALIZE,
	/* 枚举最大值 */
	RM3QSR_MAX,
};

/* \ingroup REPLICA_MANAGER_GROUP3 */
enum RM3ActionOnPopConnection
{
	RM3AOPC_DO_NOTHING,
	RM3AOPC_DELETE_REPLICA,
	RM3AOPC_DELETE_REPLICA_AND_BROADCAST_DESTRUCTION,
	RM3AOPC_MAX,
};

/*
 * \ingroup REPLICA_MANAGER_GROUP3
 * 用于Replica3::QueryConstruction_PeerToPeer() and Replica3::QuerySerialization_PeerToPeer() to describe how the object replicates between hosts
 */
enum Replica3P2PMode
{
	/*
	 * The Replica3 instance is constructed and serialized by one system only.
	 * Example: Your avatar. No other player serializes or can create your avatar.
	 */
	R3P2PM_SINGLE_OWNER,
	/*
	 * The Replica3 instance is constructed and/or serialized by different systems
	 * This system is currently in charge of construction and/or serialization
	 * Example: A pickup. When an avatar holds it, that avatar controls it. When it is on the ground, the host controls it.
	 */
	R3P2PM_MULTI_OWNER_CURRENTLY_AUTHORITATIVE,
	/*
	 * The Replica3 instance is constructed and/or serialized by different systems
	 * Another system is in charge of construction and/or serialization, but this system may be in charge at a later time
	 * Example: A pickup held by another player. That player sends creation of that object to new connections, and serializes it until it is dropped.
	 */
	R3P2PM_MULTI_OWNER_NOT_CURRENTLY_AUTHORITATIVE,
	/*
	 * The Replica3 instance is a static object (already exists on the remote system).
	 * This system is currently in charge of construction and/or serialization
	 */
	R3P2PM_STATIC_OBJECT_CURRENTLY_AUTHORITATIVE,
	/*
	 * The Replica3 instance is a static object (already exists on the remote system).
	 * Another system is in charge of construction and/or serialization, but this system may be in charge at a later time
	 */
	R3P2PM_STATIC_OBJECT_NOT_CURRENTLY_AUTHORITATIVE,

};

/*
 * Base class for your replicated objects for the ReplicaManager3 system.
 * To use, derive your class, or a member of your class, from Replica3.<BR>
 * \ingroup REPLICA_MANAGER_GROUP3
 */
class RAK_DLL_EXPORT Replica3 : public NetworkIDObject
{
public:
	Replica3();

	/*
	 * Before deleting a local instance of Replica3, call Replica3::BroadcastDestruction() for the deletion notification to go out on the network.
	 * It is not necessary to call ReplicaManager3::Dereference(), as this happens automatically in the 析构函数
	 */
	virtual ~Replica3();

	/*
	 * Write a unique identifer that can be read on a remote system to create an object of this same class.
	 * The value written to allocationIdBitstream will be passed to Connection_RM3::AllocReplica().<BR>
	 * Sample implementation:<BR>
	 * {allocationIdBitstream->Write(RakNet::RakString("Soldier");}<BR>
	 * 参数[输出] allocationIdBitstream Bitstream for the user to write to, to identify this class
	 */
	virtual void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const=0;

	/*
	 * Ask if this object, which does not exist on destinationConnection should (now) be sent to that system.
	 * If ReplicaManager3::QueryConstructionMode() returns QUERY_CONNECTION_FOR_REPLICA_LIST or QUERY_REPLICA_FOR_CONSTRUCTION_AND_DESTRUCTION (default),
	 * then QueyrConstruction() is called once per tick from ReplicaManager3::Update() to determine if an object should exist on a given system.<BR>
	 * Based on the return value, a network message may be sent to the other system to create the object.<BR>
	 * If QueryConstructionMode() is overriden to return QUERY_CONNECTION_FOR_REPLICA_LIST, this function is unused.<BR>
	 * 注意: Defaults are provided: QueryConstruction_PeerToPeer(), QueryConstruction_ServerConstruction(), QueryConstruction_ClientConstruction(). Return one of these functions for a working default for the relevant topology.
	 * 参数[输入] destinationConnection Which system we will send to
	 * 参数[输入] replicaManager3 Plugin instance for this Replica3
	 * 返回值: What action to take
	 */
	virtual RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3)=0;

	/*
	 * Ask if this object, which does exist on destinationConnection should be removed from the remote system
	 * If ReplicaManager3::QueryConstructionMode() returns QUERY_REPLICA_FOR_CONSTRUCTION_AND_DESTRUCTION (default),
	 * then QueryDestruction() is called once per tick from ReplicaManager3::Update() to determine if an object that exists on a remote system should be destroyed for a given system.<BR>
	 * Based on the return value, a network message may be sent to the other system to destroy the object.<BR>
	 * Note that you can also destroy objects with BroadcastDestruction(), so this function is not useful unless you plan to delete objects for only a particular connection.<BR>
	 * If QueryConstructionMode() is overriden to return QUERY_CONNECTION_FOR_REPLICA_LIST, this function is unused.<BR>
	 * 参数[输入] destinationConnection Which system we will send to
	 * 参数[输入] replicaManager3 Plugin instance for this Replica3
	 * 返回值: What action to take. Only RM3CS_SEND_DESTRUCTION does anything at this time.
	 */
	virtual RM3DestructionState QueryDestruction(RakNet::Connection_RM3 *destinationConnection, ReplicaManager3 *replicaManager3) {(void) destinationConnection; (void) replicaManager3; return RM3DS_DO_NOT_QUERY_DESTRUCTION;}

	/*
	 * We're about to call DeserializeConstruction() on this Replica3. If QueryRemoteConstruction() returns false, this object is deleted instead.
	 * By default, QueryRemoteConstruction_ServerConstruction() does not allow clients to create objects. The client will get Replica3::DeserializeConstructionRequestRejected().<BR>
	 * If you want the client to be able to potentially create objects for client/server, override accordingly.<BR>
	 * Other variants of QueryRemoteConstruction_* just return true.
	 * 注意: Defaults are provided: QueryRemoteConstruction_PeerToPeer(), QueryRemoteConstruction_ServerConstruction(), QueryRemoteConstruction_ClientConstruction(). Return one of these functions for a working default for the relevant topology.
	 * 参数[输入] sourceConnection Which system sent us the object creation request message.
	 * 返回值: True to allow the object to pass onto DeserializeConstruction() (where it may also be rejected), false to immediately reject the remote construction request
	 */
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection)=0;

	/*
	 * We got a message from a connection to destroy this replica
	 * Return true to automatically relay the destruction message to all our other connections
	 * For a client in client/server, it does not matter what this funtion returns
	 * For a server in client/server, this should normally return true
	 * For a peer in peer to peer, you can normally return false since the original destroying peer would have told all other peers about the destruction
	 * If a system gets a destruction command for an object that was already destroyed, the destruction message is ignored
	 */
	virtual bool QueryRelayDestruction(Connection_RM3 *sourceConnection) const {(void) sourceConnection; return true;}

	/*
	 * Write data to be sent only when the object is constructed on a remote system.
	 * SerializeConstruction is used to write out data that you need to create this object in the context of your game, such as health, score, name. Use it for data you only need to send when the object is created.<BR>
	 * After SerializeConstruction() is called, Serialize() will be called immediately thereafter. However, they are sent in different messages, so Serialize() may arrive a later frame than SerializeConstruction()
	 * For that reason, the object should be valid after a call to DeserializeConstruction() for at least a short time.<BR>
	 * 注意: The object's NetworkID and allocation id are handled by the system automatically, you do not need to write these values to constructionBitstream
	 * 参数[输出] constructionBitstream Destination bitstream to write your data to
	 * 参数[输入] destinationConnection System that will receive this network message.
	 */
	virtual void SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection)=0;

	/*
	 * Read data written by Replica3::SerializeConstruction()
	 * Reads whatever data was written to constructionBitstream in Replica3::SerializeConstruction()
	 * 参数[输出] constructionBitstream Bitstream written to in Replica3::SerializeConstruction()
	 * 参数[输入] sourceConnection System that sent us this network message.
	 * 返回值: true to accept construction of the object. false to reject, in which case the object will be deleted via Replica3::DeallocReplica()
	 */
	virtual bool DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection)=0;

	/*
	 * 与 SerializeConstruction(), but for an object that already exists on the remote system 相同
	 * Used if you return RM3CS_ALREADY_EXISTS_REMOTELY from QueryConstruction
	 */
	virtual void SerializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {(void) constructionBitstream; (void) destinationConnection;};

	/*
	 * 与 DeserializeConstruction(), but for an object that already exists on the remote system 相同
	 * Used if you return RM3CS_ALREADY_EXISTS_REMOTELY from QueryConstruction
	 */
	virtual void DeserializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {(void) constructionBitstream; (void) sourceConnection;};

	/*
	 * Write extra data to send with the object deletion event, if desired
	 * Replica3::SerializeDestruction() will be called to write any object destruction specific data you want to send with this event.
	 * destructionBitstream can be read in DeserializeDestruction()
	 * 参数[输出] destructionBitstream Bitstream for you to write to
	 * 参数[输入] destinationConnection System that will receive this network message.
	 */
	virtual void SerializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *destinationConnection)=0;

	/*
	 * Read data written by Replica3::SerializeDestruction()
	 * Return true to delete the object. BroadcastDestruction() will be called automatically, followed by ReplicaManager3::Dereference.<BR>
	 * Return false to not delete it. If you delete it at a later point, you are responsible for calling BroadcastDestruction() yourself.
	 */
	virtual bool DeserializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *sourceConnection)=0;

	/*
	 * The system is asking what to do with this replica when the connection is dropped
	 * Return QueryActionOnPopConnection_Client, QueryActionOnPopConnection_Server, or QueryActionOnPopConnection_PeerToPeer
	 */
	virtual RakNet::RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const=0;

	/* Notification called for each of our replicas when a connection is popped */
	virtual void OnPoppedConnection(RakNet::Connection_RM3 *droppedConnection) {(void) droppedConnection;}

	/*
	 * Override with {delete this;}
	 * * <OL>
	 * <LI>Got a remote message to delete this object which passed DeserializeDestruction(), OR
	 * <LI>ReplicaManager3::SetAutoManageConnections() was called autoDestroy true (which is the default setting), and a remote system that owns this object disconnected) OR
	 * <\OL>
	 * <BR>
	 * Override with {delete this;} to actually delete the object (and any other processing you wish).<BR>
	 * If you don't want to delete the object, just do nothing, however, the system will not know this. You may wish to call Dereference() if the object should no longer be networked, but remain in memory. You are responsible for deleting it yoruself later.<BR>
	 * destructionBitstream may be 0 if the object was deleted locally
	 */
	virtual void DeallocReplica(RakNet::Connection_RM3 *sourceConnection)=0;

	/*
	 * Implement with QuerySerialization_ClientSerializable(), QuerySerialization_ServerSerializable(), or QuerySerialization_PeerToPeer()
	 * QuerySerialization() is a first pass query to check if a given object should serializable to a given system. The intent is that the user implements with one of the defaults for client, server, or peer to peer.<BR>
	 * Without this function, a careless implementation would serialize an object anytime it changed to all systems. This would give you feedback loops as the sender gets the same message back from the recipient it just sent to.<BR>
	 * If more than one system can serialize the same object then you will need to override to return true, and control the serialization result from Replica3::Serialize(). Be careful not to send back the same data to the system that just sent to you!
	 * 返回值: True to allow calling Replica3::Serialize() for this connection, false to not call.
	 */
	virtual RakNet::RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection)=0;

	/*
	 * Called for each replica owned by the user, once per Serialization tick, before Serialize() is called.
	 * If you want to do some kind of operation on the Replica objects that you own, just before Serialization(), then overload this function
	 */
	virtual void OnUserReplicaPreSerializeTick() {}

	/*
	 * Serialize our class to a bitstream
	 * User should implement this function to write the contents of this class to SerializationParamters::serializationBitstream.<BR>
	 * If data only needs to be written once, you can write it to SerializeConstruction() instead for efficiency.<BR>
	 * Transmitted over the network if it changed from the last time we called Serialize().<BR>
	 * Called every time the time interval to ReplicaManager3::SetAutoSerializeInterval() elapses and ReplicaManager3::Update is subsequently called.
	 * \param[in/out] serializeParameters Parameters controlling the serialization, including destination bitstream to write to
	 * 返回值: Whether to serialize, and if so, how to optimize the results
	 */
	virtual RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters)=0;

	/*
	 * 当the class is actually transmitted via Serialize()时调用
	 * Use to track how much bandwidth this class it taking
	 */
	virtual void OnSerializeTransmission(RakNet::BitStream *bitStream, RakNet::Connection_RM3 *destinationConnection, BitSize_t bitsPerChannel[RM3_NUM_OUTPUT_BITSTREAM_CHANNELS], RakNet::Time curTime) {(void) bitStream; (void) destinationConnection; (void) bitsPerChannel; (void) curTime;}

	/*
	 * Read what was written in Serialize()
	 * Reads the contents of the class from SerializationParamters::serializationBitstream.<BR>
	 * Called whenever Serialize() is called with different data from the last send.
	 * 参数[输入] serializationBitstream Bitstream passed to Serialize()
	 * 参数[输入] timeStamp 0 if unused, else contains the time the message originated on the remote system
	 * 参数[输入] sourceConnection Which system sent to us
	 */
	virtual void Deserialize(RakNet::DeserializeParameters *deserializeParameters)=0;

	/*
	 * Called after SerializeConstruction completes for all objects in a given update tick.<BR>
	 * Writes to PostDeserializeConstruction(), which is called after all objects are created for a given Construction tick().
	 * Override to send data to PostDeserializeConstruction(), such as the NetworkID of other objects to resolve pointers to
	 */
	virtual void PostSerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {(void) constructionBitstream; (void) destinationConnection;}

	/*
	 * Called after DeserializeConstruction completes for all objects in a given update tick.<BR>
	 * This is used to resolve dependency chains, where two objects would refer to each other in DeserializeConstruction, yet one had not been constructed yet
	 * In PostDeserializeConstruction(), you know that all objects have already been created, so can resolve NetworkIDs to pointers safely.
	 * You can also use it to trigger some sort of event when you know the object has completed deserialization.
	 * 参数[输入] constructionBitstream BitStream written in PostSerializeConstruction()
	 * 参数[输入] sourceConnection System that sent us this network message.
	 */
	virtual void PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {(void) constructionBitstream; (void) sourceConnection;}

	/* 与 PostSerializeConstruction(), but for objects that returned RM3CS_ALREADY_EXISTS_REMOTELY from QueryConstruction 相同 */
	virtual void PostSerializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {(void) constructionBitstream; (void) destinationConnection;}

	/* 与 PostDeserializeConstruction(), but for objects that returned RM3CS_ALREADY_EXISTS_REMOTELY from QueryConstruction 相同 */
	virtual void PostDeserializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {(void) constructionBitstream; (void) sourceConnection;}

	/*
	 * Called after DeserializeDestruction completes for the object successfully, but obviously before the object is deleted.<BR>
	 * Override to trigger some sort of event when you know the object has completed destruction.
	 * 参数[输入] sourceConnection System that sent us this network message.
	 */
	virtual void PreDestruction(RakNet::Connection_RM3 *sourceConnection) {(void) sourceConnection;}

	/*
	 * QueryConstruction() 的默认调用。
	 * Both the client and the server is allowed to create this object. The network topology is client/server
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual RM3ConstructionState QueryConstruction_ClientConstruction(RakNet::Connection_RM3 *destinationConnection, bool isThisTheServer);
	
	/*
	 * Default call for QueryRemoteConstruction().
	 * Both the client and the server is allowed to create this object. The network topology is client/server
	 * The code means on the client or the server, allow creation of Replica3 instances
	 * 参数[输入] sourceConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual bool QueryRemoteConstruction_ClientConstruction(RakNet::Connection_RM3 *sourceConnection, bool isThisTheServer);

	/*
	 * QueryConstruction() 的默认调用。
	 * Only the server is allowed to create this object. The network topology is client/server
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual RM3ConstructionState QueryConstruction_ServerConstruction(RakNet::Connection_RM3 *destinationConnection, bool isThisTheServer);

	/*
	 * Default call for QueryRemoteConstruction(). Allow the server to create this object, but not the client.
	 * Only the server is allowed to create this object. The network topology is client/server
	 * The code means if this is the server, and I got a command to create a Replica3 to ignore it. If this is the client, to allow it.
	 * 参数[输入] sourceConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual bool QueryRemoteConstruction_ServerConstruction(RakNet::Connection_RM3 *sourceConnection, bool isThisTheServer);

	/*
	 * QueryConstruction() 的默认调用。
	 * All clients are allowed to create all objects. The object is not relayed when remotely created
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] p2pMode If controlled only by this system ever, pass R3P2PM_SINGLE_OWNER. Otherwise pass R3P2PM_MULTI_OWNER_CURRENTLY_AUTHORITATIVE or R3P2PM_MULTI_OWNER_NOT_CURRENTLY_AUTHORITATIVE
	 */
	virtual RM3ConstructionState QueryConstruction_PeerToPeer(RakNet::Connection_RM3 *destinationConnection, Replica3P2PMode p2pMode=R3P2PM_SINGLE_OWNER);
	/*
	 * Default call for QueryRemoteConstruction().
	 * All clients are allowed to create all objects. The object is not relayed when remotely created
	 * 参数[输入] sourceConnection destinationConnection parameter passed to QueryConstruction()
	 */
	virtual bool QueryRemoteConstruction_PeerToPeer(RakNet::Connection_RM3 *sourceConnection);

	/*
	 * QuerySerialization() 的默认调用。
	 * Use if the values you are serializing are generated by the client that owns the object. The serialization will be relayed through the server to the other clients.
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual RakNet::RM3QuerySerializationResult QuerySerialization_ClientSerializable(RakNet::Connection_RM3 *destinationConnection, bool isThisTheServer);
	/*
	 * QuerySerialization() 的默认调用。
	 * Use if the values you are serializing are generated only by the server. The serialization will be sent to all clients, but the clients will not send back to the server.
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] isThisTheServer 如果this system is the server, false if not则为 true
	 */
	virtual RakNet::RM3QuerySerializationResult QuerySerialization_ServerSerializable(RakNet::Connection_RM3 *destinationConnection, bool isThisTheServer);
	/*
	 * QuerySerialization() 的默认调用。
	 * Use if the values you are serializing are on a peer to peer network. The peer that owns the object will send to all. Remote peers will not send.
	 * 参数[输入] destinationConnection destinationConnection parameter passed to QueryConstruction()
	 * 参数[输入] p2pMode If controlled only by this system ever, pass R3P2PM_SINGLE_OWNER. Otherwise pass R3P2PM_MULTI_OWNER_CURRENTLY_AUTHORITATIVE or R3P2PM_MULTI_OWNER_NOT_CURRENTLY_AUTHORITATIVE
	 */
	virtual RakNet::RM3QuerySerializationResult QuerySerialization_PeerToPeer(RakNet::Connection_RM3 *destinationConnection, Replica3P2PMode p2pMode=R3P2PM_SINGLE_OWNER);
	
	/* Default: If we are a client, and the connection is lost, delete the server's objects */
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection_Client(RakNet::Connection_RM3 *droppedConnection) const;
	/* Default: If we are a server, and the connection is lost, delete the client's objects and broadcast the destruction */
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection_Server(RakNet::Connection_RM3 *droppedConnection) const;
	/* Default: If we are a peer, and the connection is lost, delete the peer's objects */
	virtual RM3ActionOnPopConnection QueryActionOnPopConnection_PeerToPeer(RakNet::Connection_RM3 *droppedConnection) const;

	/*
	 * Call to send a network message to delete this object on other systems.<BR>
	 * Call it before deleting the object
	 */
	virtual void BroadcastDestruction();

	/*
	 * creatingSystemGUID is set the first time Reference() is called, or if we get the object from another system
	 * 返回值: System that originally created this object
	 */
	RakNetGUID GetCreatingSystemGUID(void) const;

    /* 返回值: If ReplicaManager3::Reference() was called on this object. */
	bool WasReferenced(void) const {return replicaManager!=0;}

	/*
	 * GUID of the system that first called Reference() on this object.
	 * Transmitted automatically when the object is constructed
	 */
	RakNetGUID creatingSystemGUID;
	/* GUID of the system that caused the item to send a deletion command over the network */
	RakNetGUID deletingSystemGUID;

	/*
	 * 内部使用
	 * ReplicaManager3 plugin associated with this object
	 */
	ReplicaManager3 *replicaManager;

	LastSerializationResultBS lastSentSerialization;
	bool forceSendUntilNextUpdate;
	LastSerializationResult *lsr;
	uint32_t referenceIndex;
};

/*
 * Use Replica3 through composition instead of inheritance by containing an instance of this templated class
 * Calls to parent class for all functions
 * Parent class must still define and functions though!
 * 前提条件: Parent class must call SetCompositeOwner() on this object
 */
template <class parent_type>
class RAK_DLL_EXPORT Replica3Composite : public Replica3
{
protected:
    parent_type *r3CompositeOwner;
public:
    void SetCompositeOwner(parent_type *p) {r3CompositeOwner=p;}
    parent_type* GetCompositeOwner(void) const {return r3CompositeOwner;};
	virtual void WriteAllocationID(RakNet::Connection_RM3 *destinationConnection, RakNet::BitStream *allocationIdBitstream) const {r3CompositeOwner->WriteAllocationID(destinationConnection, allocationIdBitstream);}
	virtual RakNet::RM3ConstructionState QueryConstruction(RakNet::Connection_RM3 *destinationConnection, RakNet::ReplicaManager3 *replicaManager3) {return r3CompositeOwner->QueryConstruction(destinationConnection, replicaManager3);}
	virtual RakNet::RM3DestructionState QueryDestruction(RakNet::Connection_RM3 *destinationConnection, RakNet::ReplicaManager3 *replicaManager3) {return r3CompositeOwner->QueryDestruction(destinationConnection, replicaManager3);}
	virtual bool QueryRemoteConstruction(RakNet::Connection_RM3 *sourceConnection) {return r3CompositeOwner->QueryRemoteConstruction(sourceConnection);}
	virtual bool QueryRelayDestruction(RakNet::Connection_RM3 *sourceConnection) const {return r3CompositeOwner->QueryRelayDestruction(sourceConnection);}
	virtual void SerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {r3CompositeOwner->SerializeConstruction(constructionBitstream, destinationConnection);}
	virtual bool DeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {return r3CompositeOwner->DeserializeConstruction(constructionBitstream, sourceConnection);}
	virtual void SerializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {r3CompositeOwner->SerializeConstructionExisting(constructionBitstream, destinationConnection);}
	virtual void DeserializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {r3CompositeOwner->DeserializeConstructionExisting(constructionBitstream, sourceConnection);}
	virtual void SerializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *destinationConnection) {r3CompositeOwner->SerializeDestruction(destructionBitstream, destinationConnection);}
	virtual bool DeserializeDestruction(RakNet::BitStream *destructionBitstream, RakNet::Connection_RM3 *sourceConnection) {return r3CompositeOwner->DeserializeDestruction(destructionBitstream, sourceConnection);}
	virtual RakNet::RM3ActionOnPopConnection QueryActionOnPopConnection(RakNet::Connection_RM3 *droppedConnection) const {return r3CompositeOwner->QueryActionOnPopConnection(droppedConnection);}
	virtual void OnPoppedConnection(RakNet::Connection_RM3 *droppedConnection) {r3CompositeOwner->OnPoppedConnection(droppedConnection);}
	virtual void DeallocReplica(RakNet::Connection_RM3 *sourceConnection) {r3CompositeOwner->DeallocReplica(sourceConnection);}
	virtual RakNet::RM3QuerySerializationResult QuerySerialization(RakNet::Connection_RM3 *destinationConnection) {return r3CompositeOwner->QuerySerialization(destinationConnection);}
	virtual void OnUserReplicaPreSerializeTick() {r3CompositeOwner->OnUserReplicaPreSerializeTick();}
	virtual RakNet::RM3SerializationResult Serialize(RakNet::SerializeParameters *serializeParameters) {return r3CompositeOwner->Serialize(serializeParameters);}
	virtual void OnSerializeTransmission(RakNet::BitStream *bitStream, RakNet::Connection_RM3 *destinationConnection, RakNet::BitSize_t bitsPerChannel[RakNet::RM3_NUM_OUTPUT_BITSTREAM_CHANNELS], RakNet::Time curTime) {r3CompositeOwner->OnSerializeTransmission(bitStream, destinationConnection, bitsPerChannel, curTime);}
	virtual void Deserialize(RakNet::DeserializeParameters *deserializeParameters) {r3CompositeOwner->Deserialize(deserializeParameters);}
	virtual void PostSerializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {r3CompositeOwner->PostSerializeConstruction(constructionBitstream, destinationConnection);}
	virtual void PostDeserializeConstruction(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {r3CompositeOwner->PostDeserializeConstruction(constructionBitstream, sourceConnection);}
	virtual void PostSerializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *destinationConnection) {r3CompositeOwner->PostSerializeConstructionExisting(constructionBitstream, destinationConnection);}
	virtual void PostDeserializeConstructionExisting(RakNet::BitStream *constructionBitstream, RakNet::Connection_RM3 *sourceConnection) {r3CompositeOwner->PostDeserializeConstructionExisting(constructionBitstream, sourceConnection);}
	virtual void PreDestruction(RakNet::Connection_RM3 *sourceConnection) {r3CompositeOwner->PreDestruction(sourceConnection);}
};

} /* RakNet 命名空间 */


#endif
