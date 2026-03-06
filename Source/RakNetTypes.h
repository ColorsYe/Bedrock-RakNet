/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/* 文件说明: RakNet 使用的类型定义，大部分与用户代码相关。 */


#pragma once
#include "RakNetDefines.h"
#include "NativeTypes.h"
#include "RakNetTime.h"
#include "Export.h"
#include "WindowsIncludes.h"
#include "SocketIncludes.h"




namespace RakNet {
/* 前向声明 */
class RakPeerInterface;
class BitStream;
struct Packet;

enum class StartupResult
{
	RAKNET_STARTED,
	RAKNET_ALREADY_STARTED,
	INVALID_SOCKET_DESCRIPTORS,
	INVALID_MAX_CONNECTIONS,
	SOCKET_FAMILY_NOT_SUPPORTED,
	SOCKET_PORT_ALREADY_IN_USE,
	SOCKET_FAILED_TO_BIND,
	SOCKET_FAILED_TEST_SEND,
	PORT_CANNOT_BE_ZERO,
	FAILED_TO_CREATE_NETWORK_THREAD,
	COULD_NOT_GENERATE_GUID,
	STARTUP_OTHER_FAILURE
};


enum class ConnectionAttemptResult
{
	CONNECTION_ATTEMPT_STARTED,
	INVALID_PARAMETER,
	CANNOT_RESOLVE_DOMAIN_NAME,
	ALREADY_CONNECTED_TO_ENDPOINT,
	CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
	SECURITY_INITIALIZATION_FAILED
};

/* 由 RakPeerInterface::GetConnectionState() 返回的连接状态枚举 */
enum ConnectionState
{
	/* 已调用 Connect()，但处理流程尚未开始 */
	IS_PENDING,
	/* 正在处理连接尝试 */
	IS_CONNECTING,
	/* 已连接，可以正常通信 */
	IS_CONNECTED,
	/* 已连接，但将在剩余消息传递完毕后断开连接 */
	IS_DISCONNECTING,
	/* 连接尝试失败，即将中止 */
	IS_SILENTLY_DISCONNECTING,
	/* 连接已断开 */
	IS_DISCONNECTED,
	/* 从未建立过连接，或断开连接已久条目已被丢弃 */
	IS_NOT_CONNECTED
};

/* 给定比特数，返回表示该比特数所需的字节数。 */
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

/* 另见 NetworkIDObject.h */
using UniqueIDType = unsigned char;
using SystemIndex = unsigned short;
using RPCIndex = unsigned char;
const int MAX_RPC_MAP_SIZE=((RPCIndex)-1)-1;
const int UNDEFINED_RPC_INDEX=((RPCIndex)-1);

/* 网络消息的第一个字节（消息类型标识符） */
using MessageID = unsigned char;

using BitSize_t = uint32_t;

#if defined(_MSC_VER) && _MSC_VER > 0
#define PRINTF_64_BIT_MODIFIER "I64"
#else
#define PRINTF_64_BIT_MODIFIER "ll"
#endif

/* 与 PublicKey 结构体配合使用的公钥模式枚举 */
enum PublicKeyMode
{
	/* 连接不使用加密。也可以在 RakPeerInterface::Connect() 中将 PublicKey 指针直接传 0。 */
	PKM_INSECURE_CONNECTION,

	/*
	 * 接受服务器提供的任意公钥。此模式容易受到中间人攻击，
	 * 但无需在连接前预先分发公钥。
	 */
	PKM_ACCEPT_ANY_PUBLIC_KEY,

	/*
	 * 使用已知的远程服务器公钥。PublicKey::remoteServerPublicKey 必须非零。
	 * 这是安全连接的推荐模式。
	 */
	PKM_USE_KNOWN_PUBLIC_KEY,

	/*
	 * 同时使用已知的远程服务器公钥，并为连接客户端提供自身的公钥。
	 * PublicKey::remoteServerPublicKey、myPublicKey 和 myPrivateKey 均必须非零。
	 * 服务器必须配合才能使此模式正常工作。
	 * 建议仅用于服务器间通信，因为此模式会显著增加双方连接期间的 CPU 开销。
	 * 此外，使用此模式时应同时设置连接密码，以防范 DoS 攻击。
	 */
	PKM_USE_TWO_WAY_AUTHENTICATION
};

/* 传递给 RakPeerInterface::Connect() 的公钥结构体 */
struct RAK_DLL_EXPORT PublicKey
{
	/* 指定公钥的解释方式，参见上方枚举 */
	PublicKeyMode publicKeyMode;

	/* 指向长度为 cat::EasyHandshake::PUBLIC_KEY_BYTES 的公钥指针。参见加密示例。 */
	char *remoteServerPublicKey;

	/* （可选）指向长度为 cat::EasyHandshake::PUBLIC_KEY_BYTES 的公钥指针 */
	char *myPublicKey;

	/* （可选）指向长度为 cat::EasyHandshake::PRIVATE_KEY_BYTES 的私钥指针 */
	char *myPrivateKey;
};

/* 描述 RakPeer::Startup 使用的本地套接字 */
struct RAK_DLL_EXPORT SocketDescriptor
{
	SocketDescriptor();
	SocketDescriptor(unsigned short _port, const char *_hostAddress);

	/* 要绑定的本地端口。传 0 则由操作系统自动分配端口。 */
	unsigned short port;

	/* 要绑定的本地网卡地址，例如 "127.0.0.1"。传空字符串则使用 INADDR_ANY。 */
	char hostAddress[32];

	/*
	 * IP 版本：IPv4 使用 AF_INET（默认），IPv6 使用 AF_INET6，自动选择使用 AF_UNSPEC。
	 * IPv6 是较新的互联网协议，地址形如 fe80::7c:31f7:fec4:27de%14 而非 natpunch.jenkinssoftware.com。
	 * IPv6 地址编码占 16 字节（IPv4 仅 4 字节），因此带宽效率较低。
	 * 不过 IPv6 的优势在于：地址空间充足，路由器无需创建地址映射，因此无需 NAT 穿透，也不应使用 NAT 穿透。
	 * 若系统不支持所指定的 IP 版本，RakPeer::Startup() 将失败。
	 * 前提条件: 若使用 AF_INET6，需在 RakNetDefines.h 中将 RAKNET_SUPPORT_IPV6 设为 1。
	 */
	short socketFamily;




	unsigned short remotePortRakNetWasStartedOn_PS3_PSP2;

	/* Google Chrome 平台所需 */
	_PP_Instance_ chromeInstance;

	/* 设为 true 使用阻塞套接字（默认，无特殊需求请勿修改） */
	bool blockingSocket;

	/* 仅 XBOX 平台：若需使用 VDP 协议，设置 IPPROTO_VDP。启用后该套接字不支持向 255.255.255.255 广播。 */
	unsigned int extraSocketOptions;
};

extern bool NonNumericHostString( const char *host );

/*
 * 系统的网络地址。
 * 对应一个网络地址，但不一定是唯一标识符。
 * 例如，若某系统同时拥有局域网和互联网连接，
 * 根据通信方的不同，可能通过不同地址标识该系统。
 * 因此，不应将 SystemAddress 通过网络传输后用于标识或连接某系统，
 * 除非该系统不在 NAT 之后（如专用服务器）。
 * 若需要唯一标识每个 RakPeer 实例，请使用 RakNetGUID。
 */
struct RAK_DLL_EXPORT SystemAddress
{
	/* 构造函数 */
	SystemAddress();
	SystemAddress(const char *str);
	SystemAddress(const char *str, unsigned short port);
	SystemAddress(const SystemAddress& other) = default;




	/* 当 RAKNET_SUPPORT_IPV6 已定义时，SystemAddress 同时持有 sockaddr_in6 和 sockaddr_in */
	union/* In6OrIn4 */
	{
#if RAKNET_SUPPORT_IPV6==1
		struct sockaddr_storage sa_stor;
		sockaddr_in6 addr6;
#endif

		sockaddr_in addr4;
	} address;

	/* 内部不使用此字段，但它保存了 address 联合体中的端口副本，便于调试时快速查看当前端口值 */
	unsigned short debugPort;

	/* 内部使用: 返回写入位流时所需的字节大小 */
	static int size();

	/* 对系统地址进行哈希运算 */
	static unsigned long ToInteger( const SystemAddress &sa );

	/*
	 * 返回 IP 版本，IPv4 或 IPv6。
	 * 返回值: 4 或 6
	 */
	unsigned char GetIPVersion(void) const;

	/*
	 * 内部使用: 返回 IPPROTO_IP 或 IPPROTO_IPV6。
	 * 另见 GetIPVersion
	 */
	unsigned int GetIPPROTO(void) const;

	/* 使用当前持有的 IP 版本调用 SetToLoopback()，默认为 IPv4 */
	void SetToLoopback();

	/*
	 * 使用指定 IP 版本调用 SetToLoopback()。
	 * 参数[in] ipVersion: IPv4 传 4，IPv6 传 6
	 */
	void SetToLoopback(unsigned char ipVersion);

	/* 返回值: 是否已设置为 127.0.0.1 或 ::1（回环地址） */
	bool IsLoopback(void) const;

	/*
	 * 以 <IP>|<Port> 格式将系统地址以字符串形式返回。
	 * 返回一个静态字符串（非线程安全）。
	 * portDelineator 不能为 '.'、':'、'%'、'-'、'/'、数字或 a-f。
	 */
	const char *ToString(bool writePort=true, char portDelineator='|') const;

	/*
	 * 以 <IP>|<Port> 格式将系统地址写入字符串。
	 * dest 必须足够大以容纳输出结果（线程安全）。
	 * portDelineator 不能为 '.'、':'、'%'、'-'、'/'、数字或 a-f。
	 */
	void ToString(bool writePort, char *dest, char portDelineator='|') const;

	/*
	 * 从可打印的 IP 字符串中设置系统地址，例如 "192.0.2.1" 或 "2001:db8:63b3:1::3490"。
	 * 也可以通过 portDelineator 同时指定端口，例如 "192.0.2.1|1234"。
	 * 参数[in] str:             可打印的 IP 字符串，传 0 则设置为 UNASSIGNED_SYSTEM_ADDRESS。
	 * 参数[in] portDelineator:  若 str 中包含端口，使用此字符作为分隔符，不能为 '.'、':'、'%'、'-'、'/'、数字或 a-f。
	 * 参数[in] ipVersion:       仅在 str 为预定义地址但格式不匹配时使用，例如希望将 127.0.0.1 转换为 IPv6，可传 6。
	 * 注意: 若 str 中未指定端口，则当前端口保持不变。
	 * 返回值: 成功返回 true，ipVersion 与传入字符串类型不匹配时返回 false。
	 */
	bool FromString(const char *str, char portDelineator='|', int ipVersion=0);

	/* 与 FromString() 相同，但同时显式设置端口 */
	bool FromStringExplicitPort(const char *str, unsigned short port, int ipVersion=0);

	/* 从另一个 SystemAddress 结构体复制端口 */
	void CopyPort( const SystemAddress& right );

	/* 判断两个系统地址的 IP 是否相同（不检查端口） */
	bool EqualsExcludingPort( const SystemAddress& right ) const;

	/* 以主机字节序返回端口（通常使用此函数） */
	unsigned short GetPort(void) const;

	/* 内部使用: 以网络字节序返回端口 */
	unsigned short GetPortNetworkOrder(void) const;

	/*
	 * 设置端口，端口值应为主机字节序（通常使用此函数）。
	 * 因与 winspool.h 冲突而从 SetPort 重命名，参见 http://edn.embarcadero.com/article/21494
	 */
	void SetPortHostOrder(unsigned short s);

	/* 内部使用: 设置端口，端口值应已为网络字节序。 */
	void SetPortNetworkOrder(unsigned short s);

	/* 旧版接口，用于不支持新版套接字函数的平台 */
	bool SetBinaryAddress(const char *str, char portDelineator=':');
	/* 旧版接口，用于不支持新版套接字函数的平台 */
	void ToString_Old(bool writePort, char *dest, char portDelineator=':') const;

	/* 内部使用: sockaddr_in6 除 IP 和端口外还需要额外数据，从已有 SystemAddress 中复制该额外数据 */
	void FixForIPVersion(const SystemAddress &boundAddressToSocket);

	bool IsLANAddress();

	SystemAddress& operator = ( const SystemAddress& input );
	bool operator==( const SystemAddress& right ) const;
	bool operator!=( const SystemAddress& right ) const;
	bool operator > ( const SystemAddress& right ) const;
	bool operator < ( const SystemAddress& right ) const;

	/* 内部使用: 用于快速查找，可选（使用 -1 则进行常规查找），不要传输此字段。 */
	SystemIndex systemIndex;

	private:

#if RAKNET_SUPPORT_IPV6==1
		void ToString_New(bool writePort, char *dest, char portDelineator) const;
#endif
};

/*
 * 唯一标识一个 RakPeer 实例。
 * 使用 RakPeer::GetGuidFromSystemAddress() 和 RakPeer::GetSystemAddressFromGuid() 在 SystemAddress 与 RakNetGUID 之间转换。
 * 使用 RakPeer::GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS) 获取自身的 GUID。
 */
struct RAK_DLL_EXPORT RakNetGUID
{
	RakNetGUID();
	explicit RakNetGUID(uint64_t _g) {g=_g; systemIndex=(SystemIndex)-1;}
/* uint32_t g[6]; */
	uint64_t g;

	/*
	 * 以字符串形式返回 GUID。
	 * 返回一个静态字符串（非线程安全）。
	 */
	const char *ToString(void) const;

	/*
	 * 以字符串形式返回 GUID，写入 dest（线程安全）。
	 * dest 必须足够大以容纳输出结果。
	 */
	void ToString(char *dest) const;

	bool FromString(const char *source);

	static unsigned long ToUint32( const RakNetGUID &g );

	RakNetGUID& operator = ( const RakNetGUID& input )
	{
		g=input.g;
		systemIndex=input.systemIndex;
		return *this;
	}

	/* 内部使用: 用于快速查找，可选（使用 -1 则进行常规查找），不要传输此字段。 */
	SystemIndex systemIndex;
	static int size() {return static_cast<int>(sizeof(uint64_t));}

	bool operator==( const RakNetGUID& right ) const;
	bool operator!=( const RakNetGUID& right ) const;
	bool operator > ( const RakNetGUID& right ) const;
	bool operator < ( const RakNetGUID& right ) const;
};

/* 无效 SystemAddress 的索引值 */
/*
const SystemAddress UNASSIGNED_SYSTEM_ADDRESS =
{
	0xFFFFFFFF, 0xFFFF
};
*/
#ifndef SWIG
const SystemAddress UNASSIGNED_SYSTEM_ADDRESS;
const RakNetGUID UNASSIGNED_RAKNET_GUID((uint64_t)-1);
#endif
/*
{
	{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
	0xFFFFFFFFFFFFFFFF
};
*/


struct RAK_DLL_EXPORT AddressOrGUID
{
	RakNetGUID rakNetGuid;
	SystemAddress systemAddress;

	SystemIndex GetSystemIndex(void) const {if (rakNetGuid!=UNASSIGNED_RAKNET_GUID) return rakNetGuid.systemIndex; else return systemAddress.systemIndex;}
	bool IsUndefined(void) const {return rakNetGuid==UNASSIGNED_RAKNET_GUID && systemAddress==UNASSIGNED_SYSTEM_ADDRESS;}
	void SetUndefined() {rakNetGuid=UNASSIGNED_RAKNET_GUID; systemAddress=UNASSIGNED_SYSTEM_ADDRESS;}
	static unsigned long ToInteger( const AddressOrGUID &aog );
	const char *ToString(bool writePort=true) const;
	void ToString(bool writePort, char *dest) const;

	AddressOrGUID() {}
	AddressOrGUID( const AddressOrGUID& input )
	{
		rakNetGuid=input.rakNetGuid;
		systemAddress=input.systemAddress;
	}
	AddressOrGUID( const SystemAddress& input )
	{
		rakNetGuid=UNASSIGNED_RAKNET_GUID;
		systemAddress=input;
	}
	AddressOrGUID( Packet *packet );
	AddressOrGUID( const RakNetGUID& input )
	{
		rakNetGuid=input;
		systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
	}
	AddressOrGUID& operator = ( const AddressOrGUID& input )
	{
		rakNetGuid=input.rakNetGuid;
		systemAddress=input.systemAddress;
		return *this;
	}

	AddressOrGUID& operator = ( const SystemAddress& input )
	{
		rakNetGuid=UNASSIGNED_RAKNET_GUID;
		systemAddress=input;
		return *this;
	}

	AddressOrGUID& operator = ( const RakNetGUID& input )
	{
		rakNetGuid=input;
		systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
		return *this;
	}

	inline bool operator==( const AddressOrGUID& right ) const {return (rakNetGuid!=UNASSIGNED_RAKNET_GUID && rakNetGuid==right.rakNetGuid) || (systemAddress!=UNASSIGNED_SYSTEM_ADDRESS && systemAddress==right.systemAddress);}
};

using NetworkID = uint64_t;

/* 表示来自其他系统的一条用户消息。 */
struct Packet
{
	/* 发送此数据包的系统地址。 */
	SystemAddress systemAddress;

	/*
	 * 发送此数据包的系统的唯一标识符，与 IP 地址无关（适用于内网/外网/远程系统）。
	 * 仅在连接建立后有效（收到 ID_CONNECTION_REQUEST_ACCEPTED 或 ID_NEW_INCOMING_CONNECTION 后）。
	 * 在此之前值为 UNASSIGNED_RAKNET_GUID。
	 */
	RakNetGUID guid;

	/* 数据的字节长度 */
	unsigned int length;

	/* 数据的比特长度 */
	BitSize_t bitSize;

	/* 发送方的数据内容 */
	unsigned char* data;

	/*
	 * 内部使用:
	 * 指示是否需要删除 data，或仅删除数据包本身。
	 */
	bool deleteData;

	/*
	 * 内部使用:
	 * 若为 true，此消息面向用户而非插件，不应通过插件处理。
	 */
	bool wasGeneratedLocally;
};

/* 未分配玩家的索引值 */
const SystemIndex UNASSIGNED_PLAYER_INDEX = 65535;

/* 未分配对象的 ID */
const NetworkID UNASSIGNED_NETWORK_ID = (uint64_t) -1;

const int PING_TIMES_ARRAY_SIZE = 5;

struct RAK_DLL_EXPORT uint24_t
{
	uint32_t val;

	uint24_t() {}
	inline operator uint32_t() { return val; }
	inline operator uint32_t() const { return val; }

	inline uint24_t(const uint24_t& a) {val=a.val;}
	inline uint24_t operator++() {++val; val&=0x00FFFFFF; return *this;}
	inline uint24_t operator--() {--val; val&=0x00FFFFFF; return *this;}
	inline uint24_t operator++(int) {uint24_t temp(val); ++val; val&=0x00FFFFFF; return temp;}
	inline uint24_t operator--(int) {uint24_t temp(val); --val; val&=0x00FFFFFF; return temp;}
	inline uint24_t operator&(const uint24_t& a) {return uint24_t(val&a.val);}
	inline uint24_t& operator=(const uint24_t& a) { val=a.val; return *this; }
	inline uint24_t& operator+=(const uint24_t& a) { val+=a.val; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator-=(const uint24_t& a) { val-=a.val; val&=0x00FFFFFF; return *this; }
	inline bool operator==( const uint24_t& right ) const {return val==right.val;}
	inline bool operator!=( const uint24_t& right ) const {return val!=right.val;}
	inline bool operator > ( const uint24_t& right ) const {return val>right.val;}
	inline bool operator < ( const uint24_t& right ) const {return val<right.val;}
	inline const uint24_t operator+( const uint24_t &other ) const { return uint24_t(val+other.val); }
	inline const uint24_t operator-( const uint24_t &other ) const { return uint24_t(val-other.val); }
	inline const uint24_t operator/( const uint24_t &other ) const { return uint24_t(val/other.val); }
	inline const uint24_t operator*( const uint24_t &other ) const { return uint24_t(val*other.val); }

	inline uint24_t(const uint32_t& a) {val=a; val&=0x00FFFFFF;}
	inline uint24_t operator&(const uint32_t& a) {return uint24_t(val&a);}
	inline uint24_t& operator=(const uint32_t& a) { val=a; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator+=(const uint32_t& a) { val+=a; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator-=(const uint32_t& a) { val-=a; val&=0x00FFFFFF; return *this; }
	inline bool operator==( const uint32_t& right ) const {return val==(right&0x00FFFFFF);}
	inline bool operator!=( const uint32_t& right ) const {return val!=(right&0x00FFFFFF);}
	inline bool operator > ( const uint32_t& right ) const {return val>(right&0x00FFFFFF);}
	inline bool operator < ( const uint32_t& right ) const {return val<(right&0x00FFFFFF);}
	inline const uint24_t operator+( const uint32_t &other ) const { return uint24_t(val+other); }
	inline const uint24_t operator-( const uint32_t &other ) const { return uint24_t(val-other); }
	inline const uint24_t operator/( const uint32_t &other ) const { return uint24_t(val/other); }
	inline const uint24_t operator*( const uint32_t &other ) const { return uint24_t(val*other); }
};

} /* RakNet 命名空间 */
