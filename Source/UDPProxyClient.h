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
 * A RakNet plugin performing networking to communicate with UDPProxyCoordinator. Ultimately used to tell UDPProxyServer to forward UDP packets.
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_UDPProxyClient==1

#pragma once
#include "Export.h"
#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "DS_List.h"

/*
 * \defgroup UDP_PROXY_GROUP UDPProxy
 * Forwards UDP datagrams from one system to another. Protocol independent
 * Used when NatPunchthroughClient fails
 * \ingroup PLUGINS_GROUP
 */

namespace RakNet
{
class UDPProxyClient;

/*
 * Callback to handle results of calling UDPProxyClient::RequestForwarding()
 * \ingroup UDP_PROXY_GROUP
 */
struct UDPProxyClientResultHandler
{
	UDPProxyClientResultHandler() {}
	virtual ~UDPProxyClientResultHandler() {}

	/*
	 * 当our forwarding request was completed. We can now connect to targetAddress by using proxyAddress instead时调用
	 * 参数[输出] proxyIPAddress IP Address of the proxy server, which will forward messages to targetAddress
	 * 参数[输出] proxyPort Remote port to use on the proxy server, which will forward messages to targetAddress
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. If it was UNASSIGNED_SYSTEM_ADDRESS, it is now our 外部 IP address.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnForwardingSuccess(const char *proxyIPAddress, unsigned short proxyPort,
		SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;

	/*
	 * 当another system has setup forwarding, with our system as the target address时调用
	 * Plugin automatically sends a datagram to proxyIPAddress before this callback, to open our router if necessary.
	 * 参数[输出] proxyIPAddress IP Address of the proxy server, which will forward messages to targetAddress
	 * 参数[输出] proxyPort Remote port to use on the proxy server, which will forward messages to targetAddress
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. This is originating source IP address of the remote system that will be sending to us.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding. This is our 外部 IP address.
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnForwardingNotification(const char *proxyIPAddress, unsigned short proxyPort,
		SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;

	/*
	 * 当our forwarding request failed, because no UDPProxyServers are connected to UDPProxyCoordinator时调用
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. If it was UNASSIGNED_SYSTEM_ADDRESS, it is now our 外部 IP address.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnNoServersOnline(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;

	/*
	 * 当our forwarding request failed, because no UDPProxyServers are connected to UDPProxyCoordinator时调用
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. If it was UNASSIGNED_SYSTEM_ADDRESS, it is now our 外部 IP address.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnRecipientNotConnected(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;

	/*
	 * 当our forwarding request failed, because all UDPProxyServers that are connected to UDPProxyCoordinator are at their capacity时调用
	 * Either add more servers, or increase capacity via UDPForwarder::SetMaxForwardEntries()
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. If it was UNASSIGNED_SYSTEM_ADDRESS, it is now our 外部 IP address.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnAllServersBusy(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;

	/*
	 * 当our forwarding request is already in progress on the proxyCoordinator时调用
	 * This can be ignored, but indicates an unneeded second request
	 * 参数[输出] proxyIPAddress IP Address of the proxy server, which is forwarding messages to targetAddress
	 * 参数[输出] proxyPort Remote port to use on the proxy server, which is forwarding messages to targetAddress
	 * 参数[输出] proxyCoordinator proxyCoordinator parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] sourceAddress sourceAddress parameter passed to UDPProxyClient::RequestForwarding. If it was UNASSIGNED_SYSTEM_ADDRESS, it is now our 外部 IP address.
	 * 参数[输出] targetAddress targetAddress parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] targetGuid targetGuid parameter originally passed to UDPProxyClient::RequestForwarding
	 * 参数[输出] proxyClient The plugin that is calling this callback
	 */
	virtual void OnForwardingInProgress(const char *proxyIPAddress, unsigned short proxyPort, SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddress, RakNetGUID targetGuid, RakNet::UDPProxyClient *proxyClientPlugin)=0;
};


/*
 * Communicates with UDPProxyCoordinator, in order to find a UDPProxyServer to forward our datagrams.
 * When NAT Punchthrough fails, it is possible to use a non-NAT system to forward messages from us to the recipient, and vice-versa.<BR>
 * The class to forward messages is UDPForwarder, and it is triggered over the network via the UDPProxyServer plugin.<BR>
 * The UDPProxyClient connects to UDPProxyCoordinator to get a list of servers running UDPProxyServer, and the coordinator will relay our forwarding request
 * 另见 NatPunchthroughServer
 * 另见 NatPunchthroughClient
 * \ingroup UDP_PROXY_GROUP
 */
class RAK_DLL_EXPORT UDPProxyClient : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(UDPProxyClient)

	UDPProxyClient();
	~UDPProxyClient() noexcept;

	/*
	 * Receives the results of calling RequestForwarding()
	 * 设置 before calling RequestForwarding or you won't know what happened
	 * 参数[输入] resultHandler
	 */
	void SetResultHandler(UDPProxyClientResultHandler *rh);

	/*
	 * Sends a request to proxyCoordinator to find a server and have that server setup UDPForwarder::StartForwarding() on our address to targetAddressAsSeenFromCoordinator
	 * The forwarded datagrams can be from any UDP source, not just RakNet
	 * 前提条件: Must be connected to proxyCoordinator
	 * 前提条件: Systems running UDPProxyServer must be connected to proxyCoordinator and logged in via UDPProxyCoordinator::LoginServer() or UDPProxyServer::LoginToCoordinator()
	 * 注意: May still fail, if all proxy servers have no open connections.
	 * 注意: RakNet's protocol will ensure a message is sent at least every 5 seconds, so if routing RakNet messages, it is a reasonable value for timeoutOnNoDataMS, plus an extra few seconds for latency.
	 * 参数[输入] proxyCoordinator System we are connected to that is running the UDPProxyCoordinator plugin
	 * 参数[输入] sourceAddress 外部 IP address of the system we want to forward messages from. This does not have to be our own system. To specify our own system, you can pass UNASSIGNED_SYSTEM_ADDRESS which the coordinator will treat as our 外部 IP address.
	 * 参数[输入] targetAddressAsSeenFromCoordinator 外部 IP address of the system we want to forward messages to. If this system is connected to UDPProxyCoordinator at this address using RakNet, that system will ping the server and thus open the router for incoming communication. In any other case, you are responsible for doing your own network communication to have that system ping the server. See also targetGuid in the other version of RequestForwarding(), to avoid the need to know the IP address to the coordinator of the destination.
	 * 参数[输入] timeoutOnNoData If no data is sent by the forwarded systems, how long before removing the forward entry from UDPForwarder? UDP_FORWARDER_MAXIMUM_TIMEOUT is the maximum value. Recommended 10 seconds.
	 * 参数[输入] serverSelectionBitstream If you want to send data to UDPProxyCoordinator::GetBestServer(), write it here
	 * 返回值: true if the request was sent, false if we are not connected to proxyCoordinator
	 */
	bool RequestForwarding(SystemAddress proxyCoordinator, SystemAddress sourceAddress, SystemAddress targetAddressAsSeenFromCoordinator, RakNet::TimeMS timeoutOnNoDataMS, RakNet::BitStream *serverSelectionBitstream=0);

	/*
	 * 与 above, but specify the target with a GUID, in case you don't know what its address is to the coordinator 相同
	 * If requesting forwarding to a RakNet enabled system, then it is easier to use targetGuid instead of targetAddressAsSeenFromCoordinator
	 */
	bool RequestForwarding(SystemAddress proxyCoordinator, SystemAddress sourceAddress, RakNetGUID targetGuid, RakNet::TimeMS timeoutOnNoDataMS, RakNet::BitStream *serverSelectionBitstream=0);

	/* 内部使用 */
	void Update() override;
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnRakPeerShutdown() override;
	
	struct ServerWithPing
	{
		unsigned short ping;
		SystemAddress serverAddress;
	};
	struct SenderAndTargetAddress
	{
		SystemAddress senderClientAddress;
		SystemAddress targetClientAddress;
	};
	struct PingServerGroup
	{
		SenderAndTargetAddress sata;
		RakNet::TimeMS startPingTime;
		SystemAddress coordinatorAddressForPings;
		/* DataStructures::Multilist<ML_UNORDERED_LIST, ServerWithPing> serversToPing; */
		DataStructures::List<ServerWithPing> serversToPing;
		bool AreAllServersPinged(void) const;
		void SendPingedServersToCoordinator(RakPeerInterface *rakPeerInterface);
	};
	/* DataStructures::Multilist<ML_UNORDERED_LIST, PingServerGroup*> pingServerGroups; */
	DataStructures::List<PingServerGroup*> pingServerGroups;
protected:

	void OnPingServers(Packet *packet);
	void Clear();
	UDPProxyClientResultHandler *resultHandler;

};

} /* 命名空间结束 */

#endif
