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
 * A RakNet plugin performing networking to communicate with UDPProxyServer. It allows UDPProxyServer to control our instance of UDPForwarder.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_UDPProxyServer==1 && _RAKNET_SUPPORT_UDPForwarder==1

#pragma once
#include "Export.h"
#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "UDPForwarder.h"
#include "RakString.h"

namespace RakNet
{
class UDPProxyServer;

/*
 * Callback to handle results of calling UDPProxyServer::LoginToCoordinator()
 * \ingroup UDP_PROXY_GROUP
 */
struct UDPProxyServerResultHandler
{
	UDPProxyServerResultHandler() {}
	virtual ~UDPProxyServerResultHandler() {}

	/*
	 * 当our login succeeds时调用
	 * 参数[输出] usedPassword The password we passed to UDPProxyServer::LoginToCoordinator()
	 * 参数[输出] proxyServer The plugin calling this callback
	 */
	virtual void OnLoginSuccess(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)=0;

	/*
	 * We are already logged in.
	 * This login failed, but the system is operational as if it succeeded
	 * 参数[输出] usedPassword The password we passed to UDPProxyServer::LoginToCoordinator()
	 * 参数[输出] proxyServer The plugin calling this callback
	 */
	virtual void OnAlreadyLoggedIn(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)=0;

	/*
	 * The coordinator operator forgot to call UDPProxyCoordinator::SetRemoteLoginPassword()
	 * 参数[输出] usedPassword The password we passed to UDPProxyServer::LoginToCoordinator()
	 * 参数[输出] proxyServer The plugin calling this callback
	 */
	virtual void OnNoPasswordSet(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)=0;

	/*
	 * The coordinator operator set a different password in UDPProxyCoordinator::SetRemoteLoginPassword() than what we passed
	 * 参数[输出] usedPassword The password we passed to UDPProxyServer::LoginToCoordinator()
	 * 参数[输出] proxyServer The plugin calling this callback
	 */
	virtual void OnWrongPassword(RakNet::RakString usedPassword, RakNet::UDPProxyServer *proxyServerPlugin)=0;
};

/*
 * UDPProxyServer to control our instance of UDPForwarder
 * When NAT Punchthrough fails, it is possible to use a non-NAT system to forward messages from us to the recipient, and vice-versa.<BR>
 * The class to forward messages is UDPForwarder, and it is triggered over the network via the UDPProxyServer plugin.<BR>
 * The UDPProxyServer connects to UDPProxyServer to get a list of servers running UDPProxyServer, and the coordinator will relay our forwarding request.
 * \ingroup UDP_PROXY_GROUP
 */
class RAK_DLL_EXPORT UDPProxyServer : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(UDPProxyServer)

	UDPProxyServer();
	~UDPProxyServer() noexcept;

	/*
	 * 将socket family设置为use, either IPV4 or IPV6
	 * 参数[输入] socketFamily For IPV4, use AF_INET (default). For IPV6, use AF_INET6. To autoselect, use AF_UNSPEC.
	 */
	void SetSocketFamily(unsigned short _socketFamily);

	/*
	 * Receives the results of calling LoginToCoordinator()
	 * 设置 before calling LoginToCoordinator or you won't know what happened
	 * 参数[输入] resultHandler
	 */
	void SetResultHandler(UDPProxyServerResultHandler *rh);

	/*
	 * Before the coordinator will register the UDPProxyServer, you must login
	 * 前提条件: Must be connected to the coordinator
	 * 前提条件: Coordinator must have set a password with UDPProxyCoordinator::SetRemoteLoginPassword()
	 * 返回值: false if already logged in, or logging in. Returns true otherwise
	 */
	bool LoginToCoordinator(RakNet::RakString password, SystemAddress coordinatorAddress);

	/*
	 * The server IP reported to the client is the IP address from the server to the coordinator.
	 * If the server and coordinator are on the same LAN, you need to call SetServerPublicIP() to tell the client what address to connect to
	 * 参数[输入] ip IP address to report in UDPProxyClientResultHandler::OnForwardingSuccess() and UDPProxyClientResultHandler::OnForwardingNotification() as proxyIPAddress
	 */
	void SetServerPublicIP(RakString ip);

	/*
	 * Operative class that performs the forwarding
	 * Exposed so you can call UDPForwarder::SetMaxForwardEntries() if you want to change away from the default
	 * UDPForwarder::Startup(), UDPForwarder::Shutdown(), and UDPForwarder::Update() are called automatically by the plugin
	 */
	UDPForwarder udpForwarder;

	void OnAttach() override;
	void OnDetach() override;

	/* 内部使用 */
	void Update() override;
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	void OnRakPeerStartup() override;
	void OnRakPeerShutdown() override;

protected:
	void OnForwardingRequestFromCoordinatorToServer(Packet *packet);

	DataStructures::OrderedList<SystemAddress, SystemAddress> loggingInCoordinators;
	DataStructures::OrderedList<SystemAddress, SystemAddress> loggedInCoordinators;

	UDPProxyServerResultHandler *resultHandler;
	unsigned short socketFamily;
	RakString serverPublicIp;

};

} /* 命名空间结束 */

#endif
