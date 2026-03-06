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
 * Contains RakNetTransportCommandParser and RakNetTransport used to provide a secure console connection.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_TelnetTransport==1

#pragma once
#include "TransportInterface.h"
#include "DS_Queue.h"
#include "CommandParserInterface.h"
#include "PluginInterface2.h"
#include "Export.h"

namespace RakNet
{
/* 前向声明 */
class BitStream;
class RakPeerInterface;
class RakNetTransport;

/*
 * \defgroup RAKNET_TRANSPORT_GROUP RakNetTransport
 * UDP based transport implementation for the ConsoleServer
 * * \ingroup PLUGINS_GROUP
 */

/*
 * Use RakNetTransport if you need a secure connection between the client and the console server.
 * RakNetTransport automatically initializes security for the system.  Use the project CommandConsoleClient to connect
 * To the ConsoleServer if you use RakNetTransport
 * \ingroup RAKNET_TRANSPORT_GROUP
 */
class RAK_DLL_EXPORT RakNetTransport2 : public TransportInterface, public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(RakNetTransport2)

	RakNetTransport2();
    virtual ~RakNetTransport2();

	/*
	 * Start the transport provider on the indicated port.
	 * 参数[输入] port The port to start the transport provider on
	 * 参数[输入] serverMode If true, you should allow incoming connections (I don't actually use this anywhere)
	 * 返回值: Return 成功返回 true，失败返回 false。
	 */
	bool Start(unsigned short port, bool serverMode) override;

	/* Stop the transport provider.  You can clear memory and shutdown threads here. */
	void Stop() override;

	/*
	 * Send a 以空字符结尾的字符串 to systemAddress
	 * If your transport method requires particular formatting of the outgoing data (e.g. you don't just send strings) you can do it here
	 * and parse it out in Receive().
	 * 参数[输入] systemAddress The player to send the string to
	 * 参数[输入] data format specifier - same as RAKNET_DEBUG_PRINTF
	 * 参数[输入] ... format specification arguments - same as RAKNET_DEBUG_PRINTF
	 */
	void Send( SystemAddress systemAddress, const char *data, ... ) override;

	/*
	 * Disconnect systemAddress .  The binary address and port defines the SystemAddress structure.
	 * 参数[输入] systemAddress The player/address to disconnect
	 */
	void CloseConnection( SystemAddress systemAddress ) override;

	/*
	 * 返回一个字符串。该字符串应被分配并写入 Packet::data。
	 * The byte length should be written to Packet::length .  The player/address should be written to Packet::systemAddress
	 * If your transport protocol adds special formatting to the data stream you should parse it out before returning it in the packet
	 * and thus only return a string in Packet::data
	 * 返回值: The packet structure containing the result of Receive, or 0 if no data is available
	 */
	Packet* Receive( void ) override;

	/*
	 * Deallocate the Packet structure returned by Receive
	 * 参数[输入] The packet to deallocate
	 */
	void DeallocatePacket( Packet *packet ) override;

	/*
	 * If a new system connects to you, you should queue that event and return the systemAddress/address of that player in this function.
	 * 返回值: The SystemAddress/address of the system
	 */
	SystemAddress HasNewIncomingConnection() override;

	/*
	 * If a system loses the connection, you should queue that event and return the systemAddress/address of that player in this function.
	 * 返回值: The SystemAddress/address of the system
	 */
	SystemAddress HasLostConnection() override;

	CommandParserInterface* GetCommandParser() override {return 0;}

	/* 内部使用 */
	PluginReceiveResult OnReceive(Packet *packet) override;
	/* 内部使用 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;
	/* 内部使用 */
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;
protected:
	DataStructures::Queue<SystemAddress> newConnections, lostConnections;
	DataStructures::Queue<Packet*> packetQueue;
};

} /* RakNet 命名空间 */

#endif
