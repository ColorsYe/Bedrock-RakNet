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
 *  ConnectionGraph2.h
 * Connection graph plugin, version 2. Tells new systems about existing and new connections
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ConnectionGraph2==1

#pragma once
#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "DS_List.h"
#include "DS_WeightedGraph.h"
#include "GetTime.h"
#include "Export.h"

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;

/*
 * A one hop connection graph.
 * 发送 ID_REMOTE_CONNECTION_LOST, ID_REMOTE_DISCONNECTION_NOTIFICATION, ID_REMOTE_NEW_INCOMING_CONNECTION<BR>
 * All identifiers are followed by SystemAddress, then RakNetGUID
 * Also stores the list for you, which you can access with GetConnectionListForRemoteSystem
 * \ingroup CONNECTION_GRAPH_GROUP
 */
class RAK_DLL_EXPORT ConnectionGraph2 : public PluginInterface2
{
public:

	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(ConnectionGraph2)

	ConnectionGraph2();
	~ConnectionGraph2() noexcept;

	/*
	 * Given a remote system identified by RakNetGUID, return the list of SystemAddresses and RakNetGUIDs they are connected to
	 * 参数[输入] remoteSystemGuid Which system we are referring to. This only works for remote systems, not ourselves.
	 * 参数[输出] saOut A preallocated array to hold the output list of SystemAddress. Can be 0 if you don't care.
	 * 参数[输出] guidOut A preallocated array to hold the output list of RakNetGUID. Can be 0 if you don't care.
	 * 参数[输入/输出] outLength On input, the size of saOut and guidOut. On output, modified to reflect the number of elements actually written
	 * 返回值: True if remoteSystemGuid was found. Otherwise false, and saOut, guidOut remain unchanged. outLength will be set to 0.
	 */
	bool GetConnectionListForRemoteSystem(RakNetGUID remoteSystemGuid, SystemAddress *saOut, RakNetGUID *guidOut, unsigned int *outLength);

	/* 返回 if g1 is connected to g2 */
	bool ConnectionExists(RakNetGUID g1, RakNetGUID g2);

	/* 返回 average ping between two systems in the connection graph. Returns -1 if no connection exists between those systems */
	uint16_t GetPingBetweenSystems(RakNetGUID g1, RakNetGUID g2) const;

	/*
	 * 返回 system with the lowest average ping among all its connections
	 * If you need one system in the peer to peer group to relay data, have the FullyConnectedMesh2 host call this function after host migration, and use that system
	 */
	RakNetGUID GetLowestAveragePingSystem(void) const;

	/*
	 * If called with false, then new connections are only added to the connection graph when you call ProcessNewConnection();
	 * This is useful if you want to perform validation before connecting a system to a mesh, or if you want a submesh (for example a server cloud)
	 * 参数[输入] b True to automatically call ProcessNewConnection() on any new connection, false to not do so. Defaults to true.
	 */
	void SetAutoProcessNewConnections(bool b);

	/*
	 * 返回传递给 SetAutoProcessNewConnections() 的值
	 * 返回值: Value passed to SetAutoProcessNewConnections(), or the default of true if it was never called
	 */
	bool GetAutoProcessNewConnections(void) const;

	/*
	 * If you call SetAutoProcessNewConnections(false);, then you will need to manually call ProcessNewConnection() on new connections
	 * On ID_NEW_INCOMING_CONNECTION or ID_CONNECTION_REQUEST_ACCEPTED, adds that system to the graph
	 * Do not call ProcessNewConnection() manually otherwise
	 * 参数[输入] The packet->SystemAddress member
	 * 参数[输入] The packet->guid member
	 */
	void AddParticipant(const SystemAddress &systemAddress, RakNetGUID rakNetGUID);

	/*
	 * 获取 participants added with AddParticipant()
	 * 参数[输出] participantList Participants added with AddParticipant();
	 */
	void GetParticipantList(DataStructures::OrderedList<RakNetGUID, RakNetGUID> &participantList);

	/* 内部使用 */
	struct SystemAddressAndGuid
	{
		SystemAddress systemAddress;
		RakNetGUID guid;
		uint16_t sendersPingToThatSystem;
	};
	/* 内部使用 */
	static int SystemAddressAndGuidComp( const SystemAddressAndGuid &key, const SystemAddressAndGuid &data );

	/* 内部使用 */
	struct RemoteSystem
	{
		DataStructures::OrderedList<SystemAddressAndGuid,SystemAddressAndGuid,ConnectionGraph2::SystemAddressAndGuidComp> remoteConnections;
		RakNetGUID guid;
	};
	/* 内部使用 */
	static int RemoteSystemComp( const RakNetGUID &key, RemoteSystem * const &data );
	
protected:
	/* 内部使用 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	/* 内部使用 */
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;
	/* 内部使用 */
	PluginReceiveResult OnReceive(Packet *packet) override;

	/* List of systems I am connected to, which in turn stores which systems they are connected to */
	DataStructures::OrderedList<RakNetGUID, RemoteSystem*, ConnectionGraph2::RemoteSystemComp> remoteSystems;

	bool autoProcessNewConnections;

};

} /* RakNet 命名空间 */

#endif /* #ifndef __CONNECTION_GRAPH_2_H */
