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
 * \b RakNet 的插件功能系统，第 2 版。你可以从此类派生来创建自己的插件。
 *
 */


#pragma once
#include "NativeFeatureIncludes.h"
#include "RakNetTypes.h"
#include "Export.h"
#include "PacketPriority.h"

namespace RakNet {

/* 前向声明 */
class RakPeerInterface;
class TCPInterface;
struct Packet;
struct InternalPacket;

/* \defgroup PLUGIN_INTERFACE_GROUP PluginInterface2 */

/*
 * \defgroup PLUGINS_GROUP Plugins
 * \ingroup PLUGIN_INTERFACE_GROUP
 */

/*
 * 对于到达 RakPeer 实例的每条消息，插件有机会优先处理它们。此枚举表示对该消息的处理方式
 * \ingroup PLUGIN_INTERFACE_GROUP
 */
enum PluginReceiveResult
{
	/* 插件已使用此消息，不应将其传递给用户。 */
	RR_STOP_PROCESSING_AND_DEALLOCATE=0,

	/* 此消息将由其他插件处理，最终由用户处理。 */
	RR_CONTINUE_PROCESSING,

	/* 插件将保留此消息。不要释放它，但也不要将它传递给其他插件。 */
	RR_STOP_PROCESSING
};

/*
 * 连接断开的原因
 * \ingroup PLUGIN_INTERFACE_GROUP
 */
enum PI2_LostConnectionReason
{
	/* 调用了 RakPeer::CloseConnection() */
	LCR_CLOSED_BY_USER,

	/* 收到 ID_DISCONNECTION_NOTIFICATION */
	LCR_DISCONNECTION_NOTIFICATION,

	/* 收到 ID_CONNECTION_LOST */
	LCR_CONNECTION_LOST
};

/*
 * 返回连接尝试失败的原因
 * \ingroup PLUGIN_INTERFACE_GROUP
 */
enum PI2_FailedConnectionAttemptReason
{
	FCAR_CONNECTION_ATTEMPT_FAILED,
	FCAR_ALREADY_CONNECTED,
	FCAR_NO_FREE_INCOMING_CONNECTIONS,
	FCAR_SECURITY_PUBLIC_KEY_MISMATCH,
	FCAR_CONNECTION_BANNED,
	FCAR_INVALID_PASSWORD,
	FCAR_INCOMPATIBLE_PROTOCOL,
	FCAR_IP_RECENTLY_CONNECTED,
	FCAR_REMOTE_SYSTEM_REQUIRES_PUBLIC_KEY,
	FCAR_OUR_SYSTEM_REQUIRES_SECURITY,
	FCAR_PUBLIC_KEY_MISMATCH
};

/*
 * RakNet 的插件系统。每个插件处理以下事件：
 * - 连接尝试
 * - 连接尝试的结果
 * - 每条传入消息
 * - 当 RakPeer::Receive() 被调用时随时间推进的更新
 *
 * \ingroup PLUGIN_INTERFACE_GROUP
 */
class RAK_DLL_EXPORT PluginInterface2
{
public:
	PluginInterface2();
	virtual ~PluginInterface2();

	/* 当接口被附加时调用 */
	virtual void OnAttach() {}

	/* 当接口被分离时调用 */
	virtual void OnDetach() {}

	/* 每次检查数据包时调用 Update。 */
	virtual void Update() {}

	/*
	 * 对每个数据包调用 OnReceive。
	 * 参数[输入] packet 正在返回给用户的数据包
	 * 返回值: true 允许游戏和其他插件获取此消息，false 将其吸收
	 */
	virtual PluginReceiveResult OnReceive(Packet *packet) {(void) packet; return RR_CONTINUE_PROCESSING;}

	/* 当 RakPeer 初始化时调用 */
	virtual void OnRakPeerStartup() {}

	/* 当 RakPeer 关闭时调用 */
	virtual void OnRakPeerShutdown() {}

	/*
	 * 当用户对特定系统调用 RakPeer::CloseConnection() 而导致连接断开时调用
	 * 参数[输入] systemAddress 连接被关闭的系统地址
	 * 参数[输入] rakNetGuid 指定系统的 GUID
	 * 参数[输入] lostConnectionReason 连接关闭的方式：手动关闭、连接丢失或断开通知
	 */
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ){(void) systemAddress; (void) rakNetGUID; (void) lostConnectionReason;}

	/*
	 * 当收到新连接时调用
	 * 参数[输入] systemAddress 新连接的地址
	 * 参数[输入] rakNetGuid 指定系统的 GUID
	 * 参数[输入] isIncoming 若为 true，则为 ID_NEW_INCOMING_CONNECTION 或等效消息
	 */
	virtual void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) {(void) systemAddress; (void) rakNetGUID; (void) isIncoming;}

	/*
	 * 当连接尝试失败时调用
	 * 参数[输入] packet 返回给用户的数据包
	 * 参数[输入] failedConnectionReason 连接失败的原因
	 */
	virtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason) {(void) packet; (void) failedConnectionAttemptReason;}

	/*
	 * 附加到 RakPeer 时查询
	 * 返回 true 以调用 OnDirectSocketSend()、OnDirectSocketReceive()、OnReliabilityLayerNotification()、OnInternalPacket() 和 OnAck()
	 * 若为 true，则在 RakPeer 活动期间不能调用 RakPeer::AttachPlugin() 或 RakPeer::DetachPlugin()
	 */
	virtual bool UsesReliabilityLayer(void) const {return false;}

	/*
	 * 在每个数据报发送到套接字时调用，该数据报不经过可靠性层
	 * 前提条件: 要调用此函数，UsesReliabilityLayer() 必须返回 true
	 * 参数[输入] data 正在发送的数据
	 * 参数[输入] bitsUsed 数据的位长度
	 * 参数[输入] remoteSystemAddress 此消息正在发送到的系统
	 */
	virtual void OnDirectSocketSend(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress) {(void) data; (void) bitsUsed; (void) remoteSystemAddress;}

	/*
	 * 在每个数据报从套接字接收时调用，该数据报不经过可靠性层
	 * 前提条件: 要调用此函数，UsesReliabilityLayer() 必须返回 true
	 * 参数[输入] data 正在发送的数据
	 * 参数[输入] bitsUsed 数据的位长度
	 * 参数[输入] remoteSystemAddress 此消息正在发送到的系统
	 */
	virtual void OnDirectSocketReceive(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress) {(void) data; (void) bitsUsed; (void) remoteSystemAddress;}

	/*
	 * 当可靠性层拒绝发送或接收时调用
	 * 前提条件: 要调用此函数，UsesReliabilityLayer() 必须返回 true
	 * 参数[输入] bitsUsed 数据的位长度
	 * 参数[输入] remoteSystemAddress 此消息正在发送到的系统
	 */
	virtual void OnReliabilityLayerNotification(const char *errorMessage, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress, bool isError)  {(void) errorMessage; (void) bitsUsed; (void) remoteSystemAddress; (void) isError;}

	/*
	 * 在可靠性层内发送或接收消息时调用
	 * 前提条件: 要调用此函数，UsesReliabilityLayer() 必须返回 true
	 * 参数[输入] internalPacket 用户消息，以及所有发送数据。
	 * 参数[输入] frameNumber 到目前为止，根据 isSend，为此玩家发送或接收的帧数。表示此用户消息的帧。
	 * 参数[输入] remoteSystemAddress 我们向其发送或从其接收此数据包的玩家
	 * 参数[输入] time RakNet::GetTimeMS() 返回的当前时间
	 * 参数[输入] isSend 此回调表示发送事件还是接收事件？
	 */
	virtual void OnInternalPacket(InternalPacket *internalPacket, unsigned frameNumber, SystemAddress remoteSystemAddress, RakNet::TimeMS time, int isSend) {(void) internalPacket; (void) frameNumber; (void) remoteSystemAddress; (void) time; (void) isSend;}

	/*
	 * 当我们收到可靠发送消息的确认时调用
	 * 前提条件: 要调用此函数，UsesReliabilityLayer() 必须返回 true
	 * 参数[输入] messageNumber 此消息的数字标识符
	 * 参数[输入] remoteSystemAddress 我们向其发送或从其接收此数据包的玩家
	 * 参数[输入] time RakNet::GetTimeMS() 返回的当前时间
	 */
	virtual void OnAck(unsigned int messageNumber, SystemAddress remoteSystemAddress, RakNet::TimeMS time) {(void) messageNumber; (void) remoteSystemAddress; (void) time;}

	/*
	 * 系统调用了 RakPeerInterface::PushBackPacket
	 * 参数[输入] data 正在发送的数据
	 * 参数[输入] bitsUsed 数据的位长度
	 * 参数[输入] remoteSystemAddress 我们向其发送或从其接收此数据包的玩家
	 */
	virtual void OnPushBackPacket(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress) {(void) data; (void) bitsUsed; (void) remoteSystemAddress;}

	RakPeerInterface *GetRakPeerInterface(void) const {return rakPeerInterface;}

	RakNetGUID GetMyGUIDUnified(void) const;

	/* 内部使用 */
	void SetRakPeerInterface( RakPeerInterface *ptr );

#if _RAKNET_SUPPORT_TCPInterface==1
	/* 内部使用 */
	void SetTCPInterface( TCPInterface *ptr );
#endif

protected:
	/* 通过 rakPeerInterface 或 tcpInterface 发送，使用可用的那个 */
	void SendUnified( const RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast );
	void SendUnified( const char * data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast );
	bool SendListUnified( const char **data, const int *lengths, const int numParameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast );

	Packet *AllocatePacketUnified(unsigned dataSize);
	void PushBackPacketUnified(Packet *packet, bool pushAtHead);
	void DeallocPacketUnified(Packet *packet);

	/* 附加时自动填充 */
	RakPeerInterface *rakPeerInterface;
#if _RAKNET_SUPPORT_TCPInterface==1
	TCPInterface *tcpInterface;
#endif
};

} /* RakNet 命名空间 */
