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
 * Contains TransportInterface from which you can derive custom transport providers for ConsoleServer.
 *
 */



#pragma once
#include "RakNetTypes.h"
#include "Export.h"
#include "RakMemoryOverride.h"

#define REMOTE_MAX_TEXT_INPUT 2048

namespace RakNet
{

class CommandParserInterface;


/*
 * Defines an interface that is used to send and receive null-terminated strings.
 * In practice this is only used by the CommandParser system for for servers.
 */
class RAK_DLL_EXPORT TransportInterface
{
public:
	TransportInterface() {}
	virtual ~TransportInterface() {}

	/*
	 * Start the transport provider on the indicated port.
	 * 参数[输入] port The port to start the transport provider on
	 * 参数[输入] serverMode If true, you should allow incoming connections (I don't actually use this anywhere)
	 * 返回值: Return 成功返回 true，失败返回 false。
	 */
	virtual bool Start(unsigned short port, bool serverMode)=0;

	/* Stop the transport provider.  You can clear memory and shutdown threads here. */
	virtual void Stop()=0;

	/*
	 * Send a 以空字符结尾的字符串 to systemAddress
	 * If your transport method requires particular formatting of the outgoing data (e.g. you don't just send strings) you can do it here
	 * and parse it out in Receive().
	 * 参数[输入] systemAddress The player to send the string to
	 * 参数[输入] data format specifier - same as RAKNET_DEBUG_PRINTF
	 * 参数[输入] ... format specification arguments - same as RAKNET_DEBUG_PRINTF
	 */
	virtual void Send( SystemAddress systemAddress, const char *data, ... )=0;

	/*
	 * Disconnect systemAddress .  The binary address and port defines the SystemAddress structure.
	 * 参数[输入] systemAddress The player/address to disconnect
	 */
	virtual void CloseConnection( SystemAddress systemAddress )=0;

	/*
	 * 返回一个字符串。该字符串应被分配并写入 Packet::data。
	 * The byte length should be written to Packet::length .  The player/address should be written to Packet::systemAddress
	 * If your transport protocol adds special formatting to the data stream you should parse it out before returning it in the packet
	 * and thus only return a string in Packet::data
	 * 返回值: The packet structure containing the result of Receive, or 0 if no data is available
	 */
	virtual Packet* Receive( void )=0;

	/*
	 * Deallocate the Packet structure returned by Receive
	 * 参数[输入] The packet to deallocate
	 */
	virtual void DeallocatePacket( Packet *packet )=0;

	/*
	 * If a new system connects to you, you should queue that event and return the systemAddress/address of that player in this function.
	 * 返回值: The SystemAddress/address of the system
	 */
	virtual SystemAddress HasNewIncomingConnection()=0;

	/*
	 * If a system loses the connection, you should queue that event and return the systemAddress/address of that player in this function.
	 * 返回值: The SystemAddress/address of the system
	 */
	virtual SystemAddress HasLostConnection()=0;

	/*
	 * Your transport provider can itself have command parsers if the transport layer has user-modifiable features
	 * For example, your transport layer may have a password which you want remote users to be able to set or you may want
	 * to allow remote users to turn on or off command echo
	 * 返回值: 0 if you do not need a command parser - otherwise the desired derivation of CommandParserInterface
	 */
	virtual CommandParserInterface* GetCommandParser()=0;
protected:
};

} /* RakNet 命名空间 */
