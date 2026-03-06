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
 * Remote procedure call, supporting C functions only. No 外部 dependencies 必须.
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RPC4Plugin==1

#pragma once
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakString.h"
#include "NetworkIDObject.h"
#include "DS_Hash.h"
#include "DS_OrderedList.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

/*
 * \defgroup RPC_PLUGIN_GROUP RPC
 * Remote procedure calls, without 外部 dependencies.
 * This should not be used at the same time as RPC3. This is a less functional version of RPC3, and is here for users that do not want the Boost dependency of RPC3.
 * \ingroup PLUGINS_GROUP
 */

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;
class NetworkIDManager;

	/*
	 * Error codes returned by a remote system as to why an RPC function call cannot execute
	 * Error code follows packet ID ID_RPC_REMOTE_ERROR, that is packet->data[1]<BR>
	 * Name of the function will be appended starting at packet->data[2]
	 * \ingroup RPC_PLUGIN_GROUP
	 */
	enum RPCErrorCodes
	{
		/* Named function was not registered with RegisterFunction(). Check your spelling. */
		RPC_ERROR_FUNCTION_NOT_REGISTERED,
	};

	/* Instantiate this class globally if you want to register a function with RPC4 at the global space */
	class RAK_DLL_EXPORT RPC4GlobalRegistration
	{
	public:
		/* Queue a call to RPC4::RegisterFunction() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface. */
		RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ));

		/* Queue a call to RPC4::RegisterSlot() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface. */
		RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ), int callPriority);

		/* Queue a call to RPC4::RegisterBlockingFunction() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface. */
		RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ));

		/* Queue a call to RPC4::RegisterLocalCallback() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface. */
		RPC4GlobalRegistration(const char* uniqueID, MessageID messageId);
	};

	/*
	 * The RPC4 plugin is just an association between a C function pointer and a string.
	 * It is for users that want to use RPC, but do not want to use boost.
	 * You do not have the automatic serialization or other features of RPC3, and C++ member calls are not supported.
	 * 注意: You cannot use RPC4 at the same time as RPC3Plugin
	 * \ingroup RPC_PLUGIN_GROUP
	 */
	class RAK_DLL_EXPORT RPC4 : public PluginInterface2
	{
	public:
		/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
		STATIC_FACTORY_DECLARATIONS(RPC4)

		/* 构造函数 */
		RPC4();

		/* 析构函数 */
		virtual ~RPC4();

		/*
		 * 已废弃 Use RegisterSlot
		 * Register a function pointer to be callable from a remote system
		 * The hash of the function name will be stored as an association with the function pointer
		 * When a call is made to call this function from the Call() or CallLoopback() function, the function pointer will be invoked with the passed bitStream to Call() and the actual Packet that RakNet got.
		 * 另见 RegisterPacketCallback()
		 * 参数[输入] uniqueID Identifier to be associated with functionPointer. If this identifier is already in use, the call will return false.
		 * 参数[输入] functionPointer C function pointer to be called
		 * 返回值: True if the hash of uniqueID is not in use, false otherwise.
		 */
		bool RegisterFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ));

		/*
		 * Register a slot, which is a function pointer to one or more implementations that supports this function signature
		 * When a signal occurs, all slots with the same identifier are called.
		 * 参数[输入] sharedIdentifier A string to identify the slot. Recommended to be the same as the name of the function.
		 * 参数[输入] functionPtr Pointer to the function. For C, just pass the name of the function. For C++, use ARPC_REGISTER_CPP_FUNCTION
		 * 参数[输入] callPriority Slots are called by order of the highest callPriority first. For slots with the same priority, they are called in the order they are registered
		 */
		void RegisterSlot(const char *sharedIdentifier, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ), int callPriority);

		/* 与 RegisterFunction, but is called with CallBlocking() instead of Call() and returns a value to the caller 相同 */
		bool RegisterBlockingFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ));

		/*
		 * 已废弃 Use RegisterSlot and invoke on self only when the packet you want arrives
		 * When a RakNet Packet with the specified identifier is returned, execute CallLoopback() on a function previously registered with RegisterFunction()
		 * For example, you could call "OnClosedConnection" whenever you get ID_DISCONNECTION_NOTIFICATION or ID_CONNECTION_LOST
		 * 参数[输入] uniqueID Identifier passed to RegisterFunction()
		 * 参数[输入] messageId What RakNet packet ID to call on, for example ID_DISCONNECTION_NOTIFICATION or ID_CONNECTION_LOST
		 */
		void RegisterLocalCallback(const char* uniqueID, MessageID messageId);

		/*
		 * Unregister a function pointer previously registered with RegisterFunction()
		 * 参数[输入] Identifier originally passed to RegisterFunction()
		 * 返回值: True if the hash of uniqueID was in use, and hence removed. false otherwise.
		 */
		bool UnregisterFunction(const char* uniqueID);

		/* 与 UnregisterFunction, except for a blocking function 相同 */
		bool UnregisterBlockingFunction(const char* uniqueID);

		/*
		 * Remove the association created with RegisterPacketCallback()
		 * 参数[输入] uniqueID Identifier passed as uniqueID to RegisterLocalCallback()
		 * 参数[输入] messageId Identifier passed as messageId to RegisterLocalCallback()
		 * 返回值: True if the combination of uniqueID and messageId was in use, and hence removed
		 */
		bool UnregisterLocalCallback(const char* uniqueID, MessageID messageId);

		/*
		 * Remove the association created with RegisterSlot()
		 * 参数[输入] sharedIdentifier Identifier passed as sharedIdentifier to RegisterSlot()
		 */
		bool UnregisterSlot(const char* sharedIdentifier);

		/*
		 * 已废弃 Use RegisterSlot() and Signal() with your own RakNetGUID as the send target
		 * Send to the attached instance of RakPeer. See RakPeerInterface::SendLoopback()
		 * 参数[输入] Identifier originally passed to RegisterFunction() on the local system
		 * 参数[输入] bitStream bitStream encoded data to send to the function callback
		 */
		void CallLoopback( const char* uniqueID, RakNet::BitStream * bitStream );

		/*
		 * 已废弃, use Signal()
		 * Send to the specified remote instance of RakPeer.
		 * 参数[输入] uniqueID Identifier originally passed to RegisterFunction() on the remote system(s)
		 * 参数[输入] bitStream bitStream encoded data to send to the function callback
		 * 参数[输入] priority 参见 RakPeerInterface::Send()
		 * 参数[输入] reliability 参见 RakPeerInterface::Send()
		 * 参数[输入] orderingChannel 参见 RakPeerInterface::Send()
		 * 参数[输入] systemIdentifier 参见 RakPeerInterface::Send()
		 * 参数[输入] broadcast 参见 RakPeerInterface::Send()
		 */
		void Call( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast );

		/*
		 * 与 call, but don't return until the remote system replies 相同
		 * Broadcasting parameter does not exist, this can only call one remote system
		 * 注意: This function does not return until the remote system responds, disconnects, or was never connected to begin with
		 * 参数[输入] Identifier originally passed to RegisterBlockingFunction() on the remote system(s)
		 * 参数[输入] bitStream bitStream encoded data to send to the function callback
		 * 参数[输入] priority 参见 RakPeerInterface::Send()
		 * 参数[输入] reliability 参见 RakPeerInterface::Send()
		 * 参数[输入] orderingChannel 参见 RakPeerInterface::Send()
		 * 参数[输入] systemIdentifier 参见 RakPeerInterface::Send()
		 * 参数[输出] returnData Written to by the function registered with RegisterBlockingFunction.
		 * 返回值: true if successfully called. False on disconnect, function not registered, or not connected to begin with
		 */
		bool CallBlocking( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, RakNet::BitStream *returnData );

		/*
		 * Calls zero or more functions identified by sharedIdentifier registered with RegisterSlot()
		 * 参数[输入] sharedIdentifier parameter of the same name passed to RegisterSlot() on the remote system
		 * 参数[输入] bitStream bitStream encoded data to send to the function callback
		 * 参数[输入] priority 参见 RakPeerInterface::Send()
		 * 参数[输入] reliability 参见 RakPeerInterface::Send()
		 * 参数[输入] orderingChannel 参见 RakPeerInterface::Send()
		 * 参数[输入] systemIdentifier 参见 RakPeerInterface::Send()
		 * 参数[输入] broadcast 参见 RakPeerInterface::Send()
		 * 参数[输入] invokeLocal 如果true，则also sends to self
		 */
		void Signal(const char *sharedIdentifier, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast, bool invokeLocal);

		/* If called while processing a slot, no further slots for the currently executing signal will be executed */
		void InterruptSignal();

		/* 内部使用 */
		struct LocalCallback
		{
			MessageID messageId;
			DataStructures::OrderedList<RakNet::RakString,RakNet::RakString> functions;
		};
		static int LocalCallbackComp(const MessageID &key, LocalCallback* const &data );

		/* 内部使用 */
		/* Callable object, along with priority to call relative to other objects */
		struct LocalSlotObject
		{
			LocalSlotObject() {}
			LocalSlotObject(unsigned int _registrationCount,int _callPriority, void ( *_functionPointer ) ( RakNet::BitStream *userData, Packet *packet ))
			{registrationCount=_registrationCount;callPriority=_callPriority;functionPointer=_functionPointer;}
			~LocalSlotObject() {}

			/* Used so slots are called in the order they are registered */
			unsigned int registrationCount;
			int callPriority;
			void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet );
		};

		static int LocalSlotObjectComp( const LocalSlotObject &key, const LocalSlotObject &data );

		/* 内部使用 */
		struct LocalSlot
		{
			DataStructures::OrderedList<LocalSlotObject,LocalSlotObject,LocalSlotObjectComp> slotObjects;
		};
		DataStructures::Hash<RakNet::RakString, LocalSlot*,256, RakNet::RakString::ToInteger> localSlots;

	protected:

		/* -------------------------------------------------------------------------------------------- */
		/* 数据包处理函数 */
		/* -------------------------------------------------------------------------------------------- */
		void OnAttach() override;
		PluginReceiveResult OnReceive(Packet *packet) override;

		DataStructures::Hash<RakNet::RakString, void ( * ) ( RakNet::BitStream *, Packet * ),64, RakNet::RakString::ToInteger> registeredNonblockingFunctions;
		DataStructures::Hash<RakNet::RakString, void ( * ) ( RakNet::BitStream *, RakNet::BitStream *, Packet * ),64, RakNet::RakString::ToInteger> registeredBlockingFunctions;
		DataStructures::OrderedList<MessageID,LocalCallback*,RPC4::LocalCallbackComp> localCallbacks;

		RakNet::BitStream blockingReturnValue;
		bool gotBlockingReturnValue;

		DataStructures::HashIndex GetLocalSlotIndex(const char *sharedIdentifier);

		/* Used so slots are called in the order they are registered */
		unsigned int nextSlotRegistrationCount;

		bool interruptSignal;

		void InvokeSignal(DataStructures::HashIndex functionIndex, RakNet::BitStream *serializedParameters, Packet *packet);
	};

} /* 命名空间结束 */

#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif
