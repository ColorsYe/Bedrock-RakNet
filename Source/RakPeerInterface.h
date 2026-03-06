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
 * An interface for RakPeer.  Simply contains all user functions as pure virtuals.
 *
 */



#pragma once
#include "PacketPriority.h"
#include "RakNetTypes.h"
#include "RakMemoryOverride.h"
#include "Export.h"
#include "DS_List.h"
#include "RakNetSmartPtr.h"
#include "RakNetSocket2.h"

namespace RakNet
{
/* 前向声明 */
class BitStream;
class PluginInterface2;
struct RPCMap;
struct RakNetStatistics;
struct RakNetBandwidth;
class RouterInterface;
class NetworkIDManager;

/*
 * The primary interface for RakNet, RakPeer contains all major functions for the library.
 * 参见 the individual functions for what the class can do
 * The main interface for network communications
 */
class RAK_DLL_EXPORT RakPeerInterface
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(RakPeerInterface)

	/* 析构函数 */
	virtual ~RakPeerInterface()	{}

	/* --------------------------------------------------------------------------------------------Major Low Level Functions - Functions needed by most users-------------------------------------------------------------------------------------------- */
	/*
	 * 启动网络线程，打开监听端口。
	 * 在调用 Connect() 之前必须先调用此函数。
	 * 前提条件: On the PS3, call Startup() after Client_Login()
	 * 前提条件: On Android, add the necessary permission to your application's androidmanifest.xml: <uses-permission android:name="android.permission.INTERNET" />
	 * Multiple calls while already active are ignored.  To call this function again with different settings, you must first call Shutdown().
	 * 注意: Call SetMaximumIncomingConnections if you want to accept incoming connections
	 * 参数[输入] maxConnections The maximum number of connections between this instance of RakPeer and another instance of RakPeer. 必须 so the network can preallocate and for thread safety. A pure client would set this to 1.  A pure server would set it to the number of allowed clients.- A hybrid would set it to the sum of both types of connections
	 * 参数[输入] localPort The port to listen for connections on. On linux the system may be set up so thast ports under 1024 are restricted for everything but the root user. Use a higher port for maximum compatibility.
	 * 参数[输入] socketDescriptors An array of SocketDescriptor structures to force RakNet to listen on a particular IP address or port (or both).  Each SocketDescriptor will represent one unique socket.  Do not pass redundant structures.  To listen on a specific port, you can pass SocketDescriptor(myPort,0); such as for a server.  For a client, it is usually OK to just pass SocketDescriptor(); However, on the XBOX be sure to use IPPROTO_VDP
	 * 参数[输入] socketDescriptorCount The size of the socketDescriptors array.  Pass 1 if you are not sure what to pass.
	 * 参数[输入] threadPriority Passed to the thread creation routine. Use THREAD_PRIORITY_NORMAL for Windows. For Linux based systems, you MUST pass something reasonable based on the thread priorities for your application.
	 * 返回值: StartupResult::RAKNET_STARTED on success, otherwise appropriate failure enumeration.
	 */
	virtual StartupResult Startup( unsigned int maxConnections, SocketDescriptor *socketDescriptors, unsigned socketDescriptorCount, int threadPriority=-99999 )=0;

	/*
	 * If you accept connections, you must call this or else security will not be enabled for incoming connections.
	 * This feature requires more round trips, bandwidth, and CPU time for the connection handshake
	 * x64 builds require under 25% of the CPU time of other builds
	 * 参见 the Encryption sample for example usage
	 * 前提条件: Must be called while offline
	 * 前提条件: LIBCAT_SECURITY must be defined to 1 in NativeFeatureIncludes.h for this function to have any effect
	 * 参数[输入] publicKey A pointer to the public key for accepting new connections
	 * 参数[输入] privateKey A pointer to the private key for accepting new connections
	 * 参数[输入] bRequireClientKey: Should be set to false for most servers.  Allows the server to accept a public key from connecting clients as a proof of identity but eats twice as much CPU time as a normal connection
	 */
	virtual bool InitializeSecurity( const char *publicKey, const char *privateKey, bool bRequireClientKey = false )=0;

	/*
	 * Disables security for incoming connections.
	 * 注意: Must be called while offline
	 */
	virtual void DisableSecurity( void )=0;

	/*
	 * If secure connections are on, do not use secure connections for a specific IP address.
	 * This is useful if you have a fixed-address 内部使用 server behind a LAN.
	 * 注意: Secure connections are determined by the recipient of an incoming connection. This has no effect if called on the system attempting to connect.
	 * 参数[输入] ip IP address to add. * wildcards are supported.
	 */
	virtual void AddToSecurityExceptionList(const char *ip)=0;

	/*
	 * Remove a specific connection previously added via AddToSecurityExceptionList
	 * 参数[输入] ip IP address to remove. Pass 0 to remove all IP addresses. * wildcards are supported.
	 */
	virtual void RemoveFromSecurityExceptionList(const char *ip)=0;

	/*
	 * Checks to see if a given IP is in the security exception list
	 * 参数[输入] IP address to check.
	 */
	virtual bool IsInSecurityExceptionList(const char *ip)=0;

	/*
	 * 设置 how many incoming connections are allowed. If this is less than the number of players currently connected,
	 * no more players will be allowed to connect.  If this is greater than the maximum number of peers allowed,
	 * it will be reduced to the maximum number of peers allowed.
	 * Defaults to 0, meaning by default, nobody can connect to you
	 * 参数[输入] numberAllowed Maximum number of incoming connections allowed.
	 */
	virtual void SetMaximumIncomingConnections( unsigned short numberAllowed )=0;

	/*
	 * 返回 value passed to SetMaximumIncomingConnections()
	 * 返回值: the maximum number of incoming connections, which is always <= maxConnections
	 */
	virtual unsigned int GetMaximumIncomingConnections( void ) const=0;

	/*
	 * Returns how many open connections there are at this time
	 * 返回值: the number of open connections
	 */
	virtual unsigned short NumberOfConnections(void) const=0;

	/*
	 * Sets the password incoming connections must match in the call to Connect (defaults to 无). Pass 0 to passwordData to specify no password
	 * This is a way to set a low level password for all incoming connections.  To selectively reject connections, implement your own scheme using CloseConnection() to remove unwanted connections
	 * 参数[输入] passwordData A data block that incoming connections must match.  This can be just a password, or can be a stream of data. Specify 0 for no password data
	 * 参数[输入] passwordDataLength The length in bytes of passwordData
	 */
	virtual void SetIncomingPassword( const char* passwordData, int passwordDataLength )=0;

	/*
	 * 获取 password passed to SetIncomingPassword
	 * 参数[输出] passwordData  Should point to a block large enough to hold the password data you passed to SetIncomingPassword()
	 * 参数[输入/输出] passwordDataLength Maximum size of the array passwordData.  Modified to hold the number of bytes actually written
	 */
	virtual void GetIncomingPassword( char* passwordData, int *passwordDataLength  )=0;

	/*
	 * 连接到指定主机（IP 或域名）和服务器端口。
	 * Calling Connect and not calling SetMaximumIncomingConnections acts as a dedicated client.
	 * Calling both acts as a true peer. This is a non-blocking connection.
	 * You know the connection is successful when GetConnectionState() returns IS_CONNECTED or Receive() gets a message with the type identifier ID_CONNECTION_REQUEST_ACCEPTED.
	 * If the connection is not successful, such as a rejected connection or no response then neither of these things will happen.
	 * 前提条件: Requires that you first call Startup()
	 * 参数[输入] host Either a dotted IP address or a domain name
	 * 参数[输入] remotePort Which port to connect to on the remote machine.
	 * 参数[输入] passwordData A data block that must match the data block on the server passed to SetIncomingPassword.  This can be a string or can be a stream of data.  Use 0 for no password.
	 * 参数[输入] passwordDataLength The length in bytes of passwordData
	 * 参数[输入] publicKey The public key the server is using. If 0, the server is not using security. If non-zero, the publicKeyMode member determines how to connect
	 * 参数[输入] connectionSocketIndex Index into the array of socket descriptors passed to socketDescriptors in RakPeer::Startup() to send on.
	 * 参数[输入] sendConnectionAttemptCount How many datagrams to send to the other system to try to connect.
	 * 参数[输入] timeBetweenSendConnectionAttemptsMS Time to elapse before a datagram is sent to the other system to try to connect. After sendConnectionAttemptCount number of attempts, ID_CONNECTION_ATTEMPT_FAILED is returned. Under low bandwidth conditions with multiple simultaneous outgoing connections, this value should be raised to 1000 or higher, or else the MTU detection can overrun the available bandwidth.
	 * 参数[输入] timeoutTime How long to keep the connection alive before dropping it on unable to send a reliable message. 0 to use the default from SetTimeoutTime(UNASSIGNED_SYSTEM_ADDRESS);
	 * 返回值: ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED on successful initiation. Otherwise, an appropriate enumeration indicating failure.
	 * 注意: ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED does not mean you are already connected!
	 * 注意: It is possible to immediately get back ID_CONNECTION_ATTEMPT_FAILED if you exceed the maxConnections parameter passed to Startup(). This could happen if you call CloseConnection() with sendDisconnectionNotificaiton true, then immediately call Connect() before the connection has closed.
	 */
	virtual ConnectionAttemptResult Connect( const char* host, unsigned short remotePort, const char *passwordData, int passwordDataLength, PublicKey *publicKey=0, unsigned connectionSocketIndex=0, unsigned sendConnectionAttemptCount=12, unsigned timeBetweenSendConnectionAttemptsMS=500, RakNet::TimeMS timeoutTime=0 )=0;

	/*
	 * Connect to the specified host (ip or domain name) and server port, using a shared socket from another instance of RakNet
	 * 参数[输入] host Either a dotted IP address or a domain name
	 * 参数[输入] remotePort Which port to connect to on the remote machine.
	 * 参数[输入] passwordData A data block that must match the data block on the server passed to SetIncomingPassword.  This can be a string or can be a stream of data.  Use 0 for no password.
	 * 参数[输入] passwordDataLength The length in bytes of passwordData
	 * 参数[输入] socket A bound socket returned by another instance of RakPeerInterface
	 * 参数[输入] sendConnectionAttemptCount How many datagrams to send to the other system to try to connect.
	 * 参数[输入] timeBetweenSendConnectionAttemptsMS Time to elapse before a datagram is sent to the other system to try to connect. After sendConnectionAttemptCount number of attempts, ID_CONNECTION_ATTEMPT_FAILED is returned. Under low bandwidth conditions with multiple simultaneous outgoing connections, this value should be raised to 1000 or higher, or else the MTU detection can overrun the available bandwidth.
	 * 参数[输入] timeoutTime How long to keep the connection alive before dropping it on unable to send a reliable message. 0 to use the default from SetTimeoutTime(UNASSIGNED_SYSTEM_ADDRESS);
	 * 返回值: ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED on successful initiation. Otherwise, an appropriate enumeration indicating failure.
	 * 注意: ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED does not mean you are already connected!
	 */
	virtual ConnectionAttemptResult ConnectWithSocket(const char* host, unsigned short remotePort, const char *passwordData, int passwordDataLength, RakNetSocket2* socket, PublicKey *publicKey=0, unsigned sendConnectionAttemptCount=12, unsigned timeBetweenSendConnectionAttemptsMS=500, RakNet::TimeMS timeoutTime=0)=0;

	/*
	 * Connect to the specified network ID (Platform specific console function)
	 * Does built-in NAt traversal
	 * 参数[输入] passwordData A data block that must match the data block on the server passed to SetIncomingPassword.  This can be a string or can be a stream of data.  Use 0 for no password.
	 * 参数[输入] passwordDataLength The length in bytes of passwordData
	 */
	/* virtual bool Console2LobbyConnect( void *networkServiceId, const char *passwordData, int passwordDataLength )=0; */

	/*
	 * Stops the network threads and closes all connections.
	 * 参数[输入] blockDuration How long, in milliseconds, you should wait for all remaining messages to go out, including ID_DISCONNECTION_NOTIFICATION.  If 0, it doesn't wait at all.
	 * 参数[输入] orderingChannel If blockDuration > 0, ID_DISCONNECTION_NOTIFICATION will be sent on this channel
	 * 参数[输入] disconnectionNotificationPriority Priority to send ID_DISCONNECTION_NOTIFICATION on.
	 * If you set it to 0 then the disconnection notification won't be sent
	 */
	virtual void Shutdown( unsigned int blockDuration, unsigned char orderingChannel=0, PacketPriority disconnectionNotificationPriority=LOW_PRIORITY )=0;

	/*
	 * Returns if the network thread is running
	 * 返回值: true if the network thread is running, false otherwise
	 */
	virtual bool IsActive( void ) const=0;

	/*
	 * Fills the array remoteSystems with the SystemAddress of all the systems we are connected to
	 * 参数[输出] remoteSystems An array of SystemAddress structures to be filled with the SystemAddresss of the systems we are connected to. Pass 0 to remoteSystems to only get the number of systems we are connected to
	 * \param[in, out] numberOfSystems As input, the size of remoteSystems array.  As output, the number of elements put into the array
	 */
	virtual bool GetConnectionList( SystemAddress *remoteSystems, unsigned short *numberOfSystems ) const=0;

	/*
	 * 返回 next uint32_t that Send() will return
	 * 注意: If using RakPeer from multiple threads, this may not be accurate for your thread. Use IncrementNextSendReceipt() in that case.
	 * 返回值: The next uint32_t that Send() or SendList will return
	 */
	virtual uint32_t GetNextSendReceipt()=0;

	/*
	 * 返回 next uint32_t that Send() will return, and increments the value by one
	 * 注意: If using RakPeer from multiple threads, pass this to forceReceipt in the send function
	 * 返回值: The next uint32_t that Send() or SendList will return
	 */
	virtual uint32_t IncrementNextSendReceipt()=0;

	/*
	 * 向你已连接的指定系统发送一个数据块。
	 * This function only works while connected
	 * The first byte should be a message identifier starting at ID_USER_PACKET_ENUM
	 * 参数[输入] data The block of data to send
	 * 参数[输入] length The size in bytes of the data to send
	 * 参数[输入] priority What priority level to send on.  See PacketPriority.h
	 * 参数[输入] reliability How reliability to send this data.  See PacketPriority.h
	 * 参数[输入] orderingChannel When using ordered or sequenced messages, what channel to order these on. Messages are only ordered relative to other messages on the same stream
	 * 参数[输入] systemIdentifier Who to send this packet to, or in the case of broadcasting who not to send it to.  Pass either a SystemAddress structure or a RakNetGUID structure. Use UNASSIGNED_SYSTEM_ADDRESS or to specify 无
	 * 参数[输入] broadcast True to send this packet to all connected systems. If true, then systemAddress specifies who not to send the packet to.
	 * 参数[输入] forceReceipt If 0, will automatically determine the receipt number to return. If non-zero, will return what you give it.
	 * 返回值: 0 on bad input. Otherwise a number that identifies this message. If reliability is a type that returns a receipt, on a later call to Receive() you will get ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS with bytes 1-4 inclusive containing this number
	 */
	virtual uint32_t Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, uint32_t forceReceiptNumber=0 )=0;

	/*
	 * "Send" to yourself rather than a remote system. The message will be processed through the plugins and returned to the game as usual
	 * This function works anytime
	 * The first byte should be a message identifier starting at ID_USER_PACKET_ENUM
	 * 参数[输入] data The block of data to send
	 * 参数[输入] length The size in bytes of the data to send
	 */
	virtual void SendLoopback( const char *data, const int length )=0;

	/*
	 * Sends a block of data to the specified system that you are connected to.  Same as the above version, but takes a BitStream as input.
	 * 参数[输入] bitStream The bitstream to send
	 * 参数[输入] priority What priority level to send on.  See PacketPriority.h
	 * 参数[输入] reliability How reliability to send this data.  See PacketPriority.h
	 * 参数[输入] orderingChannel When using ordered or sequenced messages, what channel to order these on. Messages are only ordered relative to other messages on the same stream
	 * 参数[输入] systemIdentifier Who to send this packet to, or in the case of broadcasting who not to send it to. Pass either a SystemAddress structure or a RakNetGUID structure. Use UNASSIGNED_SYSTEM_ADDRESS or to specify 无
	 * 参数[输入] broadcast True to send this packet to all connected systems. If true, then systemAddress specifies who not to send the packet to.
	 * 参数[输入] forceReceipt If 0, will automatically determine the receipt number to return. If non-zero, will return what you give it.
	 * 返回值: 0 on bad input. Otherwise a number that identifies this message. If reliability is a type that returns a receipt, on a later call to Receive() you will get ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS with bytes 1-4 inclusive containing this number
	 * 注意: COMMON MISTAKE: When writing the first byte, bitStream->Write((unsigned char) ID_MY_TYPE) be sure it is casted to a byte, and you are not writing a 4 byte enumeration.
	 */
	virtual uint32_t Send( const RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, uint32_t forceReceiptNumber=0 )=0;

	/*
	 * Sends multiple blocks of data, concatenating them automatically.
	 *
	 * 这是equivalent to:
	 * RakNet::BitStream bs;
	 * bs.WriteAlignedBytes(block1, blockLength1);
	 * bs.WriteAlignedBytes(block2, blockLength2);
	 * bs.WriteAlignedBytes(block3, blockLength3);
	 * Send(&bs, ...)
	 *
	 * This function only works while connected
	 * 参数[输入] data An array of pointers to blocks of data
	 * 参数[输入] lengths An array of integers indicating the length of each block of data
	 * 参数[输入] numParameters Length of the arrays data and lengths
	 * 参数[输入] priority What priority level to send on.  See PacketPriority.h
	 * 参数[输入] reliability How reliability to send this data.  See PacketPriority.h
	 * 参数[输入] orderingChannel When using ordered or sequenced messages, what channel to order these on. Messages are only ordered relative to other messages on the same stream
	 * 参数[输入] systemIdentifier Who to send this packet to, or in the case of broadcasting who not to send it to. Pass either a SystemAddress structure or a RakNetGUID structure. Use UNASSIGNED_SYSTEM_ADDRESS or to specify 无
	 * 参数[输入] broadcast True to send this packet to all connected systems. If true, then systemAddress specifies who not to send the packet to.
	 * 参数[输入] forceReceipt If 0, will automatically determine the receipt number to return. If non-zero, will return what you give it.
	 * 返回值: 0 on bad input. Otherwise a number that identifies this message. If reliability is a type that returns a receipt, on a later call to Receive() you will get ID_SND_RECEIPT_ACKED or ID_SND_RECEIPT_LOSS with bytes 1-4 inclusive containing this number
	 */
	virtual uint32_t SendList( const char **data, const int *lengths, const int numParameters, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, uint32_t forceReceiptNumber=0 )=0;

	/*
	 * 从传入消息队列中获取一条消息。
	 * Use DeallocatePacket() to deallocate the message after you are done with it.
	 * User-thread functions, such as RPC calls and the plugin function PluginInterface::Update occur here.
	 * 返回值: 0 if no packets are waiting to be handled, otherwise a pointer to a packet.
	 * 注意: COMMON MISTAKE: Be sure to call this in a loop, once per game tick, until it returns 0. If you only process one packet per game tick they will buffer up.
	 * sa RakNetTypes.h contains struct Packet
	 */
	virtual Packet* Receive( void )=0;

	/*
	 * Call this to deallocate a message returned by Receive() when you are done handling it.
	 * 参数[输入] packet The message to deallocate.
	 */
	virtual void DeallocatePacket( Packet *packet )=0;

	/* 返回允许的总连接数 */
	virtual unsigned int GetMaximumNumberOfPeers( void ) const=0;

	/* -------------------------------------------------------------------------------------------- Connection Management Functions-------------------------------------------------------------------------------------------- */
	/*
	 * Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
	 * 参数[输入] target Which system to close the connection to.
	 * 参数[输入] sendDisconnectionNotification True to send ID_DISCONNECTION_NOTIFICATION to the recipient.  False to close it silently.
	 * 参数[输入] channel Which ordering channel to send the disconnection notification on, if any
	 * 参数[输入] disconnectionNotificationPriority Priority to send ID_DISCONNECTION_NOTIFICATION on.
	 */
	virtual void CloseConnection( const AddressOrGUID target, bool sendDisconnectionNotification, unsigned char orderingChannel=0, PacketPriority disconnectionNotificationPriority=LOW_PRIORITY )=0;

	/*
	 * 返回系统是已连接、已断开、正在连接中还是其他各种状态
	 * 参数[输入] systemIdentifier The system we are referring to
	 * 注意: This locks a mutex, do not call too frequently during connection attempts or the attempt will take longer and possibly even timeout
	 * 返回值: What state the remote system is in
	 */
	virtual ConnectionState GetConnectionState(const AddressOrGUID systemIdentifier)=0;

	/*
	 * Cancel a pending connection attempt
	 * If we are already connected, the connection stays open
	 * 参数[输入] target Which system to cancel
	 */
	virtual void CancelConnectionAttempt( const SystemAddress target )=0;

	/*
	 * Given a systemAddress, returns an index from 0 to the maximum number of players allowed - 1.
	 * 参数[输入] systemAddress The SystemAddress we are referring to
	 * 返回值: The index of this SystemAddress or -1 on system not found.
	 */
	virtual int GetIndexFromSystemAddress( const SystemAddress systemAddress ) const=0;

	/*
	 * This function is only useful for looping through all systems
	 * Given an index, will return a SystemAddress.
	 * 参数[输入] index Index should range between 0 and the maximum number of players allowed - 1.
	 * 返回值: The SystemAddress
	 */
	virtual SystemAddress GetSystemAddressFromIndex( unsigned int index )=0;

	/*
	 * 与 GetSystemAddressFromIndex but returns RakNetGUID 相同
	 * 参数[输入] index Index should range between 0 and the maximum number of players allowed - 1.
	 * 返回值: The RakNetGUID
	 */
	virtual RakNetGUID GetGUIDFromIndex( unsigned int index )=0;

	/*
	 * 与 calling GetSystemAddressFromIndex and GetGUIDFromIndex for all systems, but more efficient 相同
	 * Indices match each other, so addresses[0] and guids[0] refer to the same system
	 * 参数[输出] addresses All system addresses. Size of the list is the number of connections. Size of the list will match the size of the guids list.
	 * 参数[输出] guids All guids. Size of the list is the number of connections. Size of the list will match the size of the addresses list.
	 */
	virtual void GetSystemList(DataStructures::List<SystemAddress> &addresses, DataStructures::List<RakNetGUID> &guids) const=0;

	/*
	 * Bans an IP from connecting.  Banned IPs persist between connections but are not saved on shutdown nor loaded on startup.
	 * param[in] IP Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will ban all IP addresses starting with 128.0.0
	 * 参数[输入] milliseconds how many ms for a temporary ban.  Use 0 for a permanent ban
	 */
	virtual void AddToBanList( const char *IP, RakNet::TimeMS milliseconds=0 )=0;

	/*
	 * 允许之前被禁止的 IP 重新连接。
	 * param[in] Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will banAll IP addresses starting with 128.0.0
	 */
	virtual void RemoveFromBanList( const char *IP )=0;

	/* 允许所有之前被禁止的 IP 重新连接。*/
	virtual void ClearBanList( void )=0;

	/*
	 * 返回 true 或 false 指示特定 IP 是否被禁止。
	 * 参数[输入] IP - Dotted IP address.
	 * 返回值: true if IP matches any IPs in the ban list, accounting for any wildcards. False otherwise.
	 */
	virtual bool IsBanned( const char *IP )=0;

	/*
	 * Enable or disable allowing frequent connections from the same IP adderss
	 * This is a security measure which is disabled by default, but can be set to true to prevent attackers from using up all connection slots
	 * 参数[输入] b True to limit connections from the same ip to at most 1 per 100 milliseconds.
	 */
	virtual void SetLimitIPConnectionFrequency(bool b)=0;

	/* --------------------------------------------------------------------------------------------Pinging Functions - Functions dealing with the automatic ping mechanism-------------------------------------------------------------------------------------------- */
	/*
	 * Send a ping to the specified connected system.
	 * 前提条件: The sender and recipient must already be started via a successful call to Startup()
	 * 参数[输入] target Which system to ping
	 */
	virtual void Ping( const SystemAddress target )=0;

	/*
	 * Send a ping to the specified unconnected system. The remote system, if it is Initialized, will respond with ID_PONG followed by sizeof(RakNet::TimeMS) containing the system time the ping was sent.(Default is 4 bytes - See __GET_TIME_64BIT in RakNetTypes.h
	 * System should reply with ID_PONG if it is active
	 * 参数[输入] host Either a dotted IP address or a domain name.  Can be 255.255.255.255 for LAN broadcast.
	 * 参数[输入] remotePort Which port to connect to on the remote machine.
	 * 参数[输入] onlyReplyOnAcceptingConnections Only request a reply if the remote system is accepting connections
	 * 参数[输入] connectionSocketIndex Index into the array of socket descriptors passed to socketDescriptors in RakPeer::Startup() to send on.
	 * 返回值: 成功返回 true，失败返回 false。 (unknown hostname)
	 */
	virtual bool Ping( const char* host, unsigned short remotePort, bool onlyReplyOnAcceptingConnections, unsigned connectionSocketIndex=0 )=0;

	/*
	 * Returns the average of all ping times read for the specific system or -1 if 无 read yet
	 * 参数[输入] systemAddress Which system we are referring to
	 * 返回值: The ping time for this system, or -1
	 */
	virtual int GetAveragePing( const AddressOrGUID systemIdentifier )=0;

	/*
	 * Returns the last ping time read for the specific system or -1 if 无 read yet
	 * 参数[输入] systemAddress Which system we are referring to
	 * 返回值: The last ping time for this system, or -1
	 */
	virtual int GetLastPing( const AddressOrGUID systemIdentifier ) const=0;

	/*
	 * Returns the lowest ping time read or -1 if 无 read yet
	 * 参数[输入] systemAddress Which system we are referring to
	 * 返回值: The lowest ping time for this system, or -1
	 */
	virtual int GetLowestPing( const AddressOrGUID systemIdentifier ) const=0;

	/*
	 * Ping the remote systems every so often, or not. Can be called anytime.
	 * By default this is true. Recommended to leave on, because congestion control uses it to determine how often to resend lost packets.
	 * It would be true by default to prevent timestamp drift, since in the event of a clock spike, the timestamp deltas would no longer be accurate
	 * 参数[输入] doPing True to start occasional pings.  False to stop them.
	 */
	virtual void SetOccasionalPing( bool doPing )=0;

	/*
	 * 返回 clock difference between your system and the specified system
	 * Subtract GetClockDifferential() from a time returned by the remote system to get that time relative to your own system
	 * 返回 0 if the system is unknown
	 * 参数[输入] systemIdentifier Which system we are referring to
	 */
	virtual RakNet::Time GetClockDifferential( const AddressOrGUID systemIdentifier )=0;

	/* --------------------------------------------------------------------------------------------Static Data Functions - Functions dealing with API defined synchronized memory-------------------------------------------------------------------------------------------- */
	/*
	 * 将data设置为send along with a LAN server discovery or offline ping reply
	 * length should be under 400 bytes, as a security measure against flood attacks
	 * 参数[输入] data a block of data to store, or 0 for 无
	 * 参数[输入] length The length of data in bytes, or 0 for 无
	 * 另见 Ping.cpp
	 */
	virtual void SetOfflinePingResponse( const char *data, const unsigned int length )=0;

	/*
	 * Returns pointers to a copy of the data passed to SetOfflinePingResponse
	 * 参数[输出] data A pointer to a copy of the data passed to SetOfflinePingResponse()
	 * 参数[输出] length A pointer filled in with the length parameter passed to SetOfflinePingResponse()
	 * 另见 SetOfflinePingResponse
	 */
	virtual void GetOfflinePingResponse( char **data, unsigned int *length )=0;

	/* --------------------------------------------------------------------------------------------Network Functions - Functions dealing with the network in general-------------------------------------------------------------------------------------------- */
	/*
	 * 返回 unique address identifier that represents you or another system on the the network and is based on your local IP / port
	 * 注意: Not supported by the XBOX
	 * 参数[输入] systemAddress Use UNASSIGNED_SYSTEM_ADDRESS to get your behind-LAN address. Use a connected system to get their behind-LAN address
	 * 参数[输入] index When you have multiple 内部使用 IDs, which index to return? Currently limited to MAXIMUM_NUMBER_OF_INTERNAL_IDS (so the maximum value of this variable is MAXIMUM_NUMBER_OF_INTERNAL_IDS-1)
	 * 返回值: the identifier of your system internally, which may not be how other systems see if you if you are behind a NAT or proxy
	 */
	virtual SystemAddress GetInternalID( const SystemAddress systemAddress=UNASSIGNED_SYSTEM_ADDRESS, const int index=0 ) const=0;

	/*
	 * Sets your 内部使用 IP address, for platforms that do not support reading it, or to override a value
	 * 参数[输入] systemAddress. The address to set. Use SystemAddress::FromString() if you want to use a dotted string
	 * 参数[输入] index When you have multiple 内部使用 IDs, which index to set?
	 */
	virtual void SetInternalID(SystemAddress systemAddress, int index=0)=0;

	/*
	 * 返回 unique address identifier that represents you on the the network and is based on your externalIP / port
	 * (the IP / port the specified player uses to communicate with you)
	 * 参数[输入] target Which remote system you are referring to for your 外部 ID.  Usually the same for all systems, unless you have two or more network cards.
	 */
	virtual SystemAddress GetExternalID( const SystemAddress target ) const=0;

	/* 返回 my own GUID */
	virtual const RakNetGUID GetMyGUID(void) const=0;

	/* 返回 address bound to a socket at the specified index */
	virtual SystemAddress GetMyBoundAddress(const int socketIndex=0)=0;

	/* 获取 a random number (to generate a GUID)*/
	static uint64_t Get64BitUniqueRandomNumber();

	/*
	 * Given a connected system, give us the unique GUID representing that instance of RakPeer.
	 * This will be the same on all systems connected to that instance of RakPeer, even if the 外部 system addresses are different
	 * Currently O(log(n)), but this may be improved in the future. If you use this frequently, you may want to cache the value as it won't change.
	 * Returns UNASSIGNED_RAKNET_GUID if system address can't be found.
	 * If input is UNASSIGNED_SYSTEM_ADDRESS, will return your own GUID
	 * 前提条件: Call Startup() first, or the function will return UNASSIGNED_RAKNET_GUID
	 * 参数[输入] input The system address of the system we are connected to
	 */
	virtual const RakNetGUID& GetGuidFromSystemAddress( const SystemAddress input ) const=0;

	/*
	 * Given the GUID of a connected system, give us the system address of that system.
	 * The GUID will be the same on all systems connected to that instance of RakPeer, even if the 外部 system addresses are different
	 * Currently O(log(n)), but this may be improved in the future. If you use this frequently, you may want to cache the value as it won't change.
	 * If input is UNASSIGNED_RAKNET_GUID, will return UNASSIGNED_SYSTEM_ADDRESS
	 * 参数[输入] input The RakNetGUID of the system we are checking to see if we are connected to
	 */
	virtual SystemAddress GetSystemAddressFromGuid( const RakNetGUID input ) const=0;

	/*
	 * Given the SystemAddress of a connected system, get the public key they provided as an identity
	 * 如果未找到系统地址或客户端公钥未知则返回 false
	 * 参数[输入] input The RakNetGUID of the system
	 * 参数[输入] client_public_key The connected client's public key is copied to this address.  Buffer must be cat::EasyHandshake::PUBLIC_KEY_BYTES bytes in length.
	 */
	virtual bool GetClientPublicKeyFromSystemAddress( const SystemAddress input, char *client_public_key ) const=0;

	/*
	 * 设置在无法投递可靠消息后、认定自身已断开连接之前的等待时间（毫秒）。
	 * 默认时间为 Release 模式下 10,000 毫秒（10 秒），Debug 模式下 30,000 毫秒（30 秒）。
	 * Do not set different values for different computers that are connected to each other, or you won't be able to reconnect after ID_CONNECTION_LOST
	 * 参数[输入] timeMS Time, in MS
	 * 参数[输入] target Which system to do this for. Pass UNASSIGNED_SYSTEM_ADDRESS for all systems.
	 */
	virtual void SetTimeoutTime( RakNet::TimeMS timeMS, const SystemAddress target )=0;

	/*
	 * 参数[输入] target Which system to do this for. Pass UNASSIGNED_SYSTEM_ADDRESS to get the default value
	 * 返回值: timeoutTime for a given system.
	 */
	virtual RakNet::TimeMS GetTimeoutTime( const SystemAddress target )=0;

	/*
	 * 返回当前 MTU 大小
	 * 参数[输入] target Which system to get this for.  UNASSIGNED_SYSTEM_ADDRESS to get the default
	 * 返回值: The current MTU size
	 */
	virtual int GetMTUSize( const SystemAddress target ) const=0;

	/* 返回 number of IP addresses this system has internally. Get the actual addresses from GetLocalIP()*/
	virtual unsigned GetNumberOfAddresses( void )=0;

	/*
	 * Returns an IP address at index 0 to GetNumberOfAddresses-1
	 * 参数[输入] index index into the list of IP addresses
	 * 返回值: The local IP address at this index
	 */
	virtual const char* GetLocalIP( unsigned int index )=0;

	/*
	 * Is this a local IP?
	 * 参数[输入] An IP address to check, excluding the port
	 * 返回值: True if this is one of the IP addresses returned by GetLocalIP
	 */
	virtual bool IsLocalIP( const char *ip )=0;

	/*
	 * Allow or disallow connection responses from any IP. Normally this should be false, but may be necessary
	 * when connecting to servers with multiple IP addresses.
	 * 参数[输入] allow - True to allow this behavior, false to not allow. Defaults to false. Value persists between connections
	 */
	virtual void AllowConnectionResponseIPMigration( bool allow )=0;

	/*
	 * 向远程未连接系统发送一个字节的 ID_ADVERTISE_SYSTEM 消息。
	 * This will tell the remote system our 外部 IP outside the LAN along with some user data.
	 * 前提条件: The sender and recipient must already be started via a successful call to Initialize
	 * 参数[输入] host Either a dotted IP address or a domain name
	 * 参数[输入] remotePort Which port to connect to on the remote machine.
	 * 参数[输入] data 可选 data to append to the packet.
	 * 参数[输入] dataLength length of data in bytes.  Use 0 if no data.
	 * 参数[输入] connectionSocketIndex Index into the array of socket descriptors passed to socketDescriptors in RakPeer::Startup() to send on.
	 * 返回值: false if IsActive()==false or the host is unresolvable. True otherwise
	 */
	virtual bool AdvertiseSystem( const char *host, unsigned short remotePort, const char *data, int dataLength, unsigned connectionSocketIndex=0 )=0;

	/*
	 * 控制大型消息下载时返回 ID_DOWNLOAD_PROGRESS 的频率。
	 * ID_DOWNLOAD_PROGRESS is returned to indicate a new partial message chunk, roughly the MTU size, has arrived
	 * As it can be slow or cumbersome to get this notification for every chunk, you can set the interval at which it is returned.
	 * Defaults to 0 (never return this notification)
	 * 参数[输入] interval How many messages to use as an interval
	 */
	virtual void SetSplitMessageProgressInterval(int interval)=0;

	/*
	 * Returns what was passed to SetSplitMessageProgressInterval()
	 * 返回值: What was passed to SetSplitMessageProgressInterval(). Default to 0.
	 */
	virtual int GetSplitMessageProgressInterval(void) const=0;

	/*
	 * 将how long设置为wait before giving up on sending an unreliable message
	 * Useful if the network is clogged up.
	 * 将to 0 or less设置为never timeout.  Defaults to 0
	 * 参数[输入] timeoutMS How many ms to wait before simply not sending an unreliable message.
	 */
	virtual void SetUnreliableTimeout(RakNet::TimeMS timeoutMS)=0;

	/*
	 * Send a message to host, with the IP socket option TTL set to 3
	 * This message will not reach the host, but will open the router.
	 * 用于NAT-Punchthrough
	 */
	virtual void SendTTL( const char* host, unsigned short remotePort, int ttl, unsigned connectionSocketIndex=0 )=0;

	/* -------------------------------------------------------------------------------------------- Plugin Functions-------------------------------------------------------------------------------------------- */
	/*
	 * Attaches a Plugin interface to an instance of the base class (RakPeer or PacketizedTCP) to run code automatically on message receipt in the Receive call.
	 * 如果插件从 PluginInterface::UsesReliabilityLayer() 返回 false（除 PacketLogger 外的所有插件都是如此），则可以在 RakPeer 活跃时对该插件调用 AttachPlugin() 和 DetachPlugin()。
	 * 参数[输入] messageHandler Pointer to the plugin to attach.
	 */
	virtual void AttachPlugin( PluginInterface2 *plugin )=0;

	/*
	 * Detaches a Plugin interface from the instance of the base class (RakPeer or PacketizedTCP) it is attached to.
	 * This method disables the plugin code from running automatically on base class's updates or message receipt.
	 * 如果插件从 PluginInterface::UsesReliabilityLayer() 返回 false（除 PacketLogger 外的所有插件都是如此），则可以在 RakPeer 活跃时对该插件调用 AttachPlugin() 和 DetachPlugin()。
	 * 参数[输入] messageHandler Pointer to a plugin to detach.
	 */
	virtual void DetachPlugin( PluginInterface2 *messageHandler )=0;

	/* --------------------------------------------------------------------------------------------Miscellaneous Functions-------------------------------------------------------------------------------------------- */
	/*
	 * Put a message back at the end of the receive queue in case you don't want to deal with it immediately
	 * 参数[输入] packet The packet you want to push back.
	 * 参数[输入] pushAtHead True to push the packet so that the next receive call returns it.  False to push it at the end of the queue (obviously pushing it at the end makes the packets out of order)
	 */
	virtual void PushBackPacket( Packet *packet, bool pushAtHead )=0;

	/*
	 * 内部使用
	 * For a given system identified by guid, change the SystemAddress to send to.
	 * 参数[输入] guid The connection we are referring to
	 * 参数[输入] systemAddress The new address to send to
	 */
	virtual void ChangeSystemAddress(RakNetGUID guid, const SystemAddress &systemAddress)=0;

	/*
	 * 返回值: a packet for you to write to if you want to create a Packet for some reason.
	 * You can add it to the receive buffer with PushBackPacket
	 * 参数[输入] dataSize How many bytes to allocate for the buffer
	 * 返回值: A packet you can write to
	 */
	virtual Packet* AllocatePacket(unsigned dataSize)=0;

	/*
	 * 获取 socket used with a particular active connection
	 * The smart pointer reference counts the RakNetSocket2 object, so the socket will remain active as long as the smart pointer does, even if RakNet were to shutdown or close the connection.
	 * 注意: This sends a query to the thread and blocks on the return value for up to one second. In practice it should only take a millisecond or so.
	 * 参数[输入] target Which system
	 * 返回值: A smart pointer object containing the socket information about the socket. Be sure to check IsNull() which is returned if the update thread is unresponsive, shutting down, or if this system is not connected
	 */
	virtual RakNetSocket2* GetSocket( const SystemAddress target )=0;

	/*
	 * 获取 all sockets in use
	 * 注意: This sends a query to the thread and blocks on the return value for up to one second. In practice it should only take a millisecond or so.
	 * 参数[输出] sockets List of RakNetSocket2 structures in use. Sockets will not be closed until sockets goes out of scope
	 */
	virtual void GetSockets( DataStructures::List<RakNetSocket2* > &sockets )=0;
	virtual void ReleaseSockets( DataStructures::List<RakNetSocket2* > &sockets )=0;

	virtual void WriteOutOfBandHeader(RakNet::BitStream *bitStream)=0;

	/*
	 * If you need code to run in the same thread as RakNet's update thread, this function can be used for that
	 * 参数[输入] _userUpdateThreadPtr C callback function
	 * 参数[输入] _userUpdateThreadData Passed to C callback function
	 */
	virtual void SetUserUpdateThread(void (*_userUpdateThreadPtr)(RakPeerInterface *, void *), void *_userUpdateThreadData)=0;

	/*
	 * 将a C callback设置为be called whenever a datagram arrives
	 * 从回调中返回 true 让 RakPeer 处理数据报。返回 false 则 RakPeer 将忽略该数据报。
	 * This can be used to filter incoming datagrams by system, or to share a recvfrom socket with RakPeer
	 * RNS2RecvStruct will only remain valid for the duration of the call
	 * If the incoming datagram is not from your game at all, it is a RakNet packet.
	 * If the incoming datagram has an IP address that matches a known address from your game, then check the first byte of data.
	 * For RakNet connected systems, the first bit is always 1. So for your own game packets, make sure the first bit is always 0.
	 */
	virtual void SetIncomingDatagramEventHandler( bool (*_incomingDatagramEventHandler)(RNS2RecvStruct *) )=0;

	/* --------------------------------------------------------------------------------------------Network Simulator Functions-------------------------------------------------------------------------------------------- */
	/*
	 * 为传出数据流添加模拟的 ping 和丢包。
	 * To simulate bi-directional ping and packet loss, you should call this on both the sender and the recipient, with half the total ping and packetloss value on each.
	 * You can exclude network simulator code with the _RELEASE #define to decrease code size
	 * 已废弃 Use http://www.jenkinssoftware.com/forum/index.php?topic=1671.0 instead.
	 * 注意: Doesn't work past version 3.6201
	 * 参数[输入] packetloss Chance to lose a packet. Ranges from 0 to 1.
	 * 参数[输入] minExtraPing The minimum time to delay sends.
	 * 参数[输入] extraPingVariance The additional random time to delay sends.
	 */
	virtual void ApplyNetworkSimulator( float packetloss, unsigned short minExtraPing, unsigned short extraPingVariance)=0;

	/*
	 * Limits how much outgoing bandwidth can be sent per-connection.
	 * This limit does not apply to the sum of all connections!
	 * Exceeding the limit queues up outgoing traffic
	 * 参数[输入] maxBitsPerSecond Maximum bits 每秒 to send.  Use 0 for unlimited (default). Once set, it takes effect immedately and persists until called again.
	 */
	virtual void SetPerConnectionOutgoingBandwidthLimit( unsigned maxBitsPerSecond )=0;

	/*
	 * 返回你之前是否调用过 ApplyNetworkSimulator
	 * 返回值: If you previously called ApplyNetworkSimulator
	 */
	virtual bool IsNetworkSimulatorActive( void )=0;

	/* --------------------------------------------------------------------------------------------Statistical Functions - Functions dealing with API performance-------------------------------------------------------------------------------------------- */

	/*
	 * Returns a structure containing a large set of network statistics for the specified system.
	 * You can map this data to a string using the C style StatisticsToString() function
	 * 参数[输入] systemAddress: Which connected system to get statistics for
	 * 参数[输入] rns If you supply this structure, it will be written to it.  Otherwise it will use a static struct, which is 非线程安全
	 * 返回值: 0 on can't find the specified system.  A pointer to a set of data otherwise.
	 * 另见 RakNetStatistics.h
	 */
	virtual RakNetStatistics * GetStatistics( const SystemAddress systemAddress, RakNetStatistics *rns=0 )=0;
	/*
	 * 返回 network statistics of the system at the given index in the remoteSystemList
	 * 返回值: True if the index is less than the maximum number of peers allowed and the system is active. False otherwise.
	 */
	virtual bool GetStatistics( const unsigned int index, RakNetStatistics *rns )=0;
	/*
	 * 返回 list of systems, and statistics for each of those systems
	 * Each system has one entry in each of the lists, in the same order
	 * 参数[输出] addresses SystemAddress for each connected system
	 * 参数[输出] guids RakNetGUID for each connected system
	 * 参数[输出] statistics Calculated RakNetStatistics for each connected system
	 */
	virtual void GetStatisticsList(DataStructures::List<SystemAddress> &addresses, DataStructures::List<RakNetGUID> &guids, DataStructures::List<RakNetStatistics> &statistics)=0;

	/* 返回值: how many messages are waiting when you call Receive() */
	virtual unsigned int GetReceiveBufferSize()=0;

	/* --------------------------------------------------------------------------------------------EVERYTHING AFTER THIS COMMENT IS FOR 内部使用 USE ONLY-------------------------------------------------------------------------------------------- */
	
	/* 内部使用 */
	/* 如果 RAKPEER_USER_THREADED==1，请至少每 30 毫秒手动调用一次。*/
	/* updateBitStream should be: */
	/* 	BitStream updateBitStream( MAXIMUM_MTU_SIZE */
	/* #if LIBCAT_SECURITY==1 */
	/* + cat::AuthenticatedEncryption::OVERHEAD_BYTES */
	/* #endif */
	/* ); */
	virtual bool RunUpdateCycle( BitStream &updateBitStream )=0;

	/* 内部使用 */
	virtual bool SendOutOfBand(const char *host, unsigned short remotePort, const char *data, BitSize_t dataLength, unsigned connectionSocketIndex=0 )=0;

}
/* #if defined(SN_TARGET_PSP2) */
/* __attribute__((aligned(8))) */
/* #endif */
;

} /* RakNet 命名空间 */
