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
 * Message filter plugin. Assigns systems to FilterSets.  Each FilterSet limits what messages are allowed.  This is a security related plugin.
 *
 */

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_MessageFilter==1

#pragma once
#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "DS_OrderedList.h"
#include "DS_Hash.h"
#include "Export.h"

/* MessageIdentifier (ID_*) values shoudln't go higher than this.  Change it if you do. */
#define MESSAGE_FILTER_MAX_MESSAGE_ID 256

namespace RakNet
{
/* 前向声明 */
class RakPeerInterface;

/* 内部使用 Has to be public so some of the shittier compilers can use it. */
int RAK_DLL_EXPORT MessageFilterStrComp( char *const &key,char *const &data );

/* 内部使用 Has to be public so some of the shittier compilers can use it. */
struct FilterSet
{
	bool banOnFilterTimeExceed;
	bool kickOnDisallowedMessage;
	bool banOnDisallowedMessage;
	RakNet::TimeMS disallowedMessageBanTimeMS;
	RakNet::TimeMS timeExceedBanTimeMS;
	RakNet::TimeMS maxMemberTimeMS;
	void (*invalidMessageCallback)(RakPeerInterface *peer, AddressOrGUID systemAddress, int filterSetID, void *userData, unsigned char messageID);
	void *disallowedCallbackUserData;
	void (*timeoutCallback)(RakPeerInterface *peer, AddressOrGUID systemAddress, int filterSetID, void *userData);
	void *timeoutUserData;
	int filterSetID;
	bool allowedIDs[MESSAGE_FILTER_MAX_MESSAGE_ID];
	DataStructures::OrderedList<RakNet::RakString,RakNet::RakString> allowedRPC4;
};

/* 内部使用 Has to be public so some of the shittier compilers can use it. */
int RAK_DLL_EXPORT FilterSetComp( const int &key, FilterSet * const &data );

/* 内部使用 Has to be public so some of the shittier compilers can use it. */
struct FilteredSystem
{
	FilterSet *filter;
	RakNet::TimeMS timeEnteredThisSet;
};

/*
 * \defgroup MESSAGEFILTER_GROUP MessageFilter
 * Remote incoming packets from unauthorized systems
 * * \ingroup PLUGINS_GROUP
 */

/*
 * Assigns systems to FilterSets.  Each FilterSet limits what kinds of messages are allowed.
 * The MessageFilter plugin is used for security where you limit what systems can send what kind of messages.<BR>
 * You implicitly define FilterSets, and add allowed message IDs to these FilterSets.<BR>
 * You then add systems to these filters, such that those systems are limited to sending what the filters allows.<BR>
 * You can automatically assign systems to a filter.<BR>
 * You can automatically kick and possibly ban users that stay in a filter too long, or send the wrong message.<BR>
 * Each system is a member of either zero or one filters.<BR>
 * Add this plugin before any plugin you wish to filter (most likely just add this plugin before any other).
 * \ingroup MESSAGEFILTER_GROUP
 */
class RAK_DLL_EXPORT MessageFilter : public PluginInterface2
{
public:

	/* 获取单例 GetInstance() 和销毁单例 DestroyInstance(instance*) */
	STATIC_FACTORY_DECLARATIONS(MessageFilter)

	MessageFilter();
	virtual ~MessageFilter();

	/* -------------------------------------------------------------------------------------------- */
	/* User functions */
	/* -------------------------------------------------------------------------------------------- */

	/*
	 * Automatically add all new systems to a particular filter
	 * 默认为 -1
	 * 参数[输入] filterSetID Which filter to add new systems to.  <0 for do not add.
	 */
	void SetAutoAddNewConnectionsToFilter(int filterSetID);

	/*
	 * Allow a range of message IDs
	 * Always allowed by default: ID_CONNECTION_REQUEST_ACCEPTED through ID_DOWNLOAD_PROGRESS
	 * Usually you specify a range to make it easier to add new enumerations without having to constantly refer back to this function.
	 * 参数[输入] allow True to allow this message ID, false to disallow. By default, all messageIDs except the noted types are disallowed.  This includes messages from other plugins!
	 * 参数[输入] messageIDStart The first ID_* message to allow in the range.  Inclusive.
	 * 参数[输入] messageIDEnd The last ID_* message to allow in the range.  Inclusive.
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 */
	void SetAllowMessageID(bool allow, int messageIDStart, int messageIDEnd,int filterSetID);

	/*
	 * Allow a specific RPC4 call
	 * 前提条件: MessageFilter must be attached before RPC4
	 * 参数[输入] uniqueID Identifier passed to RegisterFunction()
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 */
	void SetAllowRPC4(bool allow, const char* uniqueID, int filterSetID);

	/*
	 * What action to take on a disallowed message.  You can kick or not.  You can add them to the ban list for some time
	 * By default no action is taken.  The message is simply ignored.
	 * param[in] 0 for permanent ban, >0 for ban time in milliseconds.
	 * 参数[输入] kickOnDisallowed kick the system that sent a disallowed message.
	 * 参数[输入] banOnDisallowed ban the system that sent a disallowed message.  See banTimeMS for the ban duration
	 * 参数[输入] banTimeMS Passed to the milliseconds parameter of RakPeer::AddToBanList.
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 */
	void SetActionOnDisallowedMessage(bool kickOnDisallowed, bool banOnDisallowed, RakNet::TimeMS banTimeMS, int filterSetID);

	/*
	 * 将a user callback设置为be called on an invalid message for a particular filterSet
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 * 参数[输入] userData A pointer passed with the callback
	 * 参数[输入] invalidMessageCallback A pointer to a C function to be called back with the specified parameters.
	 */
	void SetDisallowedMessageCallback(int filterSetID, void *userData, void (*invalidMessageCallback)(RakPeerInterface *peer, AddressOrGUID addressOrGUID, int filterSetID, void *userData, unsigned char messageID));

	/*
	 * 将a user callback设置为be called when a user is disconnected due to SetFilterMaxTime
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 * 参数[输入] userData A pointer passed with the callback
	 * 参数[输入] invalidMessageCallback A pointer to a C function to be called back with the specified parameters.
	 */
	void SetTimeoutCallback(int filterSetID, void *userData, void (*invalidMessageCallback)(RakPeerInterface *peer, AddressOrGUID addressOrGUID, int filterSetID, void *userData));

	/*
	 * Limit how long a connection can stay in a particular filterSetID. After this time, the connection is kicked and possibly banned.
	 * By default there is no limit to how long a connection can stay in a particular filter set.
	 * 参数[输入] allowedTimeMS How many milliseconds to allow a connection to stay in this filter set.
	 * 参数[输入] banOnExceed True or false to ban the system, or not, when allowedTimeMS is exceeded
	 * 参数[输入] banTimeMS Passed to the milliseconds parameter of RakPeer::AddToBanList.
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.
	 */
	void SetFilterMaxTime(int allowedTimeMS, bool banOnExceed, RakNet::TimeMS banTimeMS, int filterSetID);

	/*
	 * Get the filterSetID a system is using.  Returns -1 for 无.
	 * 参数[输入] addressOrGUID The system we are referring to
	 */
	int GetSystemFilterSet(AddressOrGUID addressOrGUID);

	/*
	 * Assign a system to a filter set.
	 * Systems are automatically added to filter sets (or not) based on SetAutoAddNewConnectionsToFilter()
	 * This function is used to change the filter set a system is using, to add it to a new filter set, or to remove it from all existin filter sets.
	 * 参数[输入] addressOrGUID The system we are referring to
	 * 参数[输入] filterSetID A user defined ID to represent a filter set.  If no filter with this ID exists, one will be created with default settings.  If -1, the system will be removed from all filter sets.
	 */
	void SetSystemFilterSet(AddressOrGUID addressOrGUID, int filterSetID);

	/*
	 * 返回 number of systems subscribed to a particular filter set
	 * Using anything other than -1 for filterSetID is slow, so you should store the returned value.
	 * 参数[输入] filterSetID The filter set to limit to.  Use -1 for 无 (just returns the total number of filter systems in that case).
	 */
	unsigned GetSystemCount(int filterSetID) const;

	/*
	 * 返回 total number of filter sets
	 * 返回值: The total number of filter sets.
	 */
	unsigned GetFilterSetCount(void) const;

	/*
	 * 返回 ID of a filter set, by index
	 * 参数[输入] An index between 0 and GetFilterSetCount()-1 inclusive
	 */
	int GetFilterSetIDByIndex(unsigned index);

    /*
     * Delete a FilterSet.  All systems formerly subscribed to this filter are now unrestricted.
     * 参数[输入] filterSetID The ID of the filter set to delete.
     */
	void DeleteFilterSet(int filterSetID);

	/* -------------------------------------------------------------------------------------------- */
	/* 数据包处理函数 */
	/* -------------------------------------------------------------------------------------------- */
	void Update() override;
	PluginReceiveResult OnReceive(Packet *packet) override;
	void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming) override;
	void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason ) override;

protected:

	void Clear();
	void DeallocateFilterSet(FilterSet *filterSet);
	FilterSet* GetFilterSetByID(int filterSetID);
	void OnInvalidMessage(FilterSet *filterSet, AddressOrGUID systemAddress, unsigned char messageID);

	DataStructures::OrderedList<int, FilterSet*, FilterSetComp> filterList;
	/* Change to guid */
	DataStructures::Hash<AddressOrGUID, FilteredSystem, 2048, AddressOrGUID::ToInteger> systemList;

	int autoAddNewConnectionsToFilter;
	RakNet::Time whenLastTimeoutCheck;
};

} /* RakNet 命名空间 */

#endif
