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
 *  TwoWayAuthentication.h
 * 实现 two way authentication
 * Given two systems, each of whom known a common password, verify the password without transmitting it
 * 可用于确定应对另一个系统授予哪些权限
 *
 */


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_TwoWayAuthentication==1

#pragma once
/* How often to change the nonce. */
#define NONCE_TIMEOUT_MS 10000
/* How often to check for ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT, and the minimum timeout time. Maximum is double this value. */
#define CHALLENGE_MINIMUM_TIMEOUT 3000

#if LIBCAT_SECURITY==1
/* From CPP FILE: */
/* static constexpr int HASH_BITS = 256; */
/* static constexpr int HASH_BYTES = HASH_BITS / 8; */
/* static constexpr int STRENGTHENING_FACTOR = 1000; */
#define TWO_WAY_AUTHENTICATION_NONCE_LENGTH 32
#define HASHED_NONCE_AND_PW_LENGTH 32
#else
#include "DR_SHA1.h"
#define TWO_WAY_AUTHENTICATION_NONCE_LENGTH 20
#define HASHED_NONCE_AND_PW_LENGTH SHA1_LENGTH
#endif

#include "PluginInterface2.h"
#include "RakMemoryOverride.h"
#include "NativeTypes.h"
#include "RakString.h"
#include "DS_Hash.h"
#include "DS_Queue.h"

using FCM2Guid = int64_t;

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;

/*
 * 实现 two way authentication
 * Given two systems, each of whom known a common password / identifier pair, verify the password without transmitting it
 * 可用于确定应对另一个系统授予哪些权限
 * If the other system should not send any data until authentication passes, you can use the MessageFilter plugin for this. Call MessageFilter::SetAllowMessageID() including ID_TWO_WAY_AUTHENTICATION_NEGOTIATION when doing so. Also attach MessageFilter first in the list of plugins
 * 注意: If other systems challenges us, and fails, you will get ID_TWO_WAY_AUTHENTICATION_INCOMING_CHALLENGE_FAILED.
 * \ingroup PLUGINS_GROUP
 */
class RAK_DLL_EXPORT TwoWayAuthentication : public PluginInterface2
{
public:
	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(TwoWayAuthentication)

	TwoWayAuthentication();
	virtual ~TwoWayAuthentication();

	/*
	 * Adds a password to the list of passwords the system will accept
	 * Each password, which is secret and not transmitted, is identified by identifier.
	 * identifier is transmitted in plaintext with the request. It is only needed because the system supports multiple password.
	 * It is used to only hash against once password on the remote system, rather than having to hash against every known password.
	 * 参数[输入] identifier A unique identifier representing this password. This is transmitted in plaintext and should be considered insecure
	 * 参数[输入] password The password to add
	 * 返回值: True on success, false on identifier==password, either identifier or password is blank, or identifier is already in use
	 */
	bool AddPassword(RakNet::RakString identifier, RakNet::RakString password);

	/*
	 * Challenge another system for the specified identifier
	 * After calling Challenge, you will get back ID_TWO_WAY_AUTHENTICATION_SUCCESS, ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_TIMEOUT, or ID_TWO_WAY_AUTHENTICATION_OUTGOING_CHALLENGE_FAILED
	 * ID_TWO_WAY_AUTHENTICATION_SUCCESS will be returned if and only if the other system has called AddPassword() with the same identifier\password pair as this system.
	 * 参数[输入] identifier A unique identifier representing this password. This is transmitted in plaintext and should be considered insecure
	 * 返回值: True on success, false on remote system not connected, or identifier not previously added with AddPassword()
	 */
	bool Challenge(RakNet::RakString identifier, AddressOrGUID remoteSystem);

	/* 释放 all memory */
	void Clear();

	/* 内部使用 */
	void Update() override;
	/* 内部使用 */
	PluginReceiveResult OnReceive(Packet *packet) override;
	/* 内部使用 */
	void OnRakPeerShutdown() override;
	/* 内部使用 */
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;

	/* 内部使用 */
	struct PendingChallenge
	{
		RakNet::RakString identifier;
		AddressOrGUID remoteSystem;
		RakNet::Time time;
		bool sentHash;
	};

	DataStructures::Queue<PendingChallenge> outgoingChallenges;

	/* 内部使用 */
	struct NonceAndRemoteSystemRequest
	{
		char nonce[TWO_WAY_AUTHENTICATION_NONCE_LENGTH];
		RakNet::AddressOrGUID remoteSystem;
		unsigned short requestId;
		RakNet::Time whenGenerated;
	};
	/* 内部使用 */
	struct RAK_DLL_EXPORT NonceGenerator
	{
		NonceGenerator();
		~NonceGenerator() noexcept;
		void GetNonce(char nonce[TWO_WAY_AUTHENTICATION_NONCE_LENGTH], unsigned short *requestId, RakNet::AddressOrGUID remoteSystem);
		void GenerateNonce(char nonce[TWO_WAY_AUTHENTICATION_NONCE_LENGTH]);
		bool GetNonceById(char nonce[TWO_WAY_AUTHENTICATION_NONCE_LENGTH], unsigned short requestId, RakNet::AddressOrGUID remoteSystem, bool popIfFound);
		void Clear();
		void ClearByAddress(RakNet::AddressOrGUID remoteSystem);
		void Update(RakNet::Time curTime);

		DataStructures::List<TwoWayAuthentication::NonceAndRemoteSystemRequest*> generatedNonces;
		unsigned short nextRequestId;
	};

protected:
	void PushToUser(MessageID messageId, RakNet::RakString password, RakNet::AddressOrGUID remoteSystem);
	/* Key is identifier, data is password */
	DataStructures::Hash<RakNet::RakString, RakNet::RakString, 16, RakNet::RakString::ToInteger > passwords;

	RakNet::Time whenLastTimeoutCheck;

	NonceGenerator nonceGenerator;

	void OnNonceRequest(Packet *packet);
	void OnNonceReply(Packet *packet);
	PluginReceiveResult OnHashedNonceAndPassword(Packet *packet);
	void OnPasswordResult(Packet *packet);
	void Hash(char thierNonce[TWO_WAY_AUTHENTICATION_NONCE_LENGTH], RakNet::RakString password, char out[HASHED_NONCE_AND_PW_LENGTH]);
};

} /* RakNet 命名空间 */

#endif
