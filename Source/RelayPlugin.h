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
 * 包含 RelayPlugin 类
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RelayPlugin==1

#pragma once
#include "PluginInterface2.h"
#include "RakString.h"
#include "DS_Hash.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

/*
 * \defgroup RELAY_PLUGIN_GROUP RelayPlugin
 * A simple class to relay messages from one system to another through an intermediary
 * \ingroup PLUGINS_GROUP
 */

namespace RakNet
{

/* 前向声明 */
class RakPeerInterface;

enum RelayPluginEnums
{
	/* Server handled messages */
	RPE_MESSAGE_TO_SERVER_FROM_CLIENT,
	RPE_ADD_CLIENT_REQUEST_FROM_CLIENT,
	RPE_REMOVE_CLIENT_REQUEST_FROM_CLIENT,
	RPE_GROUP_MESSAGE_FROM_CLIENT,
	RPE_JOIN_GROUP_REQUEST_FROM_CLIENT,
	RPE_LEAVE_GROUP_REQUEST_FROM_CLIENT,
	RPE_GET_GROUP_LIST_REQUEST_FROM_CLIENT,
	/* Client handled messages */
	RPE_MESSAGE_TO_CLIENT_FROM_SERVER,
	RPE_ADD_CLIENT_NOT_ALLOWED,
	RPE_ADD_CLIENT_TARGET_NOT_CONNECTED,
	RPE_ADD_CLIENT_NAME_ALREADY_IN_USE,
	RPE_ADD_CLIENT_SUCCESS,
	RPE_USER_ENTERED_ROOM,
	RPE_USER_LEFT_ROOM,
	RPE_GROUP_MSG_FROM_SERVER,
	RPE_GET_GROUP_LIST_REPLY_FROM_SERVER,
	RPE_JOIN_GROUP_SUCCESS,
	RPE_JOIN_GROUP_FAILURE,
};

/*
 * A simple class to relay messages from one system to another, identifying remote systems by a string.
 * \ingroup RELAY_PLUGIN_GROUP
 */
class RAK_DLL_EXPORT RelayPlugin : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(RelayPlugin)

	/* 构造函数 */
	RelayPlugin();

	/* 析构函数 */
	virtual ~RelayPlugin();

	/*
	 * Forward messages from any system, to the system specified by the combination of key and guid. The sending system only needs to know the key.
	 * 参数[输入] key A string to identify the target's RakNetGUID. This is so the sending system does not need to know the RakNetGUID of the target system. The key should be unique among all guids added. If the key is not unique, only one system will be sent to (at random).
	 * 参数[输入] guid The RakNetGuid of the system to send to. If this system disconnects, it is removed from the 内部使用 hash
	 * 返回值: RPE_ADD_CLIENT_TARGET_NOT_CONNECTED, RPE_ADD_CLIENT_NAME_ALREADY_IN_USE, or RPE_ADD_CLIENT_OK
	 */
	RelayPluginEnums AddParticipantOnServer(const RakString &key, const RakNetGUID &guid);

	/* 移除 chat participant */
	void RemoveParticipantOnServer(const RakNetGUID &guid);

	/*
	 * If true, then if the client calls AddParticipantRequestFromClient(), the server will call AddParticipantOnServer() automatically
	 * 默认为 false
	 * 参数[输入] accept true to accept, false to not.
	 */
	void SetAcceptAddParticipantRequests(bool accept);

	/*
	 * Request from the client for the server to call AddParticipantOnServer()
	 * 前提条件: The server must have called SetAcceptAddParticipantRequests(true) or the request will be ignored
	 * 参数[输入] key A string to identify out system. Passed to key on AddParticipantOnServer()
	 * 参数[输入] relayPluginServerGuid the RakNetGUID of the system running RelayPlugin
	 */
	void AddParticipantRequestFromClient(const RakString &key, const RakNetGUID &relayPluginServerGuid);

	/* Remove yourself as a participant */
	void RemoveParticipantRequestFromClient(const RakNetGUID &relayPluginServerGuid);

	/*
	 * Request that the server relay bitStream to the system designated by key
	 * 参数[输入] relayPluginServerGuid the RakNetGUID of the system running RelayPlugin
	 * 参数[输入] destinationGuid The key value passed to AddParticipant() earlier on the server. If this was not done, the server will not relay the message (it will be silently discarded).
	 * 参数[输入] bitStream The data to relay
	 * 参数[输入] priority See the parameter of the same name in RakPeerInterface::Send()
	 * 参数[输入] reliability See the parameter of the same name in RakPeerInterface::Send()
	 * 参数[输入] orderingChannel See the parameter of the same name in RakPeerInterface::Send()
	 */
	void SendToParticipant(const RakNetGUID &relayPluginServerGuid, const RakString &destinationGuid, BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel);

	void SendGroupMessage(const RakNetGUID &relayPluginServerGuid, BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel);
	void JoinGroupRequest(const RakNetGUID &relayPluginServerGuid, RakString groupName);
	void LeaveGroup(const RakNetGUID &relayPluginServerGuid);
	void GetGroupList(const RakNetGUID &relayPluginServerGuid);

	/* 内部使用 */
	PluginReceiveResult OnReceive(Packet *packet) override;
	/* 内部使用 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;

	struct StrAndGuidAndRoom
	{
		RakString str;
		RakNetGUID guid;
		RakString currentRoom;
	};

	struct StrAndGuid
	{
		RakString str;
		RakNetGUID guid;
	};

	struct RP_Group
	{
		RakString roomName;
		DataStructures::List<StrAndGuid> usersInRoom;
	};
	
protected:

	RelayPlugin::RP_Group* JoinGroup(RakNetGUID userGuid, RakString roomName);
	RelayPlugin::RP_Group* JoinGroup(RP_Group* room, StrAndGuidAndRoom **strAndGuidSender);
	void LeaveGroup(StrAndGuidAndRoom **strAndGuidSender);
	void NotifyUsersInRoom(RP_Group *room, int msg, const RakString& message);
	void SendMessageToRoom(StrAndGuidAndRoom **strAndGuidSender, BitStream* message);
	void SendChatRoomsList(RakNetGUID target);
	void OnGroupMessageFromClient(Packet *packet);
	void OnJoinGroupRequestFromClient(Packet *packet);
	void OnLeaveGroupRequestFromClient(Packet *packet);

	DataStructures::Hash<RakString, StrAndGuidAndRoom*, 8096, RakNet::RakString::ToInteger> strToGuidHash;
	DataStructures::Hash<RakNetGUID, StrAndGuidAndRoom*, 8096, RakNet::RakNetGUID::ToUint32> guidToStrHash;
	DataStructures::List<RP_Group*> chatRooms;
	bool acceptAddParticipantRequests;

};

} /* 命名空间结束 */

#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
